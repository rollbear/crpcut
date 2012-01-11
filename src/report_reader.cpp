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
#include "clocks/clocks.hpp"
#include "wrapped/posix_encapsulation.hpp"

namespace crpcut {

  report_reader
  ::report_reader(crpcut_test_case_registrator *r)
    : fdreader(r)
  {
  }

  void
  report_reader
  ::set_fds(int in_fd, int out_fd, poll<fdreader>* read_poller)
  {
    fdreader::set_fd(in_fd, read_poller);
    response_fd = out_fd;
  }

  void
  report_reader
  ::close()
  {
    wrapped::close(response_fd);
    response_fd = -1;
    fdreader::close();
  }

  bool
  report_reader
  ::do_read(int fd, bool do_reply)
  {
    comm::type t;
    ssize_t rv;
    do {
      rv = wrapped::read(fd, &t, sizeof(t));
    } while (rv == -1 && errno == EINTR);
    if (rv == 0) return false;
    int kill_mask = t & comm::kill_me;
    if (kill_mask) do_reply = false; // unconditionally
    t = static_cast < comm::type >(t & ~kill_mask);
    if (rv == 0) return false;
    assert(rv == sizeof(t));
    if (t == comm::exit_fail || t == comm::fail || kill_mask)
      {
        reg_->crpcut_register_success(false);
      }
    size_t len = 0;

    do {
      rv = wrapped::read(fd, &len, sizeof(len));
    } while (rv == -1 && errno == EINTR);
    if (rv == 0)
      {
        return false;
      }
    assert(rv == sizeof(len));

    size_t bytes_read = 0;
    if (t == comm::set_timeout && !kill_mask)
      {
        assert(len == sizeof(reg_->crpcut_absolute_deadline_ms));
        clocks::monotonic::timestamp ts;
        char *p = static_cast<char*>(static_cast<void*>(&ts));
        while (bytes_read < len)
          {
            rv = wrapped::read(fd, p + bytes_read, len - bytes_read);
            if (rv == -1 && errno == EINTR) continue;
            assert(rv > 0);
            bytes_read += size_t(rv);
          }
        if (reg_->crpcut_deadline_is_set())
          {
            reg_->crpcut_clear_deadline();
          }
        ts+= clocks::monotonic::timestamp_ms_absolute();
        reg_->crpcut_set_timeout(ts);
        if (do_reply)
          {
            do {
              rv = wrapped::write(response_fd, &len, sizeof(len));
              if (rv == 0 || (rv == -1 && errno == EPIPE))
                {
                  return false;
                }
            } while (rv == -1 && errno == EINTR);
            if (rv == 0)
              {
                return false; // eof
              }
          }
        assert(reg_->crpcut_deadline_is_set());
        test_case_factory::set_deadline(reg_);
        return true;
      }
    if (t == comm::cancel_timeout && !kill_mask)
      {
        assert(len == 0);
        if (!reg_->crpcut_death_note)
          {
            reg_->crpcut_clear_deadline();
            if (do_reply)
              {
                do {
                  rv = wrapped::write(response_fd, &len, sizeof(len));
                  if (rv == 0 || (rv == -1 && errno == EPIPE))
                    {
                      return false;
                    }
                } while (rv == -1 && errno == EINTR);
              }
          }
        return true;
      }

    char *buff = static_cast<char *>(::alloca(len));
    int err;
    errno = 0;
    while (bytes_read < len)
      {
        rv = wrapped::read(fd, buff + bytes_read, len - bytes_read);
        err=errno;
        if (rv == -1 && errno == EINTR) continue;
        assert(rv > 0 && err == 0);
        bytes_read += size_t(rv);
      }
    if (kill_mask)
      {
        if (len == 0)
          {
            static char msg[] = "A child process spawned from the test has misbehaved. Process group killed";
            buff = msg;
            len = sizeof(msg) - 1;
          }
        t = comm::exit_fail;
        reg_->crpcut_phase = child;
        wrapped::killpg(reg_->crpcut_get_pid(), SIGKILL); // now
      }
    if (do_reply)
      {
        do {
          rv = wrapped::write(response_fd, &len, sizeof(len));
          if (rv == 0 || (rv == -1 && errno == EPIPE))
            {
              return false;
            }
        } while (rv == -1 && errno == EINTR);
      }
    switch (t)
      {
      case comm::begin_test:
        reg_->crpcut_phase = running;
        {
          assert(len == sizeof(reg_->crpcut_cpu_time_at_start));

          while (bytes_read < len)
            {
              rv = wrapped::read(fd, buff + bytes_read, len - bytes_read);
              if (rv == -1 && errno == EINTR) continue;
              assert(rv > 0 && errno == 0);
              bytes_read += size_t(rv);
            }
          wrapped::memcpy(&reg_->crpcut_cpu_time_at_start, buff, len);
        }
        return true;
      case comm::end_test:
        if (!reg_->crpcut_failed())
          {
            reg_->crpcut_phase = destroying;
            return true;
          }
        {
          static char msg[] = "Earlier VERIFY failed";
          buff = msg;
          len = sizeof(msg) - 1;
          t = comm::exit_fail;
          wrapped::killpg(reg_->crpcut_get_pid(), SIGKILL); // now
          break;
        }
      default:
        ; // silence warning
      }

    test_case_factory::present(reg_->crpcut_get_pid(),
                               t,
                               reg_->crpcut_phase,
                               len, buff);
    if (t == comm::exit_ok || t == comm::exit_fail)
      {
        if (!reg_->crpcut_death_note && reg_->crpcut_deadline_is_set())
          {
            reg_->crpcut_clear_deadline();
          }
        reg_->crpcut_death_note = true;
      }
    return !kill_mask;
  }
}
