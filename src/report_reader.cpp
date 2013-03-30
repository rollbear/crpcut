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

#include <crpcut.hpp>
#include "test_runner.hpp"
#include "clocks/clocks.hpp"
#include "wrapped/posix_encapsulation.hpp"
#include "posix_error.hpp"
#include "sigignore.hpp"

namespace {
void make_message(crpcut::datatypes::fixed_string location,
                  const char *msg,
                  std::string &result)
{
  std::ostringstream os;
  void *len_addr = &location.len;
  os.write(static_cast<const char*>(len_addr), sizeof(location.len));
  os.write(location.str, std::streamsize(location.len));
  os << msg;
  os.str().swap(result);
}
}
namespace crpcut {

  report_reader
  ::report_reader(crpcut_test_monitor *r)
    : fdreader(r)
  {
  }

  void
  report_reader
  ::set_timeout(void *buff, size_t len)
  {
    typedef clocks::monotonic::timestamp timestamp;
    assert(len == sizeof(timestamp));
    timestamp *ts_us = static_cast<timestamp*>(buff);

    cancel_timeout();

    mon_->set_timeout(*ts_us);
  }

  void
  report_reader
  ::cancel_timeout()
  {
    mon_->clear_deadline();
  }

  bool
  report_reader
  ::do_read_data()
  {
    comm::type t;
    int kill_mask = 0;
    sigignore ignore(SIGPIPE);
    try {
      std::string msg;
      read_loop(&t, sizeof(t));
      kill_mask = t & comm::kill_me;
      t = static_cast < comm::type >(t & ~kill_mask);
      if (t == comm::exit_fail || t == comm::fail || kill_mask)
        {
          mon_->crpcut_register_success(false);
        }
      size_t len = 0;
      read_loop(&len, sizeof(len));
      char *buff = static_cast<char *>(::alloca(len));
      read_loop(buff, len);

      if (kill_mask)
        {
          if (len == 0 || t == comm::set_timeout || t == comm::begin_test)
            {
              make_message(mon_->get_location(),
                           "A child process spawned from the test has misbehaved. Process group killed",
                           msg);
              buff = const_cast<char*>(msg.c_str());
              len = msg.length();
            }
          t = comm::exit_fail;
          mon_->set_phase(child);
          mon_->kill();
        }
      switch (t)
        {
        case comm::set_timeout:
          set_timeout(buff, len);
          return true;
        case comm::cancel_timeout:
          assert(len == 0);
          cancel_timeout();
          return true;
        case comm::begin_test:
          mon_->set_phase(running);
          {
            void *addr = buff;
            assert(len == sizeof(struct timeval));
            struct timeval *start_time = static_cast<struct timeval*>(addr);
            mon_->set_cputime_at_start(*start_time);
          }
          return true;
        case comm::end_test:
          if (!mon_->crpcut_failed())
            {
              mon_->set_phase(destroying);
              return true;
            }
          {
            make_message(mon_->get_location(), "Earlier VERIFY failed", msg);
            buff = const_cast<char*>(msg.c_str());
            len = msg.length();
            t = comm::exit_fail;
            break;
          }
        default:
          break; // silence warning
        }
      mon_->send_to_presentation(t, len, buff);
      if (t == comm::exit_ok || t == comm::exit_fail)
        {
          mon_->set_death_note();
        }
    }
    catch (posix_error &e)
    {
      if (!e.what() || e.get_errno() == EPIPE)
        {
          return false;
        }
      throw;
    }
    return !kill_mask;
  }
}
