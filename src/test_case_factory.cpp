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

#include <crpcut.hpp>
#include "wrapped/posix_encapsulation.hpp"
#include "posix_error.hpp"
#include "tag_filter.hpp"
#include "poll_fixed_array.hpp"
#include "fsfuncs.hpp"
#include "pipe_pair.hpp"
#include "output/heap_buffer.hpp"
#include <algorithm>
#include "presentation.hpp"
#include "output/xml_formatter.hpp"
#include "output/text_formatter.hpp"
extern "C" {
#  include <sys/time.h>
#  include <fcntl.h>
}

namespace {

  inline const char *nullindex(const char* str, char needle)
  {
    if (str)
      {
        while (*str && *str != needle)
          ++str;
      }
    return str;
  }

  crpcut::output::formatter &output_formatter(crpcut::output::buffer &buffer,
                                              bool use_xml,
                                              const char *id,
                                              int argc, const char *argv[])
  {
    if (use_xml)
      {
        static crpcut::output::xml_formatter xo(buffer, id, argc, argv,
                                                crpcut::tag_list::obj());
        return xo;
      }
    static crpcut::output::text_formatter to(buffer, id, argc, argv,
                                             crpcut::tag_list::obj());
    return to;
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

}

namespace crpcut {
  test_case_factory
  ::test_case_factory()
    : pending_children(0),
      verbose_mode(false),
      enable_timeouts(true),
      nodeps(false),
      num_parallel(1),
      backtrace_enabled(false),
      num_registered_tests(0),
      num_selected_tests(0),
      num_tests_run(0),
      num_successful_tests(0),
      first_free_working_dir(0),
      charset("UTF-8"),
      output_charset(0),
      illegal_rep(0)
  {
    lib::strcpy(dirbase, "/tmp/crpcutXXXXXX");
    for (unsigned n = 0; n < max_parallel; ++n)
      {
        working_dirs[n] = n + 1;
      }
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
    return obj().do_get_output_charset();
  }



  void
  test_case_factory
  ::manage_children(unsigned max_pending_children, poll<fdreader> &poller)
  {
    while (pending_children >= max_pending_children)
      {
        int timeout_ms = timeouts_enabled() && deadlines.size()
          ? int(deadlines.front()->crpcut_ms_until_deadline())
          : -1;

        poll<fdreader>::descriptor desc = poller.wait(timeout_ms);

        if (desc.timeout())
          {
            assert(deadlines.size());
            crpcut_test_case_registrator *i = deadlines.front();
            std::pop_heap(deadlines.begin(), deadlines.end(),
                          &crpcut_test_case_registrator
                          ::crpcut_timeout_compare);
            deadlines.pop_back();
            i->crpcut_kill();
            continue;
          }
        bool read_failed = false;
        if (desc.read())
          {
            read_failed = !desc->read(!desc.hup());
          }
        if (read_failed || desc.hup())
          {
            desc->close();
            crpcut_test_case_registrator *r = desc->get_registrator();
            if (!r->crpcut_has_active_readers())
              {
                r->crpcut_manage_death();
                --pending_children;
              }
          }
      }
  }


  void
  test_case_factory
  ::start_test(crpcut_test_case_registrator *i, poll<fdreader>& poller)
  {
    ++num_tests_run;
    if (!tests_as_child_procs())
      {
        std::cout << *i << " ";
        i->crpcut_run_test_case();
        std::cout << "OK\n";
        return;
      }

    pipe_pair c2p("communication pipe test-case to main process");
    pipe_pair p2c("communication pipe main process to test-case");
    pipe_pair stderr("communication pipe for test-case stderr");
    pipe_pair stdout("communication pipe for test-case stdout");

    unsigned wd = first_free_working_dir;
    first_free_working_dir = working_dirs[wd];
    i->crpcut_set_wd(wd);

    ::pid_t pid;
    for (;;)
      {
        pid = wrapped::fork();
        if (pid < 0) throw posix_error(errno, "fork test-case process");
        if (pid >= 0) break;
        assert(errno == EINTR);
      }
    if (pid < 0) return;
    if (pid == 0) // child
      {
        wrapped::setpgid(0, 0);
        heap::control::enable();
        comm::report.set_fds(p2c.for_reading(pipe_pair::release_ownership),
                             c2p.for_writing(pipe_pair::release_ownership));
        wrapped::dup2(stdout.for_writing(), 1);
        wrapped::dup2(stderr.for_writing(), 2);
        stdout.close();
        stderr.close();
        p2c.close();
        c2p.close();
        current_pid = wrapped::getpid();
        i->crpcut_goto_wd();
        i->crpcut_run_test_case();
        wrapped::exit(0);
      }

    // parent
    ++pending_children;
    i->crpcut_setup(poller, pid,
                    c2p.for_reading(pipe_pair::release_ownership),
                    p2c.for_writing(pipe_pair::release_ownership),
                    stdout.for_reading(pipe_pair::release_ownership),
                    stderr.for_reading(pipe_pair::release_ownership));
    manage_children(num_parallel, poller);
  }

  void
  test_case_factory
  ::do_introduce_name(pid_t pid, const char *name, size_t len)
  {
    int pipe = presenter_pipe;
    for (;;)
      {
        ssize_t rv = wrapped::write(pipe, &pid, sizeof(pid));
        if (rv == sizeof(pid)) break;
        assert (rv == -1 && errno == EINTR);
      }
    const comm::type t = comm::begin_test;
    for (;;)
      {
        ssize_t rv = wrapped::write(pipe, &t, sizeof(t));
        if (rv == sizeof(t)) break;
        assert(rv == -1 && errno == EINTR);
      }
    const test_phase p = running;
    for (;;)
      {
        ssize_t rv = wrapped::write(pipe, &p, sizeof(p));
        if (rv == sizeof(p)) break;
        assert(rv == -1 && errno == EINTR);
      }

    for (;;)
      {
        ssize_t rv = wrapped::write(pipe, &len, sizeof(len));
        if (rv == sizeof(len)) break;
        assert(rv == -1 && errno == EINTR);
      }
    for (;;)
      {
        ssize_t rv = wrapped::write(pipe, name, len);
        if (size_t(rv) == len) break;
        assert(rv == -1 && errno == EINTR);
      }
  }



  void
  test_case_factory
  ::kill_presenter_process()
  {
    int rc = wrapped::close(presenter_pipe);
    assert(rc == 0);
    ::siginfo_t info;
    for (;;)
      {
        int rv = wrapped::waitid(P_ALL, 0, &info, WEXITED);
        if (rv == -1 && errno == EINTR) continue;
        assert(rv == 0);
        break;
      }
  }



  bool
  test_case_factory
  ::is_naughty_child()
  {
    return obj().current_pid != wrapped::getpid();
  }


  unsigned long
  test_case_factory
  ::calc_cputime(const struct timeval &t)
  {
    return obj().do_calc_cputime(t);
  }


  int
  test_case_factory
  ::run_test(int argc, char *argv[], std::ostream &os)
  {
    return obj().do_run(argc, const_cast<const char**>(argv), os);
  }

  int
  test_case_factory
  ::run_test(int argc, const char *argv[], std::ostream &os)
  {
    return obj().do_run(argc, argv, os);
  }


  void
  test_case_factory
  ::introduce_name(pid_t pid, const char *name, size_t len)
  {
    obj().do_introduce_name(pid, name, len);
  }

  void
  test_case_factory
  ::present(pid_t pid, comm::type t, test_phase phase,
            size_t len, const char *buff)
  {
    obj().do_present(pid, t, phase, len, buff);
  }

  bool
  test_case_factory
  ::tests_as_child_procs()
  {
    return obj().num_parallel > 0;
  }

  bool
  test_case_factory
  ::timeouts_enabled()
  {
    return obj().enable_timeouts;
  }

  bool
  test_case_factory
  ::is_backtrace_enabled()
  {
    return obj().backtrace_enabled;
  }

  void
  test_case_factory
  ::set_deadline(crpcut_test_case_registrator *i)
  {
    obj().do_set_deadline(i);
  }

  void
  test_case_factory
  ::clear_deadline(crpcut_test_case_registrator *i)
  {
    obj().do_clear_deadline(i);
  }

  void
  test_case_factory
  ::return_dir(unsigned num)
  {
    obj().do_return_dir(num);
  }

  const char *
  test_case_factory
  ::get_working_dir()
  {
    return obj().do_get_working_dir();
  }

  const char *
  test_case_factory
  ::get_start_dir()
  {
    return obj().do_get_start_dir();
  }

  const char *
  test_case_factory
  ::get_parameter(const char *name)
  {
    return obj().do_get_parameter(name);
  }

  const char *
  test_case_factory
  ::get_illegal_rep()
  {
    return obj().do_get_illegal_rep();
  }

  void
  test_case_factory
  ::test_succeeded(crpcut_test_case_registrator*)
  {
    ++obj().num_successful_tests;
  }

  test_case_factory &
  test_case_factory
  ::obj()
  {
    static test_case_factory f;
    return f;
  }

  void
  test_case_factory
  ::do_set_deadline(crpcut_test_case_registrator *i)
  {
    assert(i->crpcut_deadline_is_set());
    deadlines.push_back(i);
    std::push_heap(deadlines.begin(), deadlines.end(),
                   &crpcut_test_case_registrator
                   ::crpcut_timeout_compare);

  }


  void
  test_case_factory
  ::do_clear_deadline(crpcut_test_case_registrator *i)
  {
    assert(i->crpcut_deadline_is_set());
    typedef crpcut_test_case_registrator tcr;
    tcr **found = std::find(deadlines.begin(), deadlines.end(), i);
    assert(found != deadlines.end() && "clear deadline when none was ordered");

    size_t n = size_t(found - deadlines.begin());

    for (;;)
      {
        size_t m = (n+1)*2-1;
        if (m >= deadlines.size() - 1) break;
        if (tcr::crpcut_timeout_compare(deadlines[m+1],
                                        deadlines[m]))
          {
            deadlines[n] = deadlines[m];
          }
        else
          {
            deadlines[n] = deadlines[++m];
          }
        n = m;
      }
    deadlines[n] = deadlines.back();
    deadlines.pop_back();
    if (n != deadlines.size())
      {
        while (n && !tcr::crpcut_timeout_compare(deadlines[n],
                                                 deadlines[(n-1)/2]))
          {
            std::swap(deadlines[n], deadlines[(n-1)/2]);
            n=(n-1)/2;
          }
      }
  }

  const char *
  test_case_factory
  ::do_get_working_dir() const
  {
    return dirbase;
  }

  const char *
  test_case_factory
  ::do_get_start_dir() const
  {
    return homedir;
  }

  void
  test_case_factory
  ::do_return_dir(unsigned num)
  {
    working_dirs[num] = first_free_working_dir;
    first_free_working_dir = num;
  }

  void
  test_case_factory
  ::do_set_charset(const char *set_name)
  {
    charset = set_name;
  }

  const char *
  test_case_factory
  ::do_get_charset() const
  {
    return charset;
  }

  const char *
  test_case_factory
  ::do_get_output_charset() const
  {
    return output_charset;
  }

  const char *
  test_case_factory
  ::do_get_illegal_rep() const
  {
    return illegal_rep;
  }

  const char *
  test_case_factory
  ::do_get_parameter(const char *name) const
  {
    for (const char** p = argv; *p; ++p)
      {
        const char *pstart = 0;
        if ((*p)[0] == '-')
          {
            if ((*p)[1] == 'p')
              {
                pstart = *++p;
              }
            else if ((*p)[1] == '-')
              {
                const char *pb = (*p)+2;
                const char *n = nullindex(pb, '=');
                if (wrapped::strncmp("param", pb, size_t(n - pb)) == 0)
                  {
                    pstart = n + 1;
                  }
              }
          }
        if (pstart)
          {
            const char *v = pstart;
            const char *n = name;
            while (*n && *v == *n)
              {
                ++v;
                ++n;
              }
            if (*n == 0 && *v++ == '=')
              {
                return v;
              }
          }
      }
    return 0;
  }

  void
  test_case_factory
  ::do_present(pid_t       pid,
               comm::type  t,
               test_phase  phase,
               size_t      len,
               const char *buff)
  {
    int pipe = presenter_pipe;
    ssize_t rv = wrapped::write(pipe, &pid, sizeof(pid));
    assert(rv == sizeof(pid));
    rv = wrapped::write(pipe, &t, sizeof(t));
    assert(rv == sizeof(t));
    rv = wrapped::write(pipe, &phase, sizeof(phase));
    assert(rv == sizeof(phase));
    rv = wrapped::write(pipe, &len, sizeof(len));
    assert(rv == sizeof(len));
    if (len)
      {
        rv = wrapped::write(pipe, buff, len);
        assert(size_t(rv) == len);
      }
  }

  int
  test_case_factory
  ::do_run(int argc, const char *argv_[],
           std::ostream &err_os)
  {
    argv = argv_;
    const char  *working_dir       = 0;
    bool         quiet             = false;
    int          output_fd         = 1;
    bool         xml               = false;
    char         process_limit_set = 0;
    const char  *identity          = 0;
    const char **p                 = argv+1;

    try {

      while (const char *param = *p)
        {
          if (param[0] != '-') break;

          const char *value = *(p+1);
          char       cmd    = param[1];
          unsigned   pcount = 2;

          if (cmd == '-')
            {
              pcount = 1;
              param += 2;
              value = nullindex(param, '=');
              size_t len = size_t(value - param);
              if (*value)
                {
                  ++value;
                }
              else
                {
                  value = 0;
                }
              if (wrapped::strncmp("output-charset", param, len) == 0)
                {
                  cmd = 'C';
                }
              else if (wrapped::strncmp("list", param, len) == 0)
                {
                  cmd = 'l';
                }
              else if (wrapped::strncmp("list-tags", param, len) == 0)
                {
                  cmd = 'L';
                }
              else if (wrapped::strncmp("tags", param, len) == 0)
                {
                  cmd = 'T';
                }
              else if (wrapped::strncmp("illegal-char", param, len) == 0)
                {
                  cmd = 'I';
                }
              else if (wrapped::strncmp("identity", param, len) == 0)
                {
                  cmd = 'i';
                }
              else if (wrapped::strncmp("disable-timeouts", param, len) == 0)
                {
                  cmd = 't';
                }
              else if (wrapped::strncmp("verbose", param, len) == 0)
                {
                  cmd = 'v';
                }
              else if (wrapped::strncmp("children", param, len) == 0)
                {
                  cmd = 'c';
                }
              else if (wrapped::strncmp("xml", param, len) == 0)
                {
                  cmd = 'x';
                }
              else if (wrapped::strncmp("nodeps", param, len) == 0)
                {
                  cmd = 'n';
                }
              else if (wrapped::strncmp("quiet", param, len) == 0)
                {
                  cmd = 'q';
                }
              else if (wrapped::strncmp("output", param, len) == 0)
                {
                  cmd = 'o';
                }
              else if (wrapped::strncmp("single-shot", param, len) == 0)
                {
                  cmd = 's';
                }
              else if (wrapped::strncmp("working-dir", param, len) == 0)
                {
                  cmd = 'd';
                }
              else if (wrapped::strncmp("param", param, len) == 0)
                {
                  cmd = 'p';
                }
#ifdef USE_BACKTRACE
              else if (wrapped::strncmp("backtrace-heap", param, len) == 0)
                {
                  cmd = 'b';
                }
#endif // USE_BACKTRACE
            }
          switch (cmd) {
          case 'q':
            quiet = true;
            pcount = 1;
            break;
          case 'i':
            if (!value)
              {
                err_os << "-i must be followed by a string\n";
                return -1;
              }
            if (identity)
              {
                err_os << "-i may only be used once\n";
                return -1;
              }
            identity = value;
            break;
          case 'o':
            if (!value)
              {
                err_os << "-o must be followed by a filename\n";
                return -1;
              }
            output_fd = wrapped::open(value,
                                      O_CREAT | O_WRONLY | O_TRUNC,
                                      0666);
            if (output_fd < 0)
              {
                err_os << "Failed to open " << value << " for writing\n";
                return -1;
              }
            xml = !xml;
            break;
          case 'v':
            verbose_mode = true;
            pcount = 1;
            break;
          case 'c':
            if (process_limit_set)
              {
                err_os <<
                  "The number of child processes is already limited with the -"
                       << process_limit_set << "flag\n";
                return -1;
              }
            if (value)
              {
                stream::iastream i(value);
                unsigned l;
                if ((i >> l) && l <= max_parallel && l > 0)
                  {
                    num_parallel = l;
                    process_limit_set = 'c';
                    pcount = 2;
                    break;
                  }
              }
            err_os <<
              "num child processes must be a positive integer no greater than "
                   << max_parallel
                   << "\n";
            return -1;
          case 's':
            if (process_limit_set)
              {
                err_os <<
                  "The number of child processes is already limited with the -"
                       << process_limit_set << "flag\n";
                return -1;
              }
            pcount = 1;
            num_parallel = 0;
            nodeps = true;
            enable_timeouts = false;
            process_limit_set = 's';
            break;
          case 'L':
            {
              for (tag_list::iterator i = tag_list::begin();
                   i != tag_list::end();
                   ++i)
                {
                  std::cout << i->get_name().str << "\n";
                }
              return 0;
            }
          case 'T':
            {
              tag_filter filter(value);
              filter.assert_names(tag_list::obj());
              // tag_list::end refers to the defaulted nameless tag which
              // we want to include in this loop, hence the odd appearence
              tag_list::iterator ti = tag_list::begin();
              tag_list::iterator end = tag_list::end();
              do
                {
                  tag::importance i = filter.lookup(ti->get_name());
                  ti->set_importance(i);
                } while(ti++ != end);
              break;
            }
          case 'l':
            {
              const char **names = ++p;
              if (*names && **names == '-')
                {
                  err_os <<
                    "-l must be followed by a (possibly empty) test case list"
                    "\n";
                  return -1;
                }
              int longest_tag_len = tag_list::longest_name_len();
              if (longest_tag_len > 0)
                {
                  std::cout
                    << ' ' << std::setw(longest_tag_len) << "tag"
                    << " : test-name\n="
                    << std::setfill('=') << std::setw(longest_tag_len) << "==="
                    << "============\n" << std::setfill(' ');
                }
              for (crpcut_test_case_registrator *i
                     = reg.crpcut_get_next();
                   i != &reg;
                   i = i->crpcut_get_next())
                {
                  tag &test_tag = i->crpcut_tag();
                  tag::importance importance = test_tag.get_importance();
                  if (importance == tag::ignored) continue;
                  const char prefix = importance == tag::critical ? '!' : '?';
                  bool matched = !*names;
                  for (const char **name = names; !matched && *name; ++name)
                    {
                      matched = i->crpcut_match_name(*name);
                    }
                  if (matched)
                    {
                      std::cout << prefix;
                      if (longest_tag_len > 0)
                        {
                          std::cout
                            << std::setw(longest_tag_len)
                            << i->crpcut_tag().get_name().str << " : ";
                        }
                      std::cout << *i << '\n';
                    }
                }
              return 0;
            }
          case 'd':
            working_dir = value;
            if (!working_dir)
              {
                err_os << "-d must be followed by a directory name\n";
                return -1;
              }
            lib::strcpy(dirbase, working_dir);
            break;
          case 'n':
            nodeps = true;
            pcount = 1;
            break;
          case 't':
            enable_timeouts = false;
            pcount = 1;
            break;
          case 'x':
            if (value && value[0] != '-')
              {
                if (wrapped::strcmp(value, "yes") == 0 ||
                    wrapped::strcmp(value, "Yes") == 0 ||
                    wrapped::strcmp(value, "YES") == 0)
                  {
                    xml = true;
                  }
                else if (wrapped::strcmp(value, "no") == 0 ||
                         wrapped::strcmp(value, "No") == 0 ||
                         wrapped::strcmp(value, "NO") == 0)
                  {
                    xml = false;
                  }
                else
                  {
                    err_os << "expected boolean value for --xml\n";
                    return -1;
                  }
              }
            else
              {
                pcount = 1;
                xml = !xml;
              }
            break;
          case 'p':
            // just make a syntax check here. What follows must be a name=value.
            {
              const char *n = nullindex(value, '=');
              if (!n || *n == 0)
                {
                  err_os << "-p must be followed by a name and =\n";
                  return -1;
                }
              break;
            }
#ifdef USE_BACKTRACE
          case 'b':
            {
              backtrace_enabled = true;
              pcount = 1;
              break;
            }
#endif // USE_BACKTRACE
          case 'C':
            {
              output_charset = value;
              break;
            }
          case 'I':
            {
              illegal_rep = value;
              break;
            }
          case 'V':
            {
              err_os <<
                "crpcut-" << CRPCUT_VERSION_STRING << "\n";
              return -1;
            }
          default:
            err_os <<
              "Usage: " << argv[0] << " [flags] {testcases}\n"
              "  where flags can be:\n"
#ifdef USE_BACKTRACE
              "   -b, --backtrace-heap\n"
              "        store stack backtrace for all heap objects for\n"
              "        better error pinpointing of heap violations (slow)\n\n"
#endif // USE_BACKTRACE
              "   -c number, --children=number\n"
              "        control number of concurrently running test processes\n"
              "        number must be >= 1 and <= "
                        << max_parallel << "\n\n"
              "   -C charset, --output-charset=charset\n"
              "        specify the output character set to convert text-output\n"
              "        to. Does not apply for XML-output\n\n"
              "   -d dir, --working-dir=dir\n"
              "        specify working directory (must exist)\n\n"
              "   -i \"id string\", --identity=\"id string\"\n"
              "        specify an identity string for the XML-header\n\n"
              "   -I string, --illegal-char=string\n"
              "        specify how characters that are illegal for the chosen\n"
              "        output character set are to be represented\n\n"
              "   -l, --list\n"
              "        list test cases\n\n"
              "   -L, --list-tags\n"
              "        list all tags used by tests in the test program\n\n"
              "   -n, --nodeps\n"
              "        ignore dependencies\n\n"
              "   -o file, --output=file\n"
              "        direct XML output to named file. A brief summary will be\n"
              "        displayed on stdout\n\n"
              "   -p name=val, --param=name=val\n"
              "        define a named variable for the test cases to pick up\n\n"
              "   -q, --quiet\n"
              "        don't display the -o brief summary\n\n"
              "   -s, --single-shot\n"
              "        run only one test case, and run it in the main process\n\n"
              "   -t, --disable-timeouts\n"
              "        never fail a test due to time consumption\n\n"
              "   -T {select}{/non-critical}, --tags={select}{/non-critical}\n"
              "        Select tests to run based on their tag, and which\n"
              "        tags represent non-critical tests. Both \"select\"\n"
              "        and \"non-critical\" are comma separated lists of tags.\n"
              "        Both lists can be empty. If a list begin with \"-\",\n"
              "        the list is subtractive from the full set.\n"
              "        Untagged tests cannot be made non-critical.\n\n"
              "   -v, --verbose\n"
              "        verbose mode, print result from passed tests\n\n"
              "   -V, --version\n"
              "        print version string and exit\n\n"
              "   -x, --xml\n"
              "        XML output on std-out or non-XML output on file\n";

            return -1;
          }
          p += pcount;
        }

      if (output_charset && xml)
        {
          err_os <<
            "-C / --output-charset cannot be used with XML output, since the\n"
            "output charset is always UTF-8 in crpcut XML reports.\n";
          return -1;
        }
      wrapped::getcwd(homedir, sizeof(homedir));
      registrator_list tentative;
      {
        crpcut_test_case_registrator *i = reg.crpcut_get_next();
        while (i != &reg)
          {
            const tag& test_tag = i->crpcut_tag();
            crpcut_test_case_registrator *next = i->crpcut_get_next();
            if (test_tag.get_importance() == tag::ignored)
              {
                i->crpcut_uninhibit_dependants();
                i->crpcut_unlink();
                i = next;
                continue;
              }
            ++num_registered_tests;
            if (*p)
              {
                i->crpcut_unlink();
                i->crpcut_link_after(&tentative);
              }
            i = next;
          }
      }
      unsigned mismatches = 0;
      if (*p == 0)
        {
          num_selected_tests = num_registered_tests;
        }
      else
        {
          for (const char **name = p; *name; ++name)
            {
              crpcut_test_case_registrator *i = tentative.crpcut_get_next();
              unsigned matches = 0;
              while (i != &tentative)
                {
                  if (i->crpcut_match_name(*name))
                    {
                      ++matches;
                      ++num_selected_tests;
                      crpcut_test_case_registrator *next = i->crpcut_unlink();
                      i->crpcut_link_after(&reg);
                      i = next;
                    }
                  else
                    {
                      i = i->crpcut_get_next();
                    }
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
        }
      if (mismatches)
        {
          err_os << (mismatches == 1 ? " does" : " do")
                 << " not match any test names\n";
          return -1;
        }
      if (num_parallel == 0 && num_selected_tests != 1)
        {
          err_os << "Single shot requires exactly one test selected\n";
          return -1;
        }

      {
        crpcut_test_case_registrator *i = tentative.crpcut_get_next();
        while (i != &tentative)
          {
            i->crpcut_uninhibit_dependants();
            i = i->crpcut_get_next();
          }
      }

      std_exception_translator std_except_obj;
      c_string_translator c_string_obj;

      output::heap_buffer buffer;
      output::formatter &fmt = output_formatter(buffer,
                                                xml,
                                                identity,
                                                argc, argv);

      if (tests_as_child_procs())
        {
          if (!working_dir && !wrapped::mkdtemp(dirbase))
            {
              err_os << argv[0] << ": failed to create working directory\n";
              return 1;
            }
          if (wrapped::chdir(dirbase) != 0)
            {
              err_os << argv[0] << ": couldn't move to working directoryy\n";
              wrapped::rmdir(dirbase);
              return 1;
            }
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
                  bytes_written+= size_t(n);
                }
              buffer.advance();
            }
          presenter_pipe = start_presenter_process(buffer,
                                                   output_fd,
                                                   fmt,
                                                   verbose_mode);
        }
      poll_fixed_array<fdreader, max_parallel*3> poller;
      for (;;)
        {
          bool progress = false;
          crpcut_test_case_registrator *i = reg.crpcut_get_next();
          while (i != &reg)
            {
              if (!nodeps && !i->crpcut_can_run())
                {
                  i = i->crpcut_get_next();
                  continue;
                }
              progress = true;
              start_test(i, poller);
              i = i->crpcut_unlink();
              if (!tests_as_child_procs())
                {
                  return 0;
                }
            }
          if (!progress)
            {
              if (pending_children == 0)
                {
                  break;
                }
              manage_children(1, poller);
            }
        }
      if (pending_children) manage_children(1, poller);
      if (tests_as_child_procs())
        {
          kill_presenter_process();
          for (unsigned n = 0; n < max_parallel; ++n)
            {
              stream::toastream<std::numeric_limits<unsigned>::digits/3+1> name;
              name << n << '\0';
              (void)wrapped::rmdir(name.begin());
              // failure above is taken care of as error elsewhere
            }

          if (!is_dir_empty("."))
            {
              fmt.nonempty_dir(dirbase);
              if (output_fd != 1 && !quiet)
                {
                  std::cout << "Files remain in " << dirbase << '\n';
                }
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

      if (reg.crpcut_get_next() != &reg)
        {
          if (output_fd != 1 && !quiet)
            {
              std::cout << "Blocked tests:\n";
            }
          for (crpcut_test_case_registrator *i
                 = reg.crpcut_get_next();
               i != &reg;
               i = i->crpcut_get_next())
            {
              std::size_t name_len = i->crpcut_full_name_len();
              char *buff = static_cast<char*>(alloca(name_len));
              stream::oastream os(buff, name_len);
              os << *i;
              fmt.blocked_test(os);
              if (output_fd != 1 && !quiet)
                {
                  std::cout << "  " << *i << '\n';
                }
            }
        }

      fmt.statistics(num_registered_tests,
                     num_selected_tests,
                     num_tests_run,
                     num_tests_run - num_successful_tests);
      if (output_fd != 1 && !quiet)
        {
          size_t sum_crit_pass = 0;
          size_t sum_crit_fail = 0;
          std::cout << num_selected_tests
                    << " test cases selected\n";
          const tag_list::iterator begin(tag_list::begin());
          const tag_list::iterator end(tag_list::end());
          if (begin != end)
            {
              bool header_displayed = false;
              for (tag_list::iterator i = begin; i != end; ++i)
                {
                  if (!i->get_name()) continue;
                  if (i->num_passed() + i->num_failed() == 0) continue;
                  if (!header_displayed)
                    {
                      std::cout
                        << ' '
                        << std::setw(tag_list::longest_name_len()) << "tag"
                        << std::setw(8) << "total"
                        << std::setw(8) << "passed"
                        << std::setw(8) << "failed" << '\n';

                      header_displayed = true;
                    }
                  if (i->get_importance() == tag::critical)
                    {
                      sum_crit_pass+= i->num_passed();
                      sum_crit_fail+= i->num_failed();
                    }
                  char flag = i->get_importance() == tag::critical ? '!' : '?';
                  std::cout
                    << flag
                    << std::setw(tag_list::longest_name_len()) << i->get_name()
                    << std::setw(8) << i->num_failed() + i->num_passed()
                    << std::setw(8) << i->num_failed()
                    << std::setw(8) << i->num_passed() << '\n';
                }
            }

          std::cout << "\n           "
                    << std::setw(8) << "Sum"
                    << std::setw(11) << "Critical"
                    << std::setw(15) << "Non-critical";
          std::cout << "\nPASSED   : "
                    << std::setw(8) << num_successful_tests
                    << std::setw(11) << sum_crit_pass
                    << std::setw(15) << num_successful_tests - sum_crit_pass
                    << "\nFAILED   : "
                    << std::setw(8) << num_tests_run - num_successful_tests
                    << std::setw(11) << sum_crit_fail
                    << std::setw(15)
                    << num_tests_run - num_successful_tests - sum_crit_fail
                    << '\n';
          if (num_selected_tests != num_tests_run)
            {
              std::cout << "UNTESTED : "
                        << std::setw(8) << num_selected_tests - num_tests_run
                        << '\n';
            }
        }
      while (!buffer.is_empty())
        {
          std::pair<const char *, size_t> data = buffer.get_buffer();
          const char *buff = data.first;
          const size_t len = data.second;
          size_t bytes_written = 0;
          while (bytes_written < len)
            {
              ssize_t n = wrapped::write(output_fd, buff + bytes_written, len - bytes_written);
              assert(n >= 0);
              bytes_written += size_t(n);
            }
          buffer.advance();
        }
      return int(num_tests_run - num_successful_tests);
    }
    catch (std::runtime_error &e)
      {
        err_os << "Error: " << e.what() << "\nCan't continue\n";
      }
    catch (posix_error &e)
      {
        err_os << "Fatal error:"
               << e.what()
               << "\nCan't continue\n";
      }
    return -1;
  }

  unsigned long
  test_case_factory
  ::do_calc_cputime(const struct timeval &t)
  {
    struct rusage usage;
    int rv = wrapped::getrusage(RUSAGE_CHILDREN, &usage);
    assert(rv == 0);
    struct timeval prev = accumulated_cputime;
    timeradd(&usage.ru_utime, &usage.ru_stime, &accumulated_cputime);
    struct timeval child_time;
    timersub(&accumulated_cputime, &prev, &child_time);
    struct timeval child_test_time;
    timersub(&child_time, &t, &child_test_time);
    return (unsigned long)(child_test_time.tv_sec) * 1000UL
      + (unsigned long)(child_test_time.tv_usec)/1000UL;
  }


}
