/*
 * Copyright 2011-2012 Bjorn Fahller <bjorn@fahller.se>
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "test_case_factory.hpp"
#include "wrapped/posix_encapsulation.hpp"
#include "posix_error.hpp"
#include "tag_filter.hpp"
#include "poll_buffer_vector.hpp"
#include "fsfuncs.hpp"
#include "pipe_pair.hpp"
#include "output/heap_buffer.hpp"
#include <algorithm>
#include "presentation.hpp"
#include "output/xml_formatter.hpp"
#include "output/text_formatter.hpp"
#include "output/nil_formatter.hpp"
#include "cli/interpreter.hpp"
#include "buffer_vector.hpp"
#include "working_dir_allocator.hpp"
#include "deadline_monitor.hpp"
#include "test_environment.hpp"
extern "C" {
#  include <sys/time.h>
#  include <fcntl.h>
}

namespace {

  template <typename T, size_t N>
  T *begin(T (&array)[N])
  {
    return array;
  }

  template <typename T, size_t N>
  T *end(T (&array)[N])
  {
    return array + N;
  }


  template <size_t N>
  bool strequal(const char (&s)[N], const char *p, size_t n)
  {
    return N == n + 1 && crpcut::wrapped::strncmp(s, p, n) == 0;
  }


  template <typename A, typename B>
  crpcut::output::formatter&
  select_output_formatter(crpcut::output::buffer &buffer,
                          bool                    use_A,
                          const char             *id,
                          const char * const      argv[],
                          crpcut::tag_list_root  &tags,
                          unsigned                num_registered,
                          unsigned                num_selected)
  {
    if (use_A)
      {
        static A ao(buffer, id, argv, tags, num_registered, num_selected);
        return ao;
      }
    static B bo(buffer, id, argv, tags, num_registered, num_selected);
    return bo;
  }

  CRPCUT_DEFINE_EXCEPTION_TRANSLATOR_CLASS(std_exception_translator,
                                           std::exception &e)
  {
    return std::string("std::exception\n\twhat()=") + e.what();
  }

  CRPCUT_DEFINE_EXCEPTION_TRANSLATOR_CLASS(c_string_translator,
                                           const char *p)
  {
    return "\"" + std::string(p) + "\"";
  }

  class cli_exception : public std::exception
  {
  public:
    cli_exception(int i = -1) : code_(i) {}
    const char *what() const throw () { return ""; }
    int return_code() const { return code_; }
  private:
    int code_;
  };

  int open_report_file(const char *name, std::ostream &err_os)
  {
    if (!name) return 1;
    int fd = crpcut::wrapped::open(name, O_CREAT | O_WRONLY | O_TRUNC,
                                   0666);
    if (fd <      0)
      {
        err_os << "Failed to open " << name << " for writing\n";
        throw cli_exception();
      }
    return fd;
  }

  void list_tags(crpcut::tag_list_root &tags)
  {
    for (crpcut::tag_list::iterator i = tags.begin(); i != tags.end(); ++i)
      {
        std::cout << i->get_name().str << "\n";
      }
    throw cli_exception(0);
  }

}

namespace crpcut {

  void
  test_case_factory::registrator_list
  ::run_test_case()
  {
    wrapped::abort();
  }

  tag&
  test_case_factory::registrator_list
  ::crpcut_tag() const
  {
    return wrapped::abort(), crpcut_tag_info<crpcut::crpcut_none>::obj();
  }

  tag::importance
  test_case_factory::registrator_list
  ::get_importance() const
  {
    return wrapped::abort(), tag::disabled;
  }

  void
  test_case_factory::registrator_list
  ::setup(poll<fdreader> &, pid_t, int, int, int, int)
  {
    wrapped::abort();
  }

  test_case_factory
  ::test_case_factory()
    : cli_(0),
      current_pid_(0),
      num_pending_children_(0),
      num_tests_run_(0),
      num_successful_tests_(0),
      presenter_pipe_(-1),
      deadlines_(0),
      working_dirs_(0),
      charset_("UTF-8")
  {
    lib::strcpy(dirbase_, "/tmp/crpcutXXXXXX");
  }

  test_case_factory
  ::~test_case_factory()
  {
  }

  void
  test_case_factory
  ::set_charset(const char *set_name)
  {
    obj().do_set_charset(set_name);
  }

  const char *
  test_case_factory
  ::get_charset()
  {
    return obj().do_get_charset();
  }


  const char *
  test_case_factory
  ::get_output_charset()
  {
    return obj().cli_->output_charset();
  }



  void
  test_case_factory
  ::manage_children(std::size_t max_pending_children, poll<fdreader> &poller)
  {
    while (num_pending_children_ >= max_pending_children)
      {
        int timeout_ms = deadlines_->ms_until_deadline();

        poll<fdreader>::descriptor desc = poller.wait(timeout_ms);

        if (desc.timeout())
          {
            timeboxed *t = deadlines_->remove_first();
            t->kill();
            continue;
          }
        bool read_failed = false;
        if (desc.read())
          {
            read_failed = !desc->read_data(!desc.hup());
          }
        if (read_failed || desc.hup())
          {
            desc->close();
            crpcut_test_case_registrator *r = desc->get_registrator();
            if (!r->has_active_readers())
              {
                r->manage_death();
                --num_pending_children_;
              }
          }
      }
  }


  void
  test_case_factory
  ::start_test(crpcut_test_case_registrator *i, poll<fdreader>& poller)
  {
    ++num_tests_run_;
    if (!tests_as_child_procs())
      {
        std::cout << *i << " ";
        i->run_test_case();
        std::cout << "OK\n";
        return;
      }

    pipe_pair c2p("communication pipe test-case to main process");
    pipe_pair p2c("communication pipe main process to test-case");
    pipe_pair stderr("communication pipe for test-case stderr");
    pipe_pair stdout("communication pipe for test-case stdout");

    i->set_wd(working_dirs_->allocate());
    pid_t pid;
    do { pid = wrapped::fork(); } while (pid == -1 && errno == EINTR);

    if (pid < 0) throw posix_error(errno, "fork test-case process");

    if (pid == 0) // child
      {
        wrapped::setpgid(0, 0);
        heap::control::enable();
        try {
          typedef comm::wfile_descriptor wfd;
          wfd report_fd(c2p.for_writing(pipe_pair::release_ownership));
          comm::report.set_writer(&report_fd);

          typedef comm::rfile_descriptor rfd;
          rfd response_fd(p2c.for_reading(pipe_pair::release_ownership));
          comm::report.set_reader(&response_fd);

          test_environment current_test;
          comm::report.set_test_environment(&current_test);
          wrapped::dup2(stdout.for_writing(), 1);
          wrapped::dup2(stderr.for_writing(), 2);
          stdout.close();
          stderr.close();
          p2c.close();
          c2p.close();
          current_pid_ = wrapped::getpid();
          i->goto_wd();
          i->run_test_case();
        }
        catch (...)
        {
            wrapped::_Exit(1);
        }
        wrapped::exit(0);
      }

    // parent
    ++num_pending_children_;
    i->setup(poller, pid,
             c2p.for_reading(pipe_pair::release_ownership),
             p2c.for_writing(pipe_pair::release_ownership),
             stdout.for_reading(pipe_pair::release_ownership),
             stderr.for_reading(pipe_pair::release_ownership));
  }

  void
  test_case_factory
  ::introduce_name(pid_t pid, const char *name, size_t len)
  {
    const comm::type t = comm::begin_test;
    const test_phase p = running;
    presenter_pipe_
      .write_loop(&pid)
      .write_loop(&t)
      .write_loop(&p)
      .write_loop(&len)
      .write_loop(name, len);
  }

  void
  test_case_factory
  ::kill_presenter_process()
  {
    comm::wfile_descriptor().swap(presenter_pipe_); // close
    siginfo_t info;
    for (;;)
      {
        int rv = wrapped::waitid(P_ALL, 0, &info, WEXITED);
        if (rv == -1 && errno == EINTR) continue;
        assert(rv == 0);
        break;
      }
  }


  int
  test_case_factory
  ::run_test(int argc, char *argv[], std::ostream &os)
  {
    return run_test(argc, const_cast<const char**>(argv), os);
  }

  int
  test_case_factory
  ::run_test(int, const char *argv[], std::ostream &os)
  {
    try {
      static cli::interpreter params(argv);
      return obj().do_run(&params, os, tag_list::obj());
    }
    catch (cli::param::exception &e)
    {
        os << e.what() << '\n';
    }
    return -1;
  }


  bool
  test_case_factory
  ::tests_as_child_procs()
  {
    return !obj().cli_->single_shot_mode();
  }

  bool
  test_case_factory
  ::timeouts_enabled()
  {
    return obj().cli_->honour_timeouts();
  }

  bool
  test_case_factory
  ::is_backtrace_enabled()
  {
#ifdef USE_BACKTRACE
    return obj().cli_->backtrace_enabled();
#else
    return false;
#endif
  }


  const char*
  test_case_factory
  ::get_start_dir()
  {
    return obj().do_get_start_dir();
  }

  const char*
  test_case_factory
  ::get_parameter(const char *name)
  {
    return obj().cli_->named_parameter(name);
  }

  const char*
  test_case_factory
  ::get_illegal_rep()
  {
    return obj().cli_->illegal_representation();
  }

  void
  test_case_factory
  ::test_succeeded(crpcut_test_case_registrator*)
  {
    ++num_successful_tests_;
  }

  test_case_factory&
  test_case_factory
  ::obj()
  {
    static test_case_factory f;
    return f;
  }

  void
  test_case_factory
  ::set_deadline(crpcut_test_case_registrator *i)
  {
    assert(i->deadline_is_set());
    deadlines_->insert(i);
  }

  void
  test_case_factory
  ::clear_deadline(crpcut_test_case_registrator *i)
  {
    assert(i->deadline_is_set());
    deadlines_->remove(i);
  }

  const char*
  test_case_factory
  ::do_get_start_dir() const
  {
    return homedir_;
  }

  void
  test_case_factory
  ::return_dir(unsigned num)
  {
    working_dirs_->free(num);
  }

  void
  test_case_factory
  ::do_set_charset(const char* set_name)
  {
    charset_ = set_name;
  }

  const char*
  test_case_factory
  ::do_get_charset() const
  {
    return charset_;
  }


  void
  test_case_factory
  ::present(pid_t       pid,
            comm::type  t,
            test_phase  phase,
            size_t      len,
            const char *buff)
  {
    presenter_pipe_
      .write_loop(&pid)
      .write_loop(&t)
      .write_loop(&phase)
      .write_loop(&len);
    if (len)
      {
        presenter_pipe_.write_loop(buff, len);
      }
  }


  void
  test_case_factory
  ::list_tests(const char *const*names,
               tag_list_root &tags,
               std::ostream &err_os)
  {
    if (*names && **names == '-')
      {
        err_os
        << "-l must be followed by a (possibly empty) test case list"
        "\n";
        throw cli_exception();
      }
    int longest_tag_len = tags.longest_tag_name();
    if (longest_tag_len > 0)
      {
        std::cout << ' ' << std::setw(longest_tag_len) << "tag"
        << " : test-name\n="
        << std::setfill('=')
        << std::setw(longest_tag_len)
        << "==="
        << "============\n"
        << std::setfill(' ');
      }
    for (crpcut_test_case_registrator *i = reg_.next();
        i != &reg_; i = i->next())
      {
        tag::importance importance = i->get_importance();

        if (importance == tag::ignored) continue;

        bool matched = !*names;
        for (const char *const*name = names; !matched && *name; ++name)
          {
            matched = i->match_name(*name);
          }
        if (matched)
          {
            std::cout << importance;
            if (longest_tag_len > 0)
              {
                std::cout << std::setw(longest_tag_len)
                << i->crpcut_tag().get_name().str
                << " : ";
              }
            std::cout << *i << '\n';
          }
      }
    throw cli_exception(0);
  }

  void configure_tags(const char *specification, crpcut::tag_list_root &tags)
  {
    if (specification == 0) return;
    tag_filter filter(specification);
    filter.assert_names(tags);
    // tag.end() refers to the defaulted nameless tag which
    // we want to include in this loop, hence the odd appearence
    tag_list::iterator ti = tags.begin();
    tag_list::iterator end = tags.end();
    do
      {
        tag::importance i = filter.lookup(ti->get_name());
        ti->set_importance(i);
      }
    while (ti++ != end);
  }

  unsigned
  make_tentative_test_list(test_case_factory::registrator_list &reg,
                           test_case_factory::registrator_list *tentative)
  {
    unsigned num_registered_tests = 0U;
    crpcut_test_case_registrator *next;
    for (crpcut_test_case_registrator *i = reg.next();
        i != &reg;
        i = next)
      {
        next = i->next();
        tag::importance importance = i->get_importance();
        if (importance == tag::ignored)
          {
            i->crpcut_uninhibit_dependants();
            i->unlink();
            continue;
          }
        if (importance == tag::disabled)
          {
            i->crpcut_uninhibit_dependants();
          }
        ++num_registered_tests;
        if (tentative)
          {
            i->unlink();
            i->link_after(*tentative);
          }
      }
    return num_registered_tests;
  }

  unsigned
  filter_out_tests_by_name(const char *const                   *names,
                           test_case_factory::registrator_list &from,
                           test_case_factory::registrator_list &to,
                           std::ostream                        &err_os)
  {
    unsigned num_selected_tests = 0U;
    unsigned mismatches = 0;
    typedef crpcut_test_case_registrator reg;
    for (const char *const*name = names; *name; ++name)
      {
        unsigned matches = 0;
        for (reg *i = from.next(); i != &from;)
          {
            reg *next = i->next();
            if (i->match_name(*name))
              {
                ++matches;
                ++num_selected_tests;
                i->unlink();
                i->link_after(to);
              }
            i = next;
          }
        if (matches == 0)
          {
            if (mismatches++)
              {
                err_os << ", ";
              }
            err_os << *name;
          }
      }
    if (mismatches)
      {
        err_os << (mismatches == 1 ? " does" : " do")
               << " not match any test names\n";
        throw cli_exception(-1);
      }
    for (reg *i = from.next(); i != &from; i = i->next())
      {
        i->crpcut_uninhibit_dependants();
      }
    return num_selected_tests;
  }

  std::pair<unsigned, unsigned>
  select_tests(const char *const *names,
               test_case_factory::registrator_list &reg,
               std::ostream &err_os)
  {
    test_case_factory::registrator_list tentative;
    unsigned num_registered_tests = make_tentative_test_list(reg,
                                                             *names
                                                             ? &tentative
                                                             : 0);
    unsigned num_selected_tests = *names
                                ? filter_out_tests_by_name(names,
                                                      tentative,
                                                      reg,
                                                      err_os)
                                : num_registered_tests;
    return std::make_pair(num_selected_tests, num_registered_tests);
  }

  void flush_output_buffer(int output_fd, output::buffer &buffer)
  {
    while (!buffer.is_empty())
      {
        std::pair<const char *, size_t> data = buffer.get_buffer();
        size_t bytes_written = 0;
        while (bytes_written < data.second)
          {
            ssize_t n = wrapped::write(output_fd,
                                       data.first + bytes_written,
                                       data.second - bytes_written);
            assert(n >= 0);
            bytes_written += size_t(n);
          }
        buffer.advance();
      }
  }

  void setup_dirbase(const char   *program_name,
                     const char   *working_dir,
                     char         *dirbase,
                     std::ostream &err_os)
  {
    if (!working_dir && !wrapped::mkdtemp(dirbase))
      {
        err_os << program_name
               << ": failed to create working directory\n";
        throw cli_exception(-1);
      }
    if (wrapped::chdir(dirbase) != 0)
      {
        err_os << program_name
               << ": couldn't move to working directoryy\n";
        wrapped::rmdir (dirbase);
        throw cli_exception(-1);
      }
  }


  void cleanup_directories(std::size_t        num_parallel,
                           const char        *working_dir,
                           const char        *dirbase,
                           comm::data_writer &presenter_pipe)
  {
    for (unsigned n = 0; n < num_parallel; ++n)
      {
        static const unsigned bindigits = std::numeric_limits<unsigned>::digits;
        stream::toastream<bindigits / 3 + 1> name;
        name << n << '\0';
        (void)wrapped::rmdir(name.begin());
        // failure above is taken care of as error elsewhere
      }

    if (!is_dir_empty("."))
      {
        static const pid_t dummy = 0;
        static const comm::type nonempty_dir = comm::dir;
        static const test_phase phase = post_mortem;
        static const size_t len = wrapped::strlen(dirbase) + 1;
        presenter_pipe
          .write_loop(&dummy)
          .write_loop(&nonempty_dir)
          .write_loop(&phase)
          .write_loop(&len)
          .write_loop(dirbase, len);
      }
    else if (working_dir == 0)
      {
        if (wrapped::chdir("..") < 0)
          {
            throw posix_error(errno,
                              "chdir back from testcase working dir");
          }
        (void)wrapped::rmdir(dirbase); // ignore, taken care of as error
      }
  }

  void report_blocked_tests(test_case_factory::registrator_list &reg,
                            comm::data_writer                   &presenter_pipe)
  {
    static const pid_t dummy_pid(0);
    static const comm::type begin_test = comm::begin_test;
    static const test_phase blocked = never_run;

    for (crpcut_test_case_registrator *i = reg.next();
         i != &reg;
         i = i->next())
      {
        const std::size_t name_len = i->full_name_len();
        char *buff = static_cast<char*>(alloca(name_len));
        stream::oastream os(buff, name_len);
        os << *i;
        const tag::importance importance = i->get_importance();
        presenter_pipe
          .write_loop(&dummy_pid)
          .write_loop(&begin_test)
          .write_loop(&blocked)
          .write_loop(&name_len)
          .write_loop(buff, name_len)
          .write_loop(&importance);
      }
  }

  bool test_case_factory
  ::schedule_tests(std::size_t num_parallel, poll<fdreader> &poller)
  {
    for (;;)
      {
        bool progress = false;
        typedef crpcut_test_case_registrator reg;
        reg *next;
        for (reg *i = reg_.next(); i != &reg_;i = next)
          {
            next = i->next();
            if (   (cli_->honour_dependencies() && !i->crpcut_can_run())
                || i->get_importance() == crpcut::tag::disabled)
              {
                continue;
              }
            progress = true;
            start_test(i, poller);
            manage_children(num_parallel, poller);
            i->unlink();
            if (!tests_as_child_procs())
              {
                return false;
              }
          }
        if (!progress)
          {
            if (num_pending_children_ == 0) break;
            manage_children(1, poller);
          }
      }
    if (num_pending_children_) manage_children(1, poller);
    return true;
  }

  int
  test_case_factory::do_run(cli::interpreter *cli,
                            std::ostream& err_os,
                            tag_list_root& tags)
  {
    cli_ = cli;
    try
      {
        const char *const *test_names = cli_->get_test_list();

        if (cli_->list_tags()) list_tags(tags);
        configure_tags(cli_->tag_specification(), tags);
        if (cli_->list_tests()) list_tests(test_names, tags, err_os);
        wrapped::getcwd(homedir_, sizeof(homedir_));
        if (cli_->working_dir())
          {
            lib::strcpy(dirbase_, cli_->working_dir());
          }
        std::pair<unsigned, unsigned> rv = select_tests(test_names,
                                                        reg_,
                                                        err_os);
        unsigned num_selected_tests = rv.first;
        unsigned num_registered_tests = rv.second;

        if (cli_->single_shot_mode() && num_selected_tests != 1U)
          {
            err_os << "Single shot requires exactly one test selected\n";
            throw cli_exception(-1);
          }

        std_exception_translator std_except_obj;
        c_string_translator c_string_obj;

        int output_fd = open_report_file(cli_->report_file(), err_os);

        using output::formatter;
        typedef output::text_formatter tf;
        typedef output::xml_formatter  xf;
        typedef output::nil_formatter  nf;

        output::heap_buffer buffer;
        formatter &fmt =
          select_output_formatter<xf, tf>(buffer,
                                          cli_->xml_output(),
                                          cli_->identity_string(),
                                          cli_->argv(),
                                          tags,
                                          num_registered_tests,
                                          num_selected_tests);
        output::heap_buffer summary_buffer;
        formatter &summary_fmt =
          select_output_formatter<nf, tf>(summary_buffer,
                                          cli_->quiet() || output_fd == 1,
                                          cli_->identity_string(),
                                          cli_->argv(),
                                          tags,
                                          num_registered_tests,
                                          num_selected_tests);

        if (tests_as_child_procs())
          {
            setup_dirbase(cli_->program_name(),
                          cli_->working_dir(),
                          dirbase_,
                          err_os);
            flush_output_buffer(output_fd, buffer);
            int p_fd = start_presenter_process(buffer,
                                               output_fd,
                                               fmt,
                                               summary_buffer,
                                               summary_fmt,
                                               cli_->verbose_mode(),
                                               dirbase_);
            comm::wfile_descriptor(p_fd).swap(presenter_pipe_);
          }
        const std::size_t num_parallel = cli_->num_parallel_tests();
        typedef poll_buffer_vector<fdreader> poll_reader;
        void *poll_memory = alloca(poll_reader::space_for(num_parallel*3U));
        poll_reader poller(poll_memory, num_parallel*3U);

        void *deadline_space = alloca(deadline_monitor::space_for(num_parallel));
        deadline_monitor deadlines(deadline_space, num_parallel);
        deadlines_ = &deadlines;

        void *wd_space = alloca(working_dir_allocator::space_for(num_parallel));
        working_dir_allocator dir_allocator(wd_space, num_parallel);
        working_dirs_ = &dir_allocator;

        if (!schedule_tests(num_parallel, poller)) return 0;

        if (tests_as_child_procs())
          {
            cleanup_directories(num_parallel,
                                cli_->working_dir(),
                                dirbase_,
                                presenter_pipe_);
            report_blocked_tests(reg_, presenter_pipe_);
            kill_presenter_process();
          }
        flush_output_buffer(output_fd, buffer);
        return int(num_tests_run_ - num_successful_tests_);
      }
    catch (cli_exception &e)
    {
      return e.return_code();
    }
    catch (std::runtime_error &e)
    {
      err_os << "Error: " << e.what() << "\nCan't continue\n";
    }
    catch (posix_error &e)
    {
      err_os << "Fatal error:" << e.what() << "\nCan't continue\n";
    }
    return -1;
  }

  unsigned long
  test_case_factory::calc_cputime(const struct timeval& t)
  {
    struct rusage usage;
    int rv = wrapped::getrusage(RUSAGE_CHILDREN, &usage);
    assert(rv == 0);
    struct timeval prev = accumulated_cputime_;
    timeradd(&usage.ru_utime, &usage.ru_stime, &accumulated_cputime_);
    struct timeval child_time;
    timersub(&accumulated_cputime_, &prev, &child_time);
    struct timeval child_test_time;
    timersub(&child_time, &t, &child_test_time);
    return (unsigned long)(((child_test_time.tv_sec))) * 1000UL
           + (unsigned long)(((child_test_time.tv_usec))) / 1000UL;
  }


}
