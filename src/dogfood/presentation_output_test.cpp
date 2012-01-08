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
#include "../presentation_output.hpp"
#include "../poll.hpp"
#include "../posix_write.hpp"
#include "../output/buffer.hpp"
#include "../io.hpp"
namespace {
  using namespace testing;
  class test_poller : public crpcut::poll<crpcut::io>
  {
  public:
    MOCK_METHOD3(do_add_fd, void(int, crpcut::io*, int));
    MOCK_METHOD1(do_del_fd, void(int));
    MOCK_METHOD1(do_wait, descriptor(int));
    MOCK_CONST_METHOD0(do_num_fds, std::size_t());
  };

  class test_write : public crpcut::posix_write
  {
  public:
    MOCK_METHOD3(do_write, ssize_t(int, const void*, std::size_t));
  };

  class test_buffer : public crpcut::output::buffer
  {
  public:
    typedef std::pair<const char*, std::size_t> buff;
    MOCK_CONST_METHOD0(get_buffer, buff());
    MOCK_METHOD0(advance, void());
    MOCK_METHOD2(write, ssize_t(const char *, std::size_t));
    MOCK_CONST_METHOD0(is_empty, bool());
  };

  class fix
  {
  protected:
    test_buffer buffer;
    test_poller poller;
    test_write writer;
  };
}
TESTSUITE(presentation_output)
{
  TEST(create_and_destroy, fix)
  {
    crpcut::presentation_output obj(buffer, poller, 10, writer);
  }

  TEST(read_bombs, fix, EXPECT_SIGNAL_DEATH(SIGABRT), NO_CORE_FILE)
  {
    crpcut::presentation_output obj(buffer, poller, 10, writer);
    obj.read();
  }

  TEST(enable_registers_and_deregisters, fix)
  {
    crpcut::presentation_output obj(buffer, poller, 10, writer);
    ASSERT_FALSE(obj.enabled());
    EXPECT_CALL(poller, do_add_fd(10, &obj, test_poller::polltype::w))
      .Times(1);
    obj.enable(true);
    Mock::VerifyAndClearExpectations(&poller);
    ASSERT_TRUE(obj.enabled());
    EXPECT_CALL(poller, do_del_fd(10))
      .Times(1);
    obj.enable(false);
    ASSERT_FALSE(obj.enabled());
  }

#define S(...) test_buffer::buff(#__VA_ARGS__, sizeof(#__VA_ARGS__) - 1)
  TEST(write_sends_contents_of_buffer_to_writer, fix)
  {
    Sequence seq;
    crpcut::presentation_output obj(buffer, poller, 10, writer);
    EXPECT_CALL(poller, do_add_fd(10, &obj, test_poller::polltype::w))
      .InSequence(seq);
    obj.enable(1);
    test_buffer::buff buff_data[] = {
      S(hej hopp alla),
      S(barnen i bullerbyn)
    };
    EXPECT_CALL(buffer, is_empty())
      .WillRepeatedly(Return(false));
    EXPECT_CALL(buffer, get_buffer())
      .InSequence(seq)
      .WillOnce(Return(buff_data[0]));
    EXPECT_CALL(writer, do_write(10, "hej hopp alla", 13))
      .InSequence(seq)
      .WillOnce(Return(10));
    bool rv = obj.write();
    ASSERT_FALSE(rv);


    EXPECT_CALL(buffer, get_buffer())
      .InSequence(seq)
      .WillOnce(Return(buff_data[0]));
    EXPECT_CALL(writer, do_write(10, "lla", 3))
      .InSequence(seq)
      .WillOnce(Return(3));
    EXPECT_CALL(buffer, advance())
      .InSequence(seq);
    EXPECT_CALL(buffer, get_buffer())
      .InSequence(seq)
      .WillOnce(Return(buff_data[1]));
    EXPECT_CALL(writer, do_write(10, "barnen i bullerbyn", 18))
      .InSequence(seq)
      .WillOnce(Return(18));
    EXPECT_CALL(buffer, advance())
      .InSequence(seq);
    EXPECT_CALL(buffer, is_empty())
      .InSequence(seq)
      .WillOnce(Return(true));
    rv = obj.write();
    ASSERT_FALSE(rv);
  }
}
