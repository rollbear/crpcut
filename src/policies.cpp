/*
 * Copyright 2009-2011 Bjorn Fahller <bjorn@fahller.se>
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
#include "clocks.hpp"
extern "C" {
#include <sys/resource.h>
#include <sys/stat.h>
}
#include "posix_encapsulation.hpp"

namespace {
  void wipe_dir(char *begin, char *end)
  {
    using namespace crpcut;
    assert(*end == 0);
    DIR *d = wrapped::opendir(begin);
    if (!d) return;
    char buff[sizeof(dirent) + PATH_MAX];
    dirent *ent = reinterpret_cast<dirent*>(buff);
    dirent *result = ent;
    *end = '/';
    while (result && (wrapped::readdir_r(d, ent, &result) == 0))
      {
        if (!result) break;
        if (wrapped::strcmp(ent->d_name, ".") == 0 ||
            wrapped::strcmp(ent->d_name, "..") == 0)
          {
            continue;
          }
        char *name_end = lib::strcpy(end + 1, ent->d_name);
        assert(*name_end == 0);
        wipe_dir(begin, name_end); // no-op if name is not a dir
        (void)wrapped::remove(begin);
      }
    *end = 0;
    (void)wrapped::closedir(d);
  }
}


namespace crpcut {

  namespace policies {

    crpcut_exception_translator::crpcut_exception_translator(int)
      : next(this),
        prev(this)
    {
    }

    crpcut_exception_translator::crpcut_exception_translator()
      : next(&root_object()),
        prev(root_object().prev)
    {
      root_object().prev = this;
      prev->next = this;
    }

    crpcut_exception_translator::~crpcut_exception_translator()
    {
      crpcut_exception_translator *p = prev;
      prev = next->prev;
      next->prev = p;
    }

    std::string crpcut_exception_translator::try_all()
    {
      for (crpcut_exception_translator *p = root_object().next;
           p != &root_object();
           p = p->next)
        {
          try {
            return p->crpcut_translate();
          }
          catch (...)
            {
            }
        }
      return "...";
    }

    crpcut_exception_translator& crpcut_exception_translator::root_object()
    {
      static crpcut_exception_translator obj(0);
      return obj;
    }

    std::string crpcut_exception_translator::crpcut_translate() const
    {
      throw;
    }

    namespace deaths {

      unsigned long
      crpcut_none
      ::crpcut_calc_deadline(unsigned long ts) const
      {
        return ts + 1000;
      }

      void no_action::crpcut_on_ok_action(const char *) const
      {
      }

      void wipe_working_dir::crpcut_on_ok_action(const char *wd_name) const
      {
        if (test_case_factory::tests_as_child_procs())
          {
            static char namebuff[PATH_MAX] = "";
            char *end = lib::strcpy(namebuff, wd_name);
            wipe_dir(namebuff, end);
          }
      }

      void
      crpcut_none::crpcut_on_ok_action(const char *) const
      {
      }

      bool
      crpcut_none::crpcut_is_expected_exit(int) const
      {
        return false;
      }

      bool
      crpcut_none::crpcut_is_expected_signal(int) const
      {
        return false;
      }

    }

    no_core_file::no_core_file()
    {
      rlimit r = { 0, 0};
      wrapped::setrlimit(RLIMIT_CORE, &r);
    }

    namespace deaths {
      void crpcut_none::crpcut_expected_death(std::ostream &os) const
      {
        os << "normal exit";
      }
    }

    namespace dependencies {
      crpcut_base::~crpcut_base()
      {
      }

      void crpcut_base::crpcut_uninhibit_dependants()
      {
        for (basic_enforcer *p = crpcut_dependants; p; p = p->next)
          {
            if (--p->crpcut_num == 0)
              {
                p->crpcut_dec_action();
              };
          }
      }

      void crpcut_base::crpcut_register_success(bool value)
      {
        if (value)
          {
            if (crpcut_state != crpcut_not_run) return;
            crpcut_state = crpcut_success;
            crpcut_uninhibit_dependants();
          }
        else
          {
            crpcut_state = crpcut_fail;
          }
      }

      void crpcut_base::crpcut_add_action(dependencies::basic_enforcer *other)
      {
        other->crpcut_inc();
      }

      crpcut_base::crpcut_base()
        : crpcut_state(crpcut_not_run),
          crpcut_num(0),
          crpcut_dependants(0)
      {
      }

      void
      crpcut_base::crpcut_add(basic_enforcer *other)
      {
        other->next = crpcut_dependants;
        crpcut_dependants = other;
        crpcut_add_action(other);
      }

      void
      crpcut_base::crpcut_inc()
      {
        ++crpcut_num;
      }

      bool
      crpcut_base::crpcut_can_run() const
      {
        return crpcut_num == 0;
      }

      bool
      crpcut_base::crpcut_failed() const
      {
        return crpcut_state == crpcut_fail;
      }

      bool
      crpcut_base::crpcut_succeeded() const
      {
        return crpcut_state == crpcut_success;
      }

      basic_enforcer::basic_enforcer()
        : next(0)
      {
      }

    } // namespace dependencies

    namespace timeout {

      cputime_enforcer::cputime_enforcer(unsigned long timeout_ms)
        : duration_ms(timeout_ms),
          start_timestamp_ms(clocks::cputime::timestamp_ms_absolute())
      {
        if (timeout_ms && test_case_factory::timeouts_enabled())
          {
            rlimit r = { (duration_ms + 1500) / 1000,
                         (duration_ms + 2500) / 1000 };
            wrapped::setrlimit(RLIMIT_CPU, &r);
          }
      }

      cputime_enforcer::~cputime_enforcer()
      {
        if (!test_case_factory::timeouts_enabled() || duration_ms == 0) return;

        clocks::cputime::timestamp now
          = clocks::cputime::timestamp_ms_absolute();
        unsigned long diff = now - start_timestamp_ms;
        if  (diff > duration_ms)
          {
            stream::toastream<128> os;
            os << "CPU-time timeout " << duration_ms
               << "ms exceeded.\n  Actual time to completion was " << diff
               << "ms";
            comm::report(comm::exit_fail, os.begin(), os.size());
          }
      }

      monotonic_enforcer::monotonic_enforcer(unsigned long timeout_ms)
        : duration_ms(timeout_ms),
          start_timestamp_ms(clocks::monotonic::timestamp_ms_absolute())
      {

        if (test_case_factory::tests_as_child_procs())
          {
            clocks::monotonic::timestamp deadline = duration_ms;
            comm::report(comm::set_timeout, deadline);
          }
      }

      monotonic_enforcer::~monotonic_enforcer()
      {
        if (!test_case_factory::timeouts_enabled()) return;
        typedef clocks::monotonic mono;
        mono::timestamp now  = mono::timestamp_ms_absolute();
        comm::report(comm::cancel_timeout, 0, 0);
        unsigned long diff = now - start_timestamp_ms;
        if (diff > duration_ms)
          {
            stream::toastream<128> os;
            os << "Realtime timeout " << duration_ms
               << "ms exceeded.\n  Actual time to completion was " << diff
               << "ms";
            if (test_case_factory::tests_as_child_procs())
              {
                comm::report(comm::exit_fail, os.begin(), os.size());
              }
            else
              {
                wrapped::write(1, os.begin(), os.size());
                wrapped::write(1, "\n", 1);
              }
          }
      }

    } // namespace timeout

  } // namespace policies
}
