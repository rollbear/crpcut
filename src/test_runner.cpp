/*
 * Copyright 2011-2013 Bjorn Fahller <bjorn@fahller.se>
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

#include "test_runner.hpp"
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
#include "heap.hpp"

extern "C" {
#  include <sys/time.h>
#  include <fcntl.h>
}

namespace {

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

}

namespace crpcut {
  test_runner
  ::test_runner()
    : env_(0),
      cli_(0),
      num_pending_children_(0),
      presenter_pipe_(-1),
      deadlines_(0),
      working_dirs_(0)
  {
    lib::strcpy(dirbase_, "/tmp/crpcutXXXXXX");
  }

  test_runner
  ::~test_runner()
  {
  }



  void
  test_runner
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
            read_failed = !desc->read_data();
          }
        if (read_failed || desc.hup())
          {
            desc->close();
            crpcut_test_monitor *m = desc->get_monitor();
            if (!m->has_active_readers())
              {
                m->manage_death();
                --num_pending_children_;
              }
          }
      }
  }


  void
  test_runner
  ::start_test(crpcut_test_case_registrator *i, poll<fdreader>& poller)
  {
    pipe_pair c2p("communication pipe test-case to main process");
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
          crpcut_test_monitor::make_current(i);
          wrapped::dup2(stdout.for_writing(), 1);
          wrapped::dup2(stderr.for_writing(), 2);
          stdout.close();
          stderr.close();
          c2p.close();
          i->set_pid(wrapped::getpid());
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
    i->set_pid(pid);
    i->setup(poller,
             c2p.for_reading(pipe_pair::release_ownership),
             stdout.for_reading(pipe_pair::release_ownership),
             stderr.for_reading(pipe_pair::release_ownership));
    introduce_test(pid, i);
  }

  void
  test_runner
  ::introduce_test(pid_t pid, const crpcut_test_case_registrator *reg)
  {
    const comm::type t   = comm::begin_test;
    const test_phase p   = running;
    const size_t     len = sizeof(reg);
    presenter_pipe_
      .write_loop(&pid)
      .write_loop(&t)
      .write_loop(&p)
      .write_loop(&len)
      .write_loop(&reg, len);
  }

  int
  test_runner
  ::run_test(int argc, char *argv[], std::ostream &os)
  {
    return run_test(argc, const_cast<const char**>(argv), os);
  }

  int
  test_runner
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


  test_runner&
  test_runner
  ::obj()
  {
    static test_runner f;
    return f;
  }

  void
  test_runner
  ::set_deadline(crpcut_test_case_registrator *i)
  {
    assert(i->deadline_is_set());
    deadlines_->insert(i);
  }

  void
  test_runner
  ::clear_deadline(crpcut_test_case_registrator *i)
  {
    assert(i->deadline_is_set());
    deadlines_->remove(i);
  }


  void
  test_runner
  ::return_dir(unsigned num)
  {
    working_dirs_->free(num);
  }



  void
  test_runner
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
               << ": couldn't move to working directory\n";
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

  void test_runner
  ::schedule_tests(std::size_t num_parallel, poll<fdreader> &poller)
  {
    for (;;)
      {
        bool progress = false;
        typedef crpcut_test_case_registrator reg;
        reg *next;
        for (reg *i = reg_.next(); !reg_.is_this(i); i = next)
          {
            next = i->next();
            if (   (cli_->honour_dependencies() && !i->crpcut_can_run())
                || i->get_importance() == crpcut::tag::disabled)
              {
                continue;
              }
            progress = true;
            i->set_test_environment(env_);
            start_test(i, poller);
            manage_children(num_parallel, poller);
            i->unlink();
          }
        if (!progress)
          {
            if (num_pending_children_ == 0) break;
            manage_children(1, poller);
          }
      }
    if (num_pending_children_) manage_children(1, poller);
  }

  int
  test_runner::spawn_test_runner()
  {
    pipe_pair p("communication pipe for presenter process");

    pid_t pid = wrapped::fork();
    if (pid < 0)
      {
        throw posix_error(errno, "forking presenter process");
      }
    if (pid != 0)
      {
        return p.for_reading(pipe_pair::release_ownership);
      }
    comm::wfile_descriptor(p.for_writing()).swap(presenter_pipe_);

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

    schedule_tests(num_parallel, poller);

    cleanup_directories(num_parallel,
                        cli_->working_dir(),
                        dirbase_,
                        presenter_pipe_);
    wrapped::exit(0);
    return 0;
  }

  int
  test_runner::do_run(cli::interpreter *cli,
                      std::ostream& err_os,
                      tag_list_root& tags)
  {
    cli_ = cli;
    test_environment env(cli_);
    env_ = &env;
    try
      {
        const char *const *test_names = cli_->get_test_list();

        if (cli_->list_tags())
          {
            tags.print_to(std::cout);
            throw cli_exception(0);
          }
        tags.configure_importance(cli_->tag_specification());
        std::pair<unsigned, unsigned> rv =
            reg_.filter_out_or_throw(test_names, err_os, cli_exception(-1));
        unsigned num_selected_tests = rv.first;
        unsigned num_registered_tests = rv.second;

        if (cli_->list_tests())
          {
            reg_.list_tests_to(std::cout, tags.longest_tag_name());
            throw cli_exception(0);
          }
        if (cli_->working_dir())
          {
            lib::strcpy(dirbase_, cli_->working_dir());
          }
#ifdef USE_BACKTRACE
        if (cli_->backtrace_enabled())
          {
            heap::enable_backtrace();
          }
#endif
        if (cli_->single_shot_mode())
          {
            if (num_selected_tests != 1U)
              {
                err_os << "Single shot requires exactly one test selected\n";
                throw cli_exception(-1);
              }
            crpcut_test_case_registrator *i = reg_.next();
            i->set_test_environment(env_);
            std::cout << *i << " ";
            i->run_test_case();
            std::cout << "OK\n";
            return 0;
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

        setup_dirbase(cli_->program_name(),
                      cli_->working_dir(),
                      dirbase_,
                      err_os);
        int runner_fd = spawn_test_runner();
        unsigned num_failed = show_test_results(runner_fd,
                                                output_fd,
                                                buffer,
                                                fmt,
                                                summary_buffer,
                                                summary_fmt,
                                                cli_->verbose_mode(),
                                                dirbase_,
                                                reg_);
        siginfo_t info;
        wrapped::waitid(P_ALL, WEXITED, &info, 0);
        return int(num_failed);
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
  test_runner::calc_cputime(const struct timeval& t)
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
    return (unsigned long)(((child_test_time.tv_sec))) * 1000000UL
           + (unsigned long)(((child_test_time.tv_usec)));
  }


  test_environment*
  test_runner
  ::environment() const
  {
    return env_;
  }
}
