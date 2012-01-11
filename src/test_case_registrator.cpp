/*
 * Copyright 2009-2012 Bjorn Fahller <bjorn@fahller.se>
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
#include "clocks/clocks.hpp"
#include "fsfuncs.hpp"
extern "C" {
#include <sys/time.h>
}


#include <cassert>
#include <limits>


namespace crpcut {

  crpcut_test_case_registrator *
  crpcut_test_case_registrator
  ::crpcut_unlink()
  {
    crpcut_next->crpcut_prev = crpcut_prev;
    crpcut_prev->crpcut_next = crpcut_next;
    return crpcut_next;
  }

  void
  crpcut_test_case_registrator::
  crpcut_link_after(crpcut_test_case_registrator *r)
  {
    crpcut_next = r->crpcut_next;
    crpcut_prev = r;
    crpcut_next->crpcut_prev = this;
    r->crpcut_next = this;
  }

  bool
  crpcut_test_case_registrator
  ::crpcut_deadline_is_set() const
  {
    return crpcut_deadline_set;
  }

  bool
  crpcut_test_case_registrator
  ::crpcut_timeout_compare(const crpcut_test_case_registrator *lh,
                           const crpcut_test_case_registrator *rh)
  {
    assert(lh->crpcut_deadline_set);
    assert(rh->crpcut_deadline_set);

    long diff
      = long(lh->crpcut_absolute_deadline_ms
             - rh->crpcut_absolute_deadline_ms);
    return diff > 0;
  }

  crpcut_test_case_registrator *
  crpcut_test_case_registrator
  ::crpcut_get_next() const
  {
    return crpcut_next;
  }

  pid_t
  crpcut_test_case_registrator
  ::crpcut_get_pid() const
  {
    return crpcut_pid_;
  }

  test_phase
  crpcut_test_case_registrator
  ::crpcut_get_phase() const
  {
    return crpcut_phase;
  }

  bool
  crpcut_test_case_registrator
  ::crpcut_has_active_readers() const
  {
    return crpcut_active_readers > 0U;
  }

  void
  crpcut_test_case_registrator
  ::crpcut_deactivate_reader()
  {
    --crpcut_active_readers;
  }

  void
  crpcut_test_case_registrator
  ::crpcut_activate_reader()
  {
    ++crpcut_active_readers;
  }

  void
  crpcut_test_case_registrator
  ::crpcut_set_timeout(unsigned long ts)
  {
    crpcut_absolute_deadline_ms = crpcut_phase == running
      ? crpcut_calc_deadline(ts)
      : ts;
    crpcut_deadline_set = true;
  }

  void
  crpcut_test_case_registrator
  ::crpcut_run_test_case()
  {
    try {
      crpcut_do_run_test_case();
    }
    catch (...)
      {
        heap::set_limit(heap::system);
        std::ostringstream out;
        out << "Unexpectedly caught "
            << policies::crpcut_exception_translator::try_all();

        comm::report(comm::exit_fail, out);
      }
  }

  crpcut_test_case_registrator
  ::crpcut_test_case_registrator()
    : crpcut_next(this),
      crpcut_prev(this),
      crpcut_active_readers(0),
      crpcut_killed(false),
      crpcut_death_note(false),
      crpcut_deadline_set(false),
      crpcut_rep_reader(0),
      crpcut_stdout_reader(0),
      crpcut_stderr_reader(0),
      crpcut_cputime_limit_ms(0)
  {
  }

  crpcut_test_case_registrator
  ::crpcut_test_case_registrator(const char *name,
                                 const namespace_info &ns,
                                 unsigned long cputime_timeout_ms)
    : crpcut_name_(name),
      crpcut_ns_info(&ns),
      crpcut_next(&test_case_factory::obj().reg),
      crpcut_prev(test_case_factory::obj().reg.crpcut_prev),
      crpcut_death_note(false),
      crpcut_rep_reader(this),
      crpcut_stdout_reader(this),
      crpcut_stderr_reader(this),
      crpcut_phase(creating),
      crpcut_cputime_limit_ms(cputime_timeout_ms)
  {
    test_case_factory::obj().reg.crpcut_prev = this;
    crpcut_prev->crpcut_next = this;
  }

  void
  crpcut_test_case_registrator
  ::crpcut_prepare_construction(unsigned long deadline)
  {
    if (test_case_factory::tests_as_child_procs())
      {
        comm::report(comm::set_timeout, deadline);
      }
  }

  void
  crpcut_test_case_registrator
  ::crpcut_prepare_destruction(unsigned long deadline)
  {
    if (test_case_factory::tests_as_child_procs())
      {
        comm::report(comm::set_timeout, deadline);
      }
  }

  bool
  crpcut_test_case_registrator
  ::crpcut_match_name(const char *name_param) const
  {
    const char *p = crpcut_ns_info->match_name(name_param);
    if (p)
      {
        if (p != name_param || *p == ':')
          {
            if (!*p) return true; // match for whole suites
            if (!*p++ == ':') return false;
            if (!*p++ == ':') return false;
          }
      }
    else
      {
        p = name_param;
      }
    return crpcut::wrapped::strcmp(p, crpcut_name_) == 0;
  }

  std::size_t
  crpcut_test_case_registrator
  ::crpcut_full_name_len() const
  {
    return crpcut_ns_info->full_name_len()
      + 2
      + wrapped::strlen(crpcut_name_);
  }

  std::ostream &
  crpcut_test_case_registrator
  ::crpcut_print_name(std::ostream &os) const
  {
    os << *crpcut_ns_info;
    return os << crpcut_name_;
  }

  void
  crpcut_test_case_registrator
  ::crpcut_manage_test_case_execution(test_case_base* p)
  {
    if (test_case_factory::tests_as_child_procs())
      {
        struct rusage usage;
        int rv = wrapped::getrusage(RUSAGE_SELF, &usage);
        assert(rv == 0);
        struct timeval cputime;
        timeradd(&usage.ru_utime, &usage.ru_stime, &cputime);
        comm::report(comm::begin_test, cputime);
      }
    try {
      p->run();
    }
    catch (...)
      {
        heap::set_limit(heap::system);
        std::ostringstream out;
        out << "Unexpectedly caught "
            << policies::crpcut_exception_translator::try_all();
        comm::report(comm::exit_fail, out);
      }
    if (!test_case_factory::tests_as_child_procs())
      {
        crpcut_register_success();
      }
  }

  void
  crpcut_test_case_registrator
  ::crpcut_kill()
  {
    assert(crpcut_pid_);
    wrapped::killpg(crpcut_pid_, SIGKILL);
    crpcut_killed = true;
  }

  unsigned long
  crpcut_test_case_registrator
  ::crpcut_ms_until_deadline() const
  {
    clocks::monotonic::timestamp now
      = clocks::monotonic::timestamp_ms_absolute();
    unsigned long diff = crpcut_absolute_deadline_ms - now;
    return long(diff) < 0 ? 0UL : diff;
  }

  void
  crpcut_test_case_registrator
  ::crpcut_clear_deadline()
  {
    assert(crpcut_deadline_is_set());
    test_case_factory::clear_deadline(this);
    crpcut_deadline_set = false;
  }

  void
  crpcut_test_case_registrator
  ::crpcut_setup(poll<fdreader> &poller,
                 pid_t pid,
                 int in_fd, int out_fd,
                 int stdout_fd,
                 int stderr_fd)
  {
    crpcut_pid_ = pid;
    crpcut_stdout_reader.set_fd(stdout_fd, &poller);
    crpcut_stderr_reader.set_fd(stderr_fd, &poller);
    crpcut_rep_reader.set_fds(in_fd, out_fd, &poller);
    stream::toastream<1024> os;
    os << *this;
    test_case_factory::introduce_name(pid, os.begin(), os.size());
  }

  void
  crpcut_test_case_registrator::crpcut_set_wd(unsigned n)
  {
    crpcut_dirnum = n;
    stream::toastream<std::numeric_limits<int>::digits/3+1> name;
    name << n << '\0';
    if (wrapped::mkdir(name.begin(), 0700) != 0)
      {
        assert(errno == EEXIST);
      }
  }

  void
  crpcut_test_case_registrator
  ::crpcut_goto_wd() const
  {
    stream::toastream<std::numeric_limits<int>::digits/3+1> name;
    name << crpcut_dirnum << '\0';
    if (wrapped::chdir(name.begin()) != 0)
      {
        comm::report(comm::exit_fail, "Couldn't chdir working dir");
        assert("unreachable code reached" == 0);
      }
  }

  bool
  crpcut_test_case_registrator
  ::crpcut_cputime_timeout(unsigned long ms) const
  {
    return test_case_factory::timeouts_enabled()
      && crpcut_cputime_limit_ms
      && ms > crpcut_cputime_limit_ms;
  }

  void
  crpcut_test_case_registrator
  ::crpcut_manage_death()
  {
    typedef test_case_factory tcf;
    ::siginfo_t info;
    for (;;)
      {
        ::siginfo_t local;
        int rv = wrapped::waitid(P_PGID, id_t(crpcut_pid_), &local, WEXITED);
        int n = errno;
        if (rv == -1 && n == EINTR) continue;
        if (local.si_pid == crpcut_pid_) info = local;
        if (rv == 0) continue;
        break;
      }
    if (!crpcut_killed && crpcut_deadline_is_set())
      {
        crpcut_clear_deadline();
      }
    unsigned long cputime_ms = tcf::calc_cputime(crpcut_cpu_time_at_start);
    comm::type t = comm::exit_ok;
    stream::toastream<std::numeric_limits<int>::digits/3+1> dirname;
    dirname << crpcut_dirnum << '\0';

    stream::toastream<1024> out;
    if (!crpcut_death_note)
      {
        switch (info.si_code)
          {
          case CLD_EXITED:
            {
              if (!crpcut_failed())
                {
                  if (crpcut_is_expected_exit(info.si_status))
                    {
                      crpcut_on_ok_action(dirname.begin());
                    }
                  else
                    {
                      crpcut_register_success(false);
                      out << "Exited with code "
                          << info.si_status << "\nExpected ";
                      crpcut_expected_death(out);
                      t = comm::exit_fail;
                    }
                }
            }
            break;
          case CLD_KILLED:
            {
              if (!crpcut_failed())
                {
                  if (crpcut_is_expected_signal(info.si_status))
                    {
                      if (crpcut_cputime_timeout(cputime_ms))
                        {
                          crpcut_register_success(false);
                          out << "Test consumed "
                              << cputime_ms << "ms CPU-time\nLimit was "
                              << crpcut_cputime_limit_ms << "ms";
                          t = comm::exit_fail;
                        }
                      else
                        {
                          crpcut_on_ok_action(dirname.begin());
                        }
                    }
                  else
                    {
                      crpcut_register_success(false);
                      if (crpcut_killed)
                        {
                          out << "Timed out - killed";
                        }
                      else
                        {
                          out << "Died on signal "
                              << info.si_status;
                        }
                      out << "\nExpected ";
                      crpcut_expected_death(out);
                      t = comm::exit_fail;
                    }
                }
            }
            break;
          case CLD_DUMPED:
            out << "Died with core dump";
            t = comm::exit_fail;
            crpcut_register_success(false);
            break;
          default:
            out << "Died for unknown reason, code=" << info.si_code;
            crpcut_register_success(false);
            t = comm::exit_fail;
          }
        crpcut_death_note = true;
      }
    if (!is_dir_empty(dirname.begin()))
      {
        if (!crpcut_failed()) crpcut_phase = post_mortem;
        stream::toastream<1024> tcname;
        tcname << *this << '\0';
        tcf::present(crpcut_pid_, comm::dir, crpcut_phase, 0, 0);
        wrapped::rename(dirname.begin(), tcname.begin());
        t = comm::exit_fail;
        crpcut_register_success(false);
      }
    tcf::present(crpcut_pid_, t, crpcut_phase, out.size(), out.begin());
    crpcut_register_success(t == comm::exit_ok);
    tcf::return_dir(crpcut_dirnum);
    bool critical = crpcut_tag().get_importance() == tag::critical;
    tcf::present(crpcut_pid_,
                 comm::end_test,
                 crpcut_phase,
                 sizeof(critical), (const char*)&critical);
    assert(crpcut_succeeded() || crpcut_failed());
    if (crpcut_succeeded())
      {
        crpcut_tag().pass();
        tcf::test_succeeded(this);
      }
    else
      {
        crpcut_tag().fail();
      }
  }

} // namespace crpcut
