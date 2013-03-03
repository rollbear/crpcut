/*
 * Copyright 2012 Bjorn Fahller <bjorn@fahller.se>
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

#include <gmock/gmock.h>
#include <crpcut.hpp>
#include "../poll.hpp"
#include <deque>
#include <vector>
#include "../clocks/clocks.hpp"

using namespace testing;

namespace {
  class test_reader : public crpcut::report_reader
  {
  public:
    test_reader(crpcut::crpcut_test_monitor *mon)
    : crpcut::report_reader(mon)
    {
    }
    MOCK_METHOD0(close, void());
    MOCK_CONST_METHOD2(read, ssize_t(void *buff, size_t));
    void read_loop(void *addr, size_t bytes, const char*) const
    {
      if (bytes == 0U) return;

      ASSERT_TRUE(buffer.size() > 0U);
      data current = buffer.front();
      buffer.pop_front();
      ASSERT_TRUE(current.bytes.size() == bytes);
      memcpy(addr, &current.bytes[0], bytes);
    }

    struct data
    {
      data(const void *p, size_t l) : bytes(l) { memcpy(&bytes[0], p, l); }
      template <typename T>
      data(const T& t) : bytes(sizeof(t))
      {
    	memcpy(&bytes[0], &t, sizeof(t));
      }
      std::vector<char> bytes;
    };
    mutable std::deque<data> buffer;
  };

  class mock_monitor : public crpcut::crpcut_test_monitor
  {
  public:
    MOCK_METHOD1(set_timeout, void(unsigned long));
    MOCK_CONST_METHOD0(duration_us, unsigned long());
    MOCK_CONST_METHOD0(deadline_is_set, bool());
    MOCK_CONST_METHOD0(get_location, crpcut::datatypes::fixed_string());
    MOCK_METHOD0(clear_deadline, void());
    MOCK_METHOD1(crpcut_register_success, void(bool));
    MOCK_METHOD1(set_phase, void(crpcut::test_phase));
    MOCK_METHOD0(kill, void());
    MOCK_CONST_METHOD0(crpcut_failed, bool());
    MOCK_METHOD1(set_cputime_at_start, void(const struct timeval&));
    MOCK_CONST_METHOD3(send_to_presentation,
                       void(crpcut::comm::type, size_t, const char*));
    MOCK_METHOD0(set_death_note, void());
    MOCK_METHOD0(activate_reader, void());
    MOCK_METHOD0(deactivate_reader, void());
    MOCK_CONST_METHOD0(has_active_readers, bool());
    MOCK_METHOD0(manage_death, void());
    MOCK_CONST_METHOD0(is_naughty_child, bool());
    MOCK_CONST_METHOD0(freeze, void());
  };


  struct fix
  {
    typedef crpcut::clocks::monotonic::timestamp ts;
    fix() : loc(crpcut::datatypes::fixed_string::make("apa:3")),
    		reader(&monitor) {}
    crpcut::datatypes::fixed_string loc;
    StrictMock<mock_monitor>        monitor;
    StrictMock<test_reader>         reader;

    void verify_naughty_child()
    {
      EXPECT_CALL(monitor, crpcut_register_success(false));
      EXPECT_CALL(monitor, set_phase(crpcut::child));
      EXPECT_CALL(monitor, kill());
      EXPECT_CALL(monitor, set_death_note());
      ASSERT_FALSE(reader.read_data());
      ASSERT_TRUE(reader.buffer.size() == 0U);
    }

  };
}
TESTSUITE(report_reader)
{
  TEST(construction_and_immediate_destruction_does_nothing)
  {
     fix f;
  }



  TEST(begin_test_sets_phase_running_and_records_cputime,
       fix)
  {
    reader.buffer.push_back(crpcut::comm::begin_test);
    struct timeval tv = { 10, 1 };
    reader.buffer.push_back(sizeof(tv));
    reader.buffer.push_back(tv);
    EXPECT_CALL(monitor, set_phase(crpcut::running));
    EXPECT_CALL(monitor, set_cputime_at_start(_));
    ASSERT_TRUE(reader.read_data());
    ASSERT_TRUE(reader.buffer.size() == 0U);
  }

  TEST(set_timeout_forwards_timeout_to_monitor_without_any_other_effects,
       fix)
  {
    Sequence s1;
    reader.buffer.push_back(crpcut::comm::set_timeout);
    reader.buffer.push_back(sizeof(ts));
    reader.buffer.push_back(ts(100));
    EXPECT_CALL(monitor, deadline_is_set()).
        WillRepeatedly(Return(true));
    EXPECT_CALL(monitor, clear_deadline()).InSequence(s1);
    EXPECT_CALL(monitor, set_timeout(_)).InSequence(s1);
    ASSERT_TRUE(reader.read_data());
    ASSERT_TRUE(reader.buffer.size() == 0U);
  }

  TEST(cancel_timeout_forwards_clear_deadline_to_monitor_without_any_other_effects,
       fix)
  {
    reader.buffer.push_back(crpcut::comm::cancel_timeout);
    reader.buffer.push_back(size_t(0U));
    EXPECT_CALL(monitor, clear_deadline());
    ASSERT_TRUE(reader.read_data());
    ASSERT_TRUE(reader.buffer.size() == 0U);
  }

  TEST(exit_fail_marks_the_test_as_failed_notes_death_and_presents,
       fix)
  {
    static const char apa[] = { 'a', 'p', 'a' };
    reader.buffer.push_back(crpcut::comm::exit_fail);
    reader.buffer.push_back(sizeof(apa));
    reader.buffer.push_back(apa);
    EXPECT_CALL(monitor, set_death_note());
    EXPECT_CALL(monitor, crpcut_register_success(false));
    EXPECT_CALL(monitor, send_to_presentation(crpcut::comm::exit_fail, _, _)).
      With(Args<2,1>(ElementsAreArray(apa)));
    ASSERT_TRUE(reader.read_data());
    ASSERT_TRUE(reader.buffer.size() == 0U);
  }

  TEST(fail_marks_the_test_as_failed_and_presents,
       fix)
  {
    static const char apa[] = { 'a', 'p', 'a' };
    reader.buffer.push_back(crpcut::comm::fail);
    reader.buffer.push_back(sizeof(apa));
    reader.buffer.push_back(apa);
    EXPECT_CALL(monitor, crpcut_register_success(false));
    EXPECT_CALL(monitor, send_to_presentation(crpcut::comm::fail, _, _)).
      With(Args<2, 1>(ElementsAreArray(apa)));
    ASSERT_TRUE(reader.read_data());
    ASSERT_TRUE(reader.buffer.size() == 0U);
  }

  TEST(end_test_and_not_failed_sets_phase_destroying,
       fix)
  {
    Sequence s;
    reader.buffer.push_back(crpcut::comm::end_test);
    reader.buffer.push_back(size_t(0));
    EXPECT_CALL(monitor, crpcut_failed()).
      InSequence(s).
      WillOnce(Return(false));
    EXPECT_CALL(monitor, set_phase(crpcut::destroying)).
      InSequence(s);
    ASSERT_TRUE(reader.read_data());
    ASSERT_TRUE(reader.buffer.size() == 0U);
  }

  TEST(end_test_and_failed_presents_exit_fail_w_Earlier_VERIFY_msg,
       fix)
  {
    Sequence s;
    reader.buffer.push_back(crpcut::comm::end_test);
    reader.buffer.push_back(size_t(0));
    EXPECT_CALL(monitor, crpcut_failed()).
      InSequence(s).
      WillOnce(Return(true));
    EXPECT_CALL(monitor, get_location()).
      WillOnce(Return(crpcut::datatypes::fixed_string::make("apa:3")));
    EXPECT_CALL(monitor,
                send_to_presentation(crpcut::comm::exit_fail,
                                     _,
                                     StartsWith("apa:3\nEarlier VERIFY"))).
      InSequence(s);
    EXPECT_CALL(monitor, set_death_note());
    ASSERT_TRUE(reader.read_data());
    ASSERT_TRUE(reader.buffer.size() == 0U);
  }

  TEST(exit_ok_sends_to_presentation_and_sets_death_note,
       fix)
  {
    reader.buffer.push_back(crpcut::comm::exit_ok);
    reader.buffer.push_back(size_t(0));
    EXPECT_CALL(monitor, send_to_presentation(crpcut::comm::exit_ok, 0U, _));
    EXPECT_CALL(monitor, set_death_note());
    ASSERT_TRUE(reader.read_data());
    ASSERT_TRUE(reader.buffer.size() == 0U);
  }

  TESTSUITE(kill_mask)
  {
    TEST(info_marks_test_as_failed_sets_phase_child_kills_and_presents_exit_fail,
       fix)
    {
      using namespace crpcut::comm;
      static const char apa[] = { 'a', 'p', 'a' };
      reader.buffer.push_back(type(info | kill_me));
      reader.buffer.push_back(sizeof(apa));
      reader.buffer.push_back(apa);
      EXPECT_CALL(monitor, send_to_presentation(exit_fail, _, _)).
        With(Args<2, 1>(ElementsAreArray(apa)));
      verify_naughty_child();
    }

    TEST(begin_test_marks_as_failed_kills_and_presents_as_exit_fail_naughty,
         fix)
    {
      using namespace crpcut::comm;
      reader.buffer.push_back(type(begin_test | kill_me));
      struct timeval cpu = { 10, 1 };
      reader.buffer.push_back(sizeof(cpu));
      reader.buffer.push_back(cpu);
      EXPECT_CALL(monitor, get_location()).
        WillOnce(Return(loc));
      EXPECT_CALL(monitor,
                  send_to_presentation(exit_fail,
                                       _,
                                       StartsWith("apa:3\nA child process spawned")));
      verify_naughty_child();
    }

    TEST(set_timeout_marks_as_failed_kills_and_presents_as_exit_fail_naughty,
         fix)
    {
      using namespace crpcut::comm;
      reader.buffer.push_back(type(set_timeout | kill_me));
      crpcut::clocks::monotonic::timestamp now = 100;
      reader.buffer.push_back(sizeof(now));
      reader.buffer.push_back(now);
      EXPECT_CALL(monitor, get_location()).
        WillOnce(Return(loc));
      EXPECT_CALL(monitor,
                  send_to_presentation(exit_fail,
                                       _,
                                       StartsWith("apa:3\nA child process spawned")));
      verify_naughty_child();
    }

    TEST(cancel_timeout_marks_as_failed_kills_and_presents_as_exit_fail_naughty,
         fix)
    {
      using namespace crpcut::comm;
      reader.buffer.push_back(type(cancel_timeout | kill_me));
      reader.buffer.push_back(size_t(0));
      EXPECT_CALL(monitor, get_location()).
        WillOnce(Return(loc));
      EXPECT_CALL(monitor,
                  send_to_presentation(exit_fail,
                                       _,
                                       StartsWith("apa:3\nA child process spawned")));
      verify_naughty_child();
    }

    TEST(end_test_marks_as_failed_kills_and_presents_as_exit_fail_naughty,
         fix)
    {
      using namespace crpcut::comm;
      reader.buffer.push_back(type(end_test | kill_me));
      reader.buffer.push_back(size_t(0));
      EXPECT_CALL(monitor, get_location()).
        WillOnce(Return(loc));
      EXPECT_CALL(monitor,
                  send_to_presentation(exit_fail,
                                       _,
                                       StartsWith("apa:3\nA child process spawned")));
      verify_naughty_child();
    }

    TEST(info_marks_as_failed_kills_and_presents_as_exit_fail_w_msg,
         fix)
    {
      using namespace crpcut::comm;
      static const char apa[] = { 'a', 'p', 'a' };
      reader.buffer.push_back(type(info | kill_me));
      reader.buffer.push_back(sizeof(apa));
      reader.buffer.push_back(apa);
      EXPECT_CALL(monitor, send_to_presentation(exit_fail, _, _)).
        With(Args<2,1>(ElementsAreArray(apa)));
      verify_naughty_child();
    }

    TEST(fail_marks_as_failed_kills_and_presents_as_exit_fail_w_msg,
         fix)
    {
      using namespace crpcut::comm;
      static const char apa[] = { 'a', 'p', 'a' };
      reader.buffer.push_back(type(fail | kill_me));
      reader.buffer.push_back(sizeof(apa));
      reader.buffer.push_back(apa);
      EXPECT_CALL(monitor, send_to_presentation(exit_fail, _, _)).
        With(Args<2,1>(ElementsAreArray(apa)));
      verify_naughty_child();
    }

    TEST(exit_ok_marks_as_failed_kills_and_presents_as_exit_fail_naughty,
         fix)
    {
      using namespace crpcut::comm;
      reader.buffer.push_back(type(exit_ok | kill_me));
      reader.buffer.push_back(size_t(0));
      EXPECT_CALL(monitor, get_location()).
    	WillOnce(Return(loc));
      EXPECT_CALL(monitor,
                  send_to_presentation(exit_fail,
                                       _,
                                       StartsWith("apa:3\nA child process spawned")));
      verify_naughty_child();
    }

    TEST(exit_fail_marks_as_failed_kills_and_presents_as_exit_fail_w_msg,
         fix)
    {
      using namespace crpcut::comm;
      static const char apa[] = { 'a', 'p', 'a' };
      reader.buffer.push_back(type(exit_fail | kill_me));
      reader.buffer.push_back(sizeof(apa));
      reader.buffer.push_back(apa);
      EXPECT_CALL(monitor,
                  send_to_presentation(exit_fail, _, _)).
        With(Args<2,1>(ElementsAreArray(apa)));
      verify_naughty_child();
    }
  }
}

