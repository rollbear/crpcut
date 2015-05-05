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

#include <trompeloeil.hpp>
#include <crpcut.hpp>
#include "../../posix_error.hpp"
#include "posix_err_comp.hpp"

namespace {

  class test_reader : public crpcut::comm::data_reader
  {
  public:
    test_reader() : data_reader() {}
    virtual ssize_t read(void *p, size_t t) const
    {
      return read(static_cast<char*>(p), t);
    }
    MAKE_MOCK0(close, void());
    MAKE_CONST_MOCK2(read, ssize_t(char *, size_t));
  };

  static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
}

TESTSUITE(comm)
{
  TESTSUITE(data_reader)
  {
    using trompeloeil::_;

    TEST(read_loop_constructs_in_chunks)
    {
      test_reader d;

      REQUIRE_CALL(d, read(_, 26U))
        .SIDE_EFFECT(memcpy(_1, alphabet, 10))
        .RETURN(10);

      REQUIRE_CALL(d, read(_, 16U))
        .SIDE_EFFECT(memcpy(_1, alphabet + 10, 10))
        .RETURN(10);

      REQUIRE_CALL(d, read(_, 6U))
        .SIDE_EFFECT(memcpy(_1, alphabet + 20, 6))
        .RETURN(6);

      char buff[30];
      buff[26] = 1;
      d.read_loop(buff, 26U);
      ASSERT_TRUE(buff[26] == 1);
      buff[26] = 0;
      ASSERT_TRUE(buff == std::string(alphabet));
    }

    TEST(read_loop_throws_when_fd_closes)
    {
      const char *nullstr = 0;
      test_reader d;

      REQUIRE_CALL(d, read(_, 26U))
        .SIDE_EFFECT(memcpy(_1, alphabet, 10))
        .RETURN(10);

      REQUIRE_CALL(d, read(_, 16U))
        .RETURN(0);

      char buff[30];

      ASSERT_THROW(d.read_loop(buff, 26),
                   crpcut::posix_error,
                   posix_err_comp(0, nullstr));
      ASSERT_TRUE(std::string(buff, 10) == std::string(alphabet, 10));
    }

    TEST(read_loop_continues_after_EINTR)
    {
      trompeloeil::sequence s;

      test_reader d;

      REQUIRE_CALL(d, read(_, 26U))
        .IN_SEQUENCE(s)
        .SIDE_EFFECT(memcpy(_1, alphabet, 10))
        .RETURN(10);

      REQUIRE_CALL(d, read(_, 16U))
        .IN_SEQUENCE(s)
        .SIDE_EFFECT(memcpy(_1, alphabet + 10, 10))
        .RETURN(10);

      REQUIRE_CALL(d, read(_, 6U))
        .IN_SEQUENCE(s)
        .SIDE_EFFECT(memcpy(_1, alphabet + 20, 6))
        .RETURN(6);

      REQUIRE_CALL(d, read(_, 16U))
        .SIDE_EFFECT(errno = EINTR)
        .RETURN(-1);

      char buff[30];
      buff[26] = 0;
      d.read_loop(buff, 26U);
      ASSERT_TRUE(buff == std::string(alphabet));

    }
  }
}






