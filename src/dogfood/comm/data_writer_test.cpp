/*
 * Copyright 2012,2016 Bjorn Fahller <bjorn@fahller.se>
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
  class test_writer : public crpcut::comm::data_writer
  {
  public:
    test_writer() : data_writer() {}
    MAKE_MOCK0(close, void());
    ssize_t write(const void *p, size_t n) const
    {
      return write(static_cast<const char*>(p), n);
    }
    MAKE_CONST_MOCK2(write, ssize_t(const char *, size_t));
  };

  static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";

}

TESTSUITE(comm)
{
  TESTSUITE(data_writer)
  {
    using trompeloeil::_;
    using trompeloeil::ne;

    TEST(write_loop_constructs_in_chucks)
    {
      test_writer d;
      REQUIRE_CALL(d, write(ne<const char*>(nullptr), 26U))
        .WITH(std::string(_1,_2) == alphabet)
        .RETURN(10);
      REQUIRE_CALL(d, write(ne<const char*>(nullptr), 16U))
        .WITH(std::string(_1,_2) == alphabet + 10)
        .RETURN(10);
      REQUIRE_CALL(d, write(ne<const char*>(nullptr), 6U))
        .WITH(std::string(_1,_2) == alphabet + 20)
        .RETURN(6);
      d.write_loop(alphabet, 26U);
    }

    TEST(write_loop_throws_when_fd_closes)
    {
      const char *nullstr = 0;
      test_writer d;
      REQUIRE_CALL(d, write(ne<const char*>(nullptr), 26U))
        .WITH(std::string(_1,_2) == alphabet)
        .RETURN(10);
      REQUIRE_CALL(d, write(ne<const char*>(nullptr), 16U))
        .WITH(std::string(_1, _2) == alphabet + 10)
        .RETURN(0);
      ASSERT_THROW(d.write_loop(alphabet, 26U),
                   crpcut::posix_error,
                   posix_err_comp(0, nullstr));
    }

    TEST(write_loop_continues_after_EINTR)
    {
      trompeloeil::sequence s;
      test_writer d;
      ALLOW_CALL(d, write(ne<const char*>(nullptr),_))
        .RETURN(ssize_t(_2));
      REQUIRE_CALL(d, write(ne<const char*>(nullptr), 26U))
        .IN_SEQUENCE(s)
        .WITH(std::string(_1,_2) == alphabet)
        .RETURN(10);

      REQUIRE_CALL(d, write(ne<const char*>(nullptr), 16U)) // actually matched last...
        .WITH(std::string(_1,_2) == alphabet + 10)
        .RETURN(16);

      REQUIRE_CALL(d, write(ne<const char*>(nullptr), 16U))
        .IN_SEQUENCE(s)
        .WITH(std::string(_1,_2) == alphabet + 10)
        .SIDE_EFFECT(errno = EINTR)
        .RETURN(-1);
      d.write_loop(alphabet, 26);
    }

  }
}






