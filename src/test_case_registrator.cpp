/*
 * Copyright 2009-2013 Bjorn Fahller <bjorn@fahller.se>
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
#include "test_runner.hpp"
#include "wrapped/posix_encapsulation.hpp"
#include "filesystem_operations.hpp"
#include "process_control.hpp"
#include "clocks/clocks.hpp"
#include "fsfuncs.hpp"
extern "C" {
#include <sys/time.h>
}


#include <cassert>
#include <limits>

namespace {
  ::siginfo_t get_siginfo(pid_t pid, crpcut::process_control *process)
  {
    ::siginfo_t info;
    for (;;)
      {
        ::siginfo_t local;
        int rv = process->waitid(P_PGID, id_t(pid), &local, WEXITED);
        int n = errno;
        if (rv == -1 && n == EINTR) continue;
        if (local.si_pid == pid) info = local;
        if (rv == 0) continue;
        break;
      }
    return info;
  }

  void set_location(std::ostream& os, crpcut::datatypes::fixed_string location)
  {
    assert(location.len);
    const void *len_addr = &location.len;
    os.write(static_cast<const char*>(len_addr), sizeof(location.len));
    os.write(location.str, std::streamsize(location.len));
  }
}

namespace crpcut {

  pid_t
  crpcut_test_case_registrator
  ::get_pid() const
  {
    return pid_;
  }

  test_phase
  crpcut_test_case_registrator
  ::get_phase() const
  {
    return phase_;
  }

  void
  crpcut_test_case_registrator
  ::set_phase(test_phase p)
  {
    phase_ = p;
  }

  bool
  crpcut_test_case_registrator
  ::has_death_note() const
  {
    return death_note_;
  }

  void
  crpcut_test_case_registrator
  ::set_death_note()
  {
    clear_deadline();
    death_note_ = true;
  }

  datatypes::fixed_string
  crpcut_test_case_registrator
  ::get_location() const
  {
    return location_;
  }

  void
  crpcut_test_case_registrator
  ::send_to_presentation(comm::type t, size_t len, const char *buff) const
  {
    runner_->present(get_pid(), t, get_phase(), len, buff);
  }

  bool
  crpcut_test_case_registrator
  ::has_active_readers() const
  {
    return active_readers_ > 0U;
  }

  void
  crpcut_test_case_registrator
  ::deactivate_reader()
  {
    --active_readers_;
  }

  void
  crpcut_test_case_registrator
  ::activate_reader()
  {
    ++active_readers_;
  }

  void
  crpcut_test_case_registrator
  ::set_timeout(unsigned long ts_us)
  {
    clocks::monotonic::timestamp now = clocks::monotonic::timestamp_absolute();
    ts_us+= now;
    if (phase_ == creating)
      {
        real_time_at_start_ = now;
      }
    if (!env_->timeouts_enabled()) return;
    timeboxed::set_deadline(phase_ == running
                            ? crpcut_calc_deadline(ts_us)
                            : ts_us);
    runner_->set_deadline(this);
  }

  unsigned long
  crpcut_test_case_registrator
  ::duration_us() const
  {
    return clocks::monotonic::timestamp_absolute() - real_time_at_start_;
  }

  bool
  crpcut_test_case_registrator
  ::is_naughty_child() const
  {
    return pid_ && pid_ != wrapped::getpid();
  }

  void
  crpcut_test_case_registrator
  ::freeze() const
  {
    for (;;)
      {
        wrapped::select(0,0,0,0,0);
      }
  }

  crpcut_test_case_registrator
  ::crpcut_test_case_registrator(const char *name, namespace_info *ns)
    : name_(name),
      location_(),
      ns_info_(ns),
      suite_list_(0),
      active_readers_(0),
      killed_(false),
      death_note_(false),
      pid_(0),
      real_time_at_start_(),
      cpu_time_at_start_(),
      dirnum_(~0U),
      phase_(creating),
      cputime_limit_us_(0),
      runner_(0),
      env_(0),
      reporter_(0),
      process_(0),
      filesystem_(0)
  {
  }

  crpcut_test_case_registrator
  ::crpcut_test_case_registrator(const char             *name,
                                 datatypes::fixed_string location,
                                 const namespace_info   &ns,
                                 unsigned long           cputime_timeout_us,
                                 comm::reporter         *reporter,
                                 process_control        *process,
                                 filesystem_operations  *filesystem,
                                 test_runner            *runner)
    : name_(name),
      location_(location),
      ns_info_(&ns),
      suite_list_(0),
      active_readers_(0),
      killed_(false),
      death_note_(false),
      pid_(0),
      real_time_at_start_(),
      cpu_time_at_start_(),
      dirnum_(~0U),
      phase_(creating),
      cputime_limit_us_(cputime_timeout_us),
      runner_(runner),
      env_(0),
      reporter_(reporter),
      process_(process),
      filesystem_(filesystem)
  {
    link_before(runner_->reg_);
  }


  void
  crpcut_test_case_registrator
  ::set_pid(pid_t pid)
  {
    assert(pid_ == 0);
    pid_ = pid;
  }
  void
  crpcut_test_case_registrator
  ::prepare_construction(unsigned long deadline_us)
  {
    if (env_->tests_as_child_procs())
      {
        unsigned long req = deadline_us*env_->timeout_multiplier();
        (*reporter_)(comm::set_timeout, req);
      }
  }

  void
  crpcut_test_case_registrator
  ::set_cputime_at_start(const struct timeval &t)
  {
    cpu_time_at_start_ = t;
  }

  void
  crpcut_test_case_registrator
  ::prepare_destruction(unsigned long deadline_us)
  {
    if (env_->timeouts_enabled())
      {
        unsigned long req = deadline_us*env_->timeout_multiplier();
        (*reporter_)(comm::set_timeout, req);
      }
  }

  bool
  crpcut_test_case_registrator
  ::match_name(const char *name_param) const
  {
    const char *p = ns_info_->match_name(name_param);
    if (p)
      {
        if (p != name_param || *p == ':')
          {
            if (!*p) return true; // match for whole suites
            if (*p++ != ':') return false;
            if (*p++ != ':') return false;
          }
      }
    else
      {
        p = name_param;
      }
    return crpcut::wrapped::strcmp(p, name_) == 0;
  }

  std::size_t
  crpcut_test_case_registrator
  ::full_name_len() const
  {
    return ns_info_->full_name_len()  + wrapped::strlen(name_);
  }

  std::ostream &
  crpcut_test_case_registrator
  ::print_name(std::ostream &os) const
  {
    os << *ns_info_;
    return os << name_;
  }

  void
  crpcut_test_case_registrator
  ::manage_test_case_execution(crpcut_test_case_base* p)
  {
    if (env_->tests_as_child_procs())
      {
        struct rusage usage;
        int rv = process_->getrusage(RUSAGE_SELF, &usage);
        assert(rv == 0);
        struct timeval cputime;
        timeradd(&usage.ru_utime, &usage.ru_stime, &cputime);
        (*reporter_)(comm::begin_test, cputime);
      }
    try {
      p->crpcut_run();
    }
    catch (...)
      {
        heap::set_limit(heap::system);
        std::ostringstream out;
        out << "Unexpectedly caught "
            << policies::crpcut_exception_translator::try_all();
        (*reporter_)(comm::exit_fail, out, get_location());
      }
    if (!env_->tests_as_child_procs())
      {
        crpcut_register_success(true);
      }
  }

  void
  crpcut_test_case_registrator
  ::kill()
  {
    assert(pid_);
    process_->killpg(pid_, SIGKILL);
    killed_ = true;
  }

  void
  crpcut_test_case_registrator
  ::clear_deadline()
  {
    if (deadline_is_set())
      {
        runner_->clear_deadline(this);
        timeboxed::clear_deadline();
      }
  }

  void
  crpcut_test_case_registrator::set_wd(unsigned n)
  {
    dirnum_ = n;
    stream::toastream<std::numeric_limits<int>::digits/3+1> name;
    name << n << '\0';
    if (filesystem_->mkdir(name.begin(), 0700) != 0)
      {
        assert(errno == EEXIST);
      }
  }

  void
  crpcut_test_case_registrator
  ::goto_wd() const
  {
    stream::toastream<std::numeric_limits<int>::digits/3+1> name;
    name << dirnum_ << '\0';
    if (filesystem_->chdir(name.begin()) != 0)
      {
        (*reporter_)(comm::exit_fail, "Couldn't chdir working dir", get_location());
        assert("unreachable code reached" == 0);
      }
  }

  bool
  crpcut_test_case_registrator
  ::cputime_timeout(unsigned long us) const
  {
    return env_->timeouts_enabled()
      && cputime_limit_us_
      && us > cputime_limit_us_;
  }


  bool
  crpcut_test_case_registrator
  ::check_exit_status(int status, std::ostream &out)
  {
    if (crpcut_failed() || crpcut_is_expected_exit(status)) return false;

    crpcut_register_success(false);
    set_location(out, get_location());
    out << "Exited with code " << status << "\nExpected ";
    crpcut_expected_death(out);
    return true;
   }

  bool
  crpcut_test_case_registrator
  ::check_signal_status(int signo, unsigned long cputime_us, std::ostream &out)
  {
    if (crpcut_failed()) return false;

    if (!crpcut_is_expected_signal(signo))
      {
        set_location(out, get_location());
        crpcut_register_success(false);
        if (killed_)
          {
            out << "Timed out - killed";
          }
        else
          {
            out << "Died on signal " << signo;
          }
        out << "\nExpected ";
        crpcut_expected_death(out);
        return true;
      }

    if (cputime_timeout(cputime_us))
      {
        crpcut_register_success(false);
        set_location(out, get_location());
        out << "Test consumed "
            << cputime_us / 1000 << "ms CPU-time\nLimit was "
            << cputime_limit_us_ / 1000 << "ms";
        return true;
      }
    return false;
  }

  void
  crpcut_test_case_registrator
  ::set_test_environment(test_environment *env)
  {
    assert(env_ == 0);
    env_ = env;
  }

  bool
  crpcut_test_case_registrator
  ::report_nonempty_working_dir(const char *dirname)
  {
    if (is_dir_empty(dirname)) return false;

    if (!crpcut_failed()) phase_ = post_mortem;
    stream::toastream<1024> tcname;
    tcname << *this << '\0';
    send_to_presentation(comm::dir, 0, 0);
    filesystem_->rename(dirname, tcname.begin());
    crpcut_register_success(false);
    return true;
  }

  void
  crpcut_test_case_registrator
  ::manage_death()
  {
    ::siginfo_t info = get_siginfo(pid_, process_);

    unsigned long cputime_us = runner_->calc_cputime(cpu_time_at_start_);
    if (!killed_ && deadline_is_set())
      {
        clear_deadline();
      }
    comm::type t = comm::exit_ok;

    std::ostringstream out;
    if (!death_note_)
      {
        switch (info.si_code)
        {
          case CLD_EXITED:
            if (check_exit_status(info.si_status, out))
              {
                t = comm::exit_fail;
              }
            break;
          case CLD_DUMPED:
            if (crpcut_core_dumps_allowed())
              {
                set_location(out, get_location());
                out << "Died with core dump";
                t = comm::exit_fail;
                break;
              }
            // no break - fall through
          case CLD_KILLED:
            if (check_signal_status(info.si_status, cputime_us, out))
              {
                t = comm::exit_fail;
              }
            break;
          default:
            set_location(out, get_location());
            out << "Died for unknown reason, code=" << info.si_code;
            t = comm::exit_fail;
            break;
        }
        death_note_ = true;
      }
    if (t == comm::exit_fail)
      {
        crpcut_register_success(false);
      }
    stream::toastream<std::numeric_limits<int>::digits/3+1> dirname;
    dirname << dirnum_ << '\0';
    if (!crpcut_failed())
      {
        crpcut_on_ok_action(dirname.begin());
      }
    if (report_nonempty_working_dir(dirname.begin()))
      {
        if (out.str().empty())
          {
            set_location(out, get_location());
          }
        t = comm::exit_fail;
      }
    std::string s = out.str();
    send_to_presentation(t, s.length(), s.c_str());
    crpcut_register_success(t == comm::exit_ok);
    runner_->return_dir(dirnum_);
    struct {
      unsigned long critical;
      unsigned long duration_us;
    } end_msg;
    end_msg.critical = crpcut_tag().get_importance() == tag::critical;
    end_msg.duration_us = duration_us();
    send_to_presentation(comm::end_test,
                         sizeof(end_msg), (const char*)&end_msg);
    assert(crpcut_succeeded() || crpcut_failed());
  }

} // namespace crpcut
