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
#include "../../posix_error.hpp"
#include "posix_err_comp.hpp"

namespace {
    class test_writer : public crpcut::comm::data_writer
  {
  public:
    test_writer() : data_writer() {}
    MOCK_METHOD0(close, void());
    ssize_t write(const void *p, size_t n) const
    {
      return write(static_cast<const char*>(p), n);
    }
    MOCK_CONST_METHOD2(write, ssize_t(const char *, size_t));
  };

  static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";

}

TESTSUITE(comm)
{
  TESTSUITE(data_writer)
  {
    using namespace testing;

    TEST(write_loop_constructs_in_chucks)
    {
      StrictMock<test_writer> d;
      EXPECT_CALL(d, write(StartsWith(alphabet), 26)).
          WillOnce(Return(10));
      EXPECT_CALL(d, write(StartsWith(alphabet + 10), 16)).
          WillOnce(Return(10));
      EXPECT_CALL(d, write(StartsWith(alphabet + 20), 6)).
          WillOnce(Return(6));
      d.write_loop(alphabet, 26);
    }

    TEST(write_loop_throws_when_fd_closes)
    {
      const char *nullstr = 0;
      StrictMock<test_writer> d;
      EXPECT_CALL(d, write(StartsWith(alphabet), 26)).
          WillOnce(Return(10));
      EXPECT_CALL(d, write(StartsWith(alphabet + 10), 16)).
          WillOnce(Return(0));
      ASSERT_THROW(d.write_loop(alphabet, 26),
                   crpcut::posix_error,
                   posix_err_comp(0, nullstr));
    }

    TEST(write_loop_continues_after_EINTR)
    {
      InSequence s;
      StrictMock<test_writer> d;
      EXPECT_CALL(d, write(StartsWith(alphabet), 26)).
          Times(1).
          WillOnce(Return(10));
      EXPECT_CALL(d, write(StartsWith(alphabet + 10), 16)).
          Times(1).
          WillOnce(SetErrnoAndReturn(EINTR, -1));
      EXPECT_CALL(d, write(StartsWith(alphabet + 10), 16)).
          Times(1).
          WillOnce(Return(16));
      d.write_loop(alphabet, 26);
    }

  }
}






