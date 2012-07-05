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
#include "../clocks/clocks.hpp"
using testing::StrictMock;
using testing::_;
using testing::Return;
using testing::Sequence;

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
      ASSERT_TRUE(current.len == bytes);
      memcpy(addr, current.addr, bytes);
    }

    struct data
    {
      data(const void *p, size_t l) : addr(p), len(l) {}
      template <typename T>
      data(const T& t) : addr(&t), len(sizeof(t)) {}
      const void *addr;
      size_t len;
    };
    mutable std::deque<data> buffer;
  };

  class mock_monitor : public crpcut::crpcut_test_monitor
  {
  public:
    MOCK_METHOD1(set_timeout, void(unsigned long));
    MOCK_CONST_METHOD0(deadline_is_set, bool());
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
  };


  struct fix
  {
    typedef crpcut::clocks::monotonic::timestamp ts;
    fix() : reader(&monitor) {}
    StrictMock<mock_monitor>      monitor;
    StrictMock<test_reader>      reader;
  };
}
TESTSUITE(report_reader)
{
  TEST(construction_and_immediate_destruction_does_nothing)
  {
     fix f;
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
}

