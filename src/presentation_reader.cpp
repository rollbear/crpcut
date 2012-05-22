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
#include "posix_error.hpp"
#include <crpcut.hpp>

namespace {
#define ESTR(s) { #s, sizeof(#s)-1 }
  static const crpcut::datatypes::fixed_string tag_info[]
  =  { CRPCUT_COMM_MSGS(ESTR) };
#undef ESTR
}

namespace crpcut {

  presentation_reader
  ::presentation_reader(poll<io>               &poller,
                        comm::rfile_descriptor &fd,
                        output::formatter      &fmt,
                        bool                    verbose,
                        const char             *working_dir)
    : poller_(poller),
      fd_(fd),
      fmt_(fmt),
      working_dir_(working_dir),
      verbose_(verbose),
      num_run_(0),
      num_failed_(0)
  {
    poller_.add_fd(fd_, this);
  }

  presentation_reader
  ::~presentation_reader()
  {
  }

  void
  presentation_reader
  ::exception()
  {
    poller_.del_fd(&fd_);
    comm::rfile_descriptor().swap(fd_);
    fmt_.statistics(num_run_, num_failed_);
  }

  test_case_result *
  presentation_reader
  ::find_result_for(pid_t id)
  {
    // a linear search isn't that great, but the
    // amount of data is small.
    for (test_case_result *i = messages_.next();
         i != messages_.next()->prev();
         i = i->next())
      {
        if (i->id == id)
          {
            return i;
          }
      }
    return 0;
  }

  void
  presentation_reader
  ::begin_test(test_case_result *s)
  {
    assert(!s->name);
    assert(s->history.is_empty());
    // introduction to test case, name follows

    size_t len = 0;
    fd_.read_loop(&len, sizeof(len));
    char *buff = static_cast<char *>(wrapped::malloc(len + 1));
    assert (buff);
    fd_.read_loop(buff, len);
    buff[len] = 0;
    s->name.str = buff;
    s->name.len = len;
    s->success = true;
    s->nonempty_dir = false;
  }

  void
  presentation_reader
  ::end_test(test_phase phase, test_case_result* s)
  {
    size_t len;
    fd_.read_loop(&len, sizeof(len));
    bool critical;
    assert(len == sizeof(critical));
    fd_.read_loop(&critical, len);

    ++num_run_;
    if (s->explicit_fail || !s->success || verbose_)
      {
        num_failed_ += s->explicit_fail || !s->success;

        printer print(fmt_,
                      s->name,
                      s->success && !s->explicit_fail,
                      critical);

        for (event *i = s->history.next();
            i != static_cast<event*>(&s->history);
            i = i->next())
          {

            fmt_.print(tag_info[i->tag], i->body);
          }
        if (s->termination || s->nonempty_dir || s->explicit_fail)
          {
            if (s->nonempty_dir)
              {
                const size_t dlen = wrapped::strlen(working_dir_);
                len = dlen;
                len+= 1;
                len+= s->name.len;
                char *dn = static_cast<char*>(alloca(len + 1));
                lib::strcpy(lib::strcpy(lib::strcpy(dn,  working_dir_),
                                        "/"),
                            s->name.str);
                fmt_.terminate(phase, s->termination,
                              datatypes::fixed_string::make(dn, len));
              }
            else
              {
                fmt_.terminate(phase, s->termination);
              }
          }
      }
    delete s;
  }

  void
  presentation_reader
  ::nonempty_dir(test_case_result *s)
  {
    size_t len;
    fd_.read_loop(&len, sizeof(len));
    assert(s || len > 0U);
    if (!s)
      {
        char *name = static_cast<char*>(alloca(len));
        fd_.read_loop(name, len);
        fmt_.nonempty_dir(name);
        return;
      }
    s->success = false;
    s->nonempty_dir = true;
  }

  void
  presentation_reader
  ::output_data(comm::type t, test_case_result *s)
  {
    size_t len;
    fd_.read_loop(&len, sizeof(len));
    if (len == 0) return;

    char *buff = static_cast<char *>(wrapped::malloc(len));
    fd_.read_loop(buff, len);

    if (t == comm::exit_ok || t == comm::exit_fail)
      {
        s->termination = datatypes::fixed_string::make(buff, len);
        return;
      }
    event *e = new event(t, buff, len);
    e->link_before(s->history);
  }

  void
  presentation_reader
  ::blocked_test()
  {
    size_t len;
    fd_.read_loop(&len, sizeof(len));
    assert(len);
    void *addr = alloca(len + 1);
    char *name = static_cast<char*>(addr);
    fd_.read_loop(name, len);
    tag::importance importance;
    fd_.read_loop(&importance, sizeof(importance));
    fmt_.blocked_test(importance,
                      datatypes::fixed_string::make(name, len));
  }

  bool
  presentation_reader
  ::read()
  {
    try {
      pid_t test_case_id;
      fd_.read_loop(&test_case_id, sizeof(test_case_id));

      comm::type t;
      fd_.read_loop(&t, sizeof(t));

      test_phase phase;
      fd_.read_loop(&phase, sizeof(phase));

      if (phase == never_run)
        {
          blocked_test();
          return false;
        }
      test_case_result *s = find_result_for(test_case_id);
      if (!s && test_case_id)
        {
          s = new test_case_result(test_case_id);
          s->link_after(messages_);
        }

      int mask = t & comm::kill_me;
      t = static_cast<comm::type>(t & ~mask);
      switch (t)
        {
        case comm::begin_test:
          begin_test(s);
          break;
        case comm::end_test:
          end_test(phase, s);
          break;
        case comm::dir:
          nonempty_dir(s);
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
          output_data(t, s);
          break;
        default:
          assert("unreachable code reached" == 0);
          /* no break */
        }
    }
    catch (posix_error &)
    {
        return true;
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
