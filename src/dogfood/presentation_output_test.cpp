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
#include "../presentation_output.hpp"
#include "../poll.hpp"
#include "../posix_write.hpp"
#include "../output/buffer.hpp"
#include "../io.hpp"

using trompeloeil::_;
using trompeloeil::ne;

namespace {
  class test_poller : public crpcut::poll<crpcut::io>
  {
  public:
    MAKE_MOCK3(do_add_fd, void(int, crpcut::io*, int));
    MAKE_MOCK1(do_del_fd, void(int));
    MAKE_MOCK1(do_wait, descriptor(int));
    MAKE_CONST_MOCK0(do_num_fds, std::size_t());
  };

  class test_write : public crpcut::posix_write
  {
  public:
    MAKE_MOCK3(do_write, ssize_t(int, const char*, std::size_t));
    ssize_t do_write(int i, const void* p, std::size_t s)
    {
      return do_write(i, (const char*)p, s);
    }
  };

  class test_buffer : public crpcut::output::buffer
  {
  public:
    typedef std::pair<const char*, std::size_t> buff;
    MAKE_CONST_MOCK0(get_buffer, buff());
    MAKE_MOCK0(advance, void());
    MAKE_MOCK2(write, ssize_t(const char *, std::size_t));
    MAKE_CONST_MOCK0(is_empty, bool());
  };

  class fix
  {
  protected:
    test_buffer buffer;
    test_poller poller;
    test_write  writer;
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
    {
      REQUIRE_CALL(poller, do_add_fd(10, &obj, test_poller::polltype::w));
      obj.enable(true);
    }
    ASSERT_TRUE(obj.enabled());
    {
      REQUIRE_CALL(poller, do_del_fd(10));
      obj.enable(false);
    }
    ASSERT_FALSE(obj.enabled());
  }

#define S(...) test_buffer::buff(#__VA_ARGS__, sizeof(#__VA_ARGS__) - 1)

  TESTSUITE(write)
  {
    TEST(returns_after_partial_fd_write, fix)
    {
      trompeloeil::sequence seq;
      crpcut::presentation_output obj(buffer, poller, 10, writer);
      REQUIRE_CALL(poller, do_add_fd(10, &obj, test_poller::polltype::w))
        .IN_SEQUENCE(seq);
      obj.enable(1);
      test_buffer::buff buff_data[] = {
        S(hej hopp alla),
        S(barnen i bullerbyn)
      };
      REQUIRE_CALL(buffer, is_empty())
        .RETURN(false);
      REQUIRE_CALL(buffer, get_buffer())
        .IN_SEQUENCE(seq)
        .RETURN(buff_data[0]);
      REQUIRE_CALL(writer, do_write(10, ne<const char*>(nullptr), 13U))
        .WITH(std::string(_2, _3) == "hej hopp alla")
        .IN_SEQUENCE(seq)
        .RETURN(10);
      bool rv = obj.write();
      ASSERT_FALSE(rv);
    }

    TEST(advance_buffer_and_return_when_current_buffer_has_been_consumed,
         fix)
    {
      trompeloeil::sequence seq;
      crpcut::presentation_output obj(buffer, poller, 10, writer);
      REQUIRE_CALL(poller, do_add_fd(10, &obj, test_poller::polltype::w))
        .IN_SEQUENCE(seq);
      obj.enable(1);
      test_buffer::buff buff_data[] = {
        S(hej hopp alla),
        S(barnen i bullerbyn)
      };
      ALLOW_CALL(buffer, is_empty())
        .RETURN(false);
      REQUIRE_CALL(buffer, get_buffer())
        .IN_SEQUENCE(seq)
        .RETURN(buff_data[0]);
      REQUIRE_CALL(writer, do_write(10, ne<const char*>(nullptr), 13U))
        .WITH(std::string(_2,_3) == "hej hopp alla")
        .IN_SEQUENCE(seq)
        .RETURN(13);
      REQUIRE_CALL(buffer, advance())
        .IN_SEQUENCE(seq);
      bool rv = obj.write();
      ASSERT_FALSE(rv);
    }

    TEST(second_call_continues_after_partial_fd_write, fix,
         DEPENDS_ON(returns_after_partial_fd_write,
                    advance_buffer_and_return_when_current_buffer_has_been_consumed))
    {
      trompeloeil::sequence seq;
      crpcut::presentation_output obj(buffer, poller, 10, writer);
      REQUIRE_CALL(poller, do_add_fd(10, &obj, test_poller::polltype::w))
        .IN_SEQUENCE(seq);
      obj.enable(1);
      test_buffer::buff buff_data[] = {
        S(hej hopp alla),
        S(barnen i bullerbyn)
      };
      ALLOW_CALL(buffer, is_empty())
        .RETURN(false);
      REQUIRE_CALL(buffer, get_buffer())
        .IN_SEQUENCE(seq)
        .RETURN(buff_data[0]);
      REQUIRE_CALL(writer, do_write(10, ne<const char*>(nullptr), 13U))
        .WITH(std::string(_2, _3) == "hej hopp alla")
        .IN_SEQUENCE(seq)
        .RETURN(10);
      bool rv = obj.write();
      ASSERT_FALSE(rv);

      REQUIRE_CALL(buffer, get_buffer())
        .IN_SEQUENCE(seq)
        .RETURN(buff_data[0]);
      REQUIRE_CALL(writer, do_write(10, ne<const char*>(nullptr), 3U))
        .WITH(std::string(_2, _3) == "lla")
        .IN_SEQUENCE(seq)
        .RETURN(3);
      REQUIRE_CALL(buffer, advance())
        .IN_SEQUENCE(seq);
      rv = obj.write();
      ASSERT_FALSE(rv);
    }
  }
}
