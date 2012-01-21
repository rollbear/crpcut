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


#include "presentation_reader.hpp"
#include "printer.hpp"
#include "output/formatter.hpp"
#include "poll.hpp"
#include <crpcut.hpp>

namespace {
#define ESTR(s) { #s, sizeof(#s)-1 }
  static const crpcut::datatypes::fixed_string tag_info[]
  =  { CRPCUT_COMM_MSGS(ESTR) };
#undef ESTR
}

namespace crpcut {

  presentation_reader
  ::presentation_reader(poll<io>          &poller_,
                        int                fd_,
                        output::formatter &fmt_,
                        bool               verbose_)
    : poller(poller_),
      fd(fd_),
      fmt(fmt_),
      verbose(verbose_)
  {
    poller.add_fd(fd_, this);
  }

  presentation_reader
  ::~presentation_reader()
  {
    assert(fd >= 0);
  }

  void
  presentation_reader
  ::exception()
  {
    poller.del_fd(fd);
    fd = -1;
  }

  bool
  presentation_reader
  ::read()
  {
    assert(fd >= 0);
    pid_t test_case_id;

    ssize_t rv = wrapped::read(fd, &test_case_id, sizeof(test_case_id));
    if (rv == 0) return true;
    assert(rv == sizeof(test_case_id));
    test_case_result *s = 0;

    // a linear search isn't that great, but the
    // amount of data is small.
    for (test_case_result *i = messages.next();
         i != messages.next()->prev();
         i = i->next())
      {
        if (i->id == test_case_id)
          {
            s = i;
            break;
          }
      }
    if (!s)
      {
        s = new test_case_result(test_case_id);
        s->link_after(messages);
      }
    comm::type t;
    rv = wrapped::read(fd, &t, sizeof(t));
    assert(rv == sizeof(t));

    test_phase phase;
    rv = wrapped::read(fd, &phase, sizeof(phase));
    assert(rv == sizeof(phase));
    int mask = t & comm::kill_me;
    t = static_cast<comm::type>(t & ~mask);
    switch (t)
      {
      case comm::begin_test:
        {
          assert(!s->name);
          assert(s->history.is_empty());
          // introduction to test case, name follows

          size_t len = 0;
          char *ptr = static_cast<char*>(static_cast<void*>(&len));
          size_t bytes_read = 0;
          while (bytes_read < sizeof(len))
            {
              rv = wrapped::read(fd,
                                 ptr + bytes_read,
                                 sizeof(len) - bytes_read);
              assert(rv > 0);
              bytes_read += size_t(rv);
            }
          char *buff = static_cast<char *>(wrapped::malloc(len + 1));
          bytes_read = 0;
          while (bytes_read < len)
            {
              rv = wrapped::read(fd,
                                 buff + bytes_read,
                                 len - bytes_read);
              assert(rv >= 0);
              bytes_read += size_t(rv);
            }
          buff[len] = 0;
          s->name.str = buff;
          s->name.len = len;
          s->success = true;
          s->nonempty_dir = false;
        }
        break;
      case comm::end_test:
        {
          size_t len;
          rv = wrapped::read(fd, &len, sizeof(len));
          assert(rv == sizeof(len));
          bool critical;
          assert(len == sizeof(critical));
          rv = wrapped::read(fd, &critical, len);
          assert(rv == sizeof(critical));

          if (s->explicit_fail || !s->success || verbose)
            {
              printer print(fmt,
                            s->name,
                            s->success && !s->explicit_fail,
                            critical);

              for (event *i = s->history.next();
                   i != static_cast<event*>(&s->history);
                   i = i->next())
                {

                  fmt.print(tag_info[i->tag], i->body);
                }
              if (s->termination || s->nonempty_dir || s->explicit_fail)
                {
                  if (s->nonempty_dir)
                    {
                      const char *wd  = test_case_factory::get_working_dir();
                      const size_t dlen = wrapped::strlen(wd);
                      len = dlen;
                      len+= 1;
                      len+= s->name.len;
                      char *dn = static_cast<char*>(alloca(len + 1));
                      lib::strcpy(lib::strcpy(lib::strcpy(dn,  wd),
                                              "/"),
                                  s->name.str);
                      fmt.terminate(phase, s->termination,
                                    datatypes::fixed_string::make(dn, len));
                    }
                  else
                    {
                      fmt.terminate(phase, s->termination);
                    }
                }
            }
        }
        delete s;
        break;
      case comm::dir:
        {
          size_t len;
          rv = wrapped::read(fd, &len, sizeof(len));
          assert(rv == sizeof(len));
          assert(len == 0);
          (void)len; // silense warning
          s->success = false;
          s->nonempty_dir = true;
        }
        break;
      case comm::fail:
      case comm::exit_fail:
        s->explicit_fail = true;
                                           /* no break */
      case comm::exit_ok:
        s->success &= t == comm::exit_ok;
                                           /* no break */
      case comm::stdout:
      case comm::stderr:
      case comm::info:
        {
          size_t len;
          rv = wrapped::read(fd, &len, sizeof(len));
          assert(rv == sizeof(len));
          if (len)
            {
              char *buff = static_cast<char *>(wrapped::malloc(len));
              rv = wrapped::read(fd, buff, len);
              assert(size_t(rv) == len);

              if (t == comm::exit_ok || t == comm::exit_fail)
                {
                  s->termination = datatypes::fixed_string::make(buff, len);
                }
              else
                {
                  event *e = new event(t, buff, len);
                  e->link_before(s->history);
                }
            }
        }
        break;
      default:
        assert("unreachable code reached" == 0);
        /* no break */
      }
    return false;
  }

  bool
  presentation_reader
  ::write()
  {
    assert("Shan't ever write back" == 0);
    return true;
  }

}
