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
namespace {
  class test_wfile_descriptor : public crpcut::comm::wfile_descriptor
  {
  public:
    test_wfile_descriptor() : wfile_descriptor() {}
    test_wfile_descriptor(int fd) : wfile_descriptor(fd) {}
    using file_descriptor::get_fd;
    MOCK_METHOD0(close, void());
    ssize_t write(const void *p, size_t n) const
    {
      return write(static_cast<const char*>(p), n);
    }
    MOCK_CONST_METHOD2(write, ssize_t(const char *, size_t));
  };
  class test_rfile_descriptor : public crpcut::comm::rfile_descriptor
  {
  public:
    test_rfile_descriptor() : rfile_descriptor() {}
    test_rfile_descriptor(int fd) : rfile_descriptor(fd) {}
    using file_descriptor::get_fd;
    virtual ssize_t read(void *p, size_t t) const
    {
      return read(static_cast<char*>(p), t);
    }
    MOCK_METHOD0(close, void());
    MOCK_CONST_METHOD2(read, ssize_t(char *, size_t));
  };

  static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";

  class posix_err_comp
  {
  public:
    posix_err_comp(int e, const char *s) : e_(e), s_(s) {}
    bool operator()(::crpcut::posix_error &e)
    {
      return e_ == e.get_errno() && (e.what() == s_ || std::string(s_) == e.what());
    }
  private:
    int e_;
    const char *s_;
  };

  int pending_errno;
  void set_errno()
  {
    errno = pending_errno;
  }
}

TESTSUITE(file_descriptor)
{
  using namespace testing;
  TESTSUITE(wfile_descriptor)
  {
    TEST(default_constructed_file_descriptor_does_nothing)
    {
      StrictMock<test_wfile_descriptor> d;
      ASSERT_TRUE(d.get_fd() == -1);
    }

    TEST(write_loop_constructs_in_chucks)
    {
      StrictMock<test_wfile_descriptor> d;
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
      StrictMock<test_wfile_descriptor> d;
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
      pending_errno = EINTR;
      InSequence s;
      StrictMock<test_wfile_descriptor> d;
      EXPECT_CALL(d, write(StartsWith(alphabet), 26)).
          Times(1).
          WillOnce(Return(10));
      EXPECT_CALL(d, write(StartsWith(alphabet + 10), 16)).
          Times(1).
          WillOnce(DoAll(InvokeWithoutArgs(set_errno),
                         Return(-1)));
      EXPECT_CALL(d, write(StartsWith(alphabet + 10), 16)).
          Times(1).
          WillOnce(Return(16));
      d.write_loop(alphabet, 26);
    }
  }

  TESTSUITE(rfile_descriptor)
  {
    TEST(default_constructed_file_descriptor_does_nothing)
    {
      StrictMock<test_rfile_descriptor> d;
      ASSERT_TRUE(d.get_fd() == -1);
    }

    TEST(read_loop_constructs_in_chunks)
    {
      StrictMock<test_rfile_descriptor> d;
      EXPECT_CALL(d, read(_, 26)).
          WillOnce(DoAll(SetArrayArgument<0>(alphabet, alphabet + 10),
                         Return(10)));
      EXPECT_CALL(d, read(_, 16)).
          WillOnce(DoAll(SetArrayArgument<0>(alphabet + 10, alphabet + 20),
                         Return(10)));
      EXPECT_CALL(d, read(_, 6)).
          WillOnce(DoAll(SetArrayArgument<0>(alphabet + 20, alphabet + 26),
                         Return(6)));
      char buff[30];
      buff[26] = 1;
      d.read_loop(buff, 26);
      ASSERT_TRUE(buff[26] == 1);
      buff[26] = 0;
      ASSERT_TRUE(buff == std::string(alphabet));
    }

    TEST(read_loop_throws_when_fd_closes)
    {
      const char *nullstr = 0;
      StrictMock<test_rfile_descriptor> d;
      EXPECT_CALL(d, read(_, 26)).
          WillOnce(DoAll(SetArrayArgument<0>(alphabet, alphabet + 10),
                         Return(10)));
      EXPECT_CALL(d, read(_, 16)).
          WillOnce(Return(0));

      char buff[30];

      ASSERT_THROW(d.read_loop(buff, 26),
                   crpcut::posix_error,
                   posix_err_comp(0, nullstr));
      ASSERT_TRUE(std::string(buff, 10) == std::string(alphabet, 10));
    }

    TEST(read_loop_continues_after_EINTR)
    {
      InSequence s;
      pending_errno = EINTR;
      StrictMock<test_rfile_descriptor> d;
      EXPECT_CALL(d, read(_, 26)).
          WillOnce(DoAll(SetArrayArgument<0>(alphabet, alphabet + 10),
                         Return(10)));
      EXPECT_CALL(d, read(_, 16)).
          WillOnce(DoAll(InvokeWithoutArgs(set_errno),
                         Return(-1)));
      EXPECT_CALL(d, read(_, 16)).
          WillOnce(DoAll(SetArrayArgument<0>(alphabet + 10, alphabet + 20),
                         Return(10)));
      EXPECT_CALL(d, read(_, 6)).
          WillOnce(DoAll(SetArrayArgument<0>(alphabet + 20, alphabet + 26),
                         Return(6)));
      char buff[30];
      buff[26] = 0;
      d.read_loop(buff, 26);
      ASSERT_TRUE(buff == std::string(alphabet));

    }
  }
}






