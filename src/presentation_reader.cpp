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


#include "presentation_reader.hpp"
#include "printer.hpp"
#include "output/formatter.hpp"
#include "poll.hpp"
#include "posix_error.hpp"
#include "registrator_list.hpp"
#include "wrapped/posix_encapsulation.hpp"
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
                        output::formatter      &summary_fmt,
                        bool                    verbose,
                        const char             *working_dir,
                        registrator_list       &reg)
    : poller_(poller),
      fd_(fd),
      fmt_(fmt),
      summary_fmt_(summary_fmt),
      working_dir_(working_dir),
      verbose_(verbose),
      num_run_(0),
      num_failed_(0),
      reg_(reg)
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
    for (crpcut_test_case_registrator *i = reg_.next();
         !reg_.is_this(i);
         i = i->next())
      {
        std::ostringstream os;
        os << *i;
        std::string name(os.str());
        tag::importance importance = i->get_importance();
        fmt_.blocked_test(importance, name);
        summary_fmt_.blocked_test(importance, name);
      }
    fmt_.statistics(num_run_, num_failed_);
    summary_fmt_.statistics(num_run_, num_failed_);
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
    assert(!s->test);
    assert(s->history.is_empty());
    // introduction to test case follows

    size_t len = 0;
    fd_.read_loop(&len, sizeof(len));
    assert(len == sizeof(s->test));
    fd_.read_loop(&s->test, len);
    s->test->unlink();
    s->success = true;
    s->nonempty_dir = false;
  }

  void
  presentation_reader
  ::end_test(test_phase phase, test_case_result* s)
  {
    assert(s->test);

    size_t len;
    fd_.read_loop(&len, sizeof(len));
    struct
    {
      unsigned long critical;
      unsigned long duration_us;
    } info;
    assert(len == sizeof(info));
    fd_.read_loop(&info, len);

    const bool pass = s->success && !s->explicit_fail;
    tag &t = s->test->crpcut_tag();
    if (pass) t.pass(); else t.fail();
    ++num_run_;
    if (!pass || verbose_)
      {
        num_failed_ += !pass;

        std::ostringstream name;
        name << *s->test;
        printer print(fmt_, name.str(), pass, info.critical, info.duration_us);

        for (event *i = s->history.next();
            i != static_cast<event*>(&s->history);
            i = i->next())
          {
            fmt_.print(tag_info[i->tag_], i->msg_, i->location_);
          }
        if (s->termination || s->nonempty_dir || s->explicit_fail)
          {
            if (s->nonempty_dir)
              {
                std::ostringstream dirname;
                dirname << working_dir_ << "/" << *s->test;
                fmt_.terminate(phase, s->termination, s->location, dirname.str());
              }
            else
              {
                fmt_.terminate(phase, s->termination, s->location);
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
        summary_fmt_.nonempty_dir(name);
        return;
      }
    s->success = false;
    s->nonempty_dir = true;
  }

  void
  presentation_reader
  ::output_data(comm::type t, test_case_result *s, datatypes::fixed_string msg, datatypes::fixed_string location)
  {
    if (s->termination) return;
    if (t == comm::exit_ok || t == comm::exit_fail)
      {
        s->termination = msg;
        s->location    = location;
        return;
      }
    event *e = new event(t, msg, location);
    e->link_before(s->history);
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

      test_case_result *s = find_result_for(test_case_id);
      if (!s && test_case_id)
        {
          s = new test_case_result(test_case_id);
          s->link_after(messages_);
        }
      assert(test_case_id || t == comm::dir);
      int mask = t & comm::kill_me;
      t = static_cast<comm::type>(t & ~mask);
      datatypes::fixed_string location = { 0, 0 };
      bool with_location = false;
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
        case comm::info:
          with_location = t != comm::exit_ok;
                                           /* no break */
        case comm::stdout:
        case comm::stderr:
          {
            datatypes::fixed_string msg = { 0, 0 };
            fd_.read_loop(&msg.len, sizeof(msg.len));
            assert(!with_location || msg.len);
            if (with_location)
              {
                fd_.read_loop(&location.len, sizeof(location.len));
                assert(location.len);
                char *buff = static_cast<char*>(wrapped::malloc(location.len));
                assert(buff);
                fd_.read_loop(buff, location.len);
                location.str = buff;
                msg.len -= location.len + sizeof(location.len);
              }
            char *buff = static_cast<char*>(wrapped::malloc(msg.len));
            fd_.read_loop(buff, msg.len);
            msg.str = buff;
            output_data(t, s, msg, location);
          }
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

  unsigned
  presentation_reader
  ::num_failed() const
  {
    return num_failed_;
  }

}
