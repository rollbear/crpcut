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
#include "../../test_environment.hpp"

TESTSUITE(comm)
{
  TESTSUITE(reporter)
  {
    using namespace testing;
    class wfd_mock : public crpcut::comm::wfile_descriptor
    {
    public:
      MOCK_CONST_METHOD3(write_loop, void(const char*, std::size_t,
                                          const char*));
      void write_loop(const void *addr, std::size_t len,
                      const char *context) const
      {
         write_loop(static_cast<const char*>(addr), len, context);
      }
    };

    class rfd_mock : public crpcut::comm::rfile_descriptor
    {
    public:
      MOCK_CONST_METHOD3(read_loop, void(char *, std::size_t, const char*));
      void read_loop(void *addr, std::size_t len, const char *context) const
      {
        read_loop(static_cast<char*>(addr), len, context);
      }
    };
    TEST(reporter_without_test_environment_prints_on_stream)
    {
       std::ostringstream os;
       {
         crpcut::comm::reporter r(os);
         r(crpcut::comm::info, "apa");
       }
       ASSERT_TRUE(os.str() == "\napa");
    }

    TEST(reporter_without_test_environment_prints_and_aborts_for_exit_fail,
         EXPECT_SIGNAL_DEATH(SIGABRT),
         NO_CORE_FILE)
    {
      std::ostringstream os;
      crpcut::comm::reporter r(os);
      r(crpcut::comm::exit_fail, "apa");
    }

    TEST(naughty_children_are_killed_by_starvation,
         EXPECT_EXIT(1))
    {
      class env : public crpcut::test_environment
      {
      public:
        bool is_naughty_child() const { return true; }
        void freeze() const { exit(1); }
      };

      env e;
      wfd_mock wfd;
      rfd_mock rfd;
      std::ostringstream os;
      crpcut::comm::reporter r(os);
      r.set_test_environment(&e);
      r.set_write_fd(&wfd);
      r.set_read_fd(&rfd);
      EXPECT_CALL(wfd, write_loop(_, _, _));
      EXPECT_CALL(rfd, read_loop(_,_,_)).
        WillOnce(Throw(3));
      r(crpcut::comm::fail, "apa");
    }

    template <typename T>
    void *set_to(void *addr, T t)
    {
      *static_cast<T*>(addr) = t;
      return static_cast<char*>(addr) + sizeof(T);
    }

    struct fix
    {
      fix() : e(), wfd(), rfd(), r(fake_cout)
      {
        r.set_test_environment(&e);
        r.set_write_fd(&wfd);
        r.set_read_fd(&rfd);
      }
      crpcut::test_environment e;
      wfd_mock                 wfd;
      rfd_mock                 rfd;
      std::ostringstream       fake_cout;
      crpcut::comm::reporter   r;
    };

    TEST(reporter_exits_on_exit_fail,
         EXPECT_EXIT(0), fix)
    {
      char request[sizeof(crpcut::comm::type) + sizeof(size_t) + 3];
      void *addr = set_to(request, crpcut::comm::exit_fail);
      char *p = static_cast<char*>(set_to(addr, std::size_t(3)));
      *p++='a'; *p++='p'; *p++='a';
      EXPECT_CALL(wfd, write_loop(_, sizeof(request),_)).
          With(Args<0,1>(ElementsAreArray(request)));
      char response[sizeof(std::size_t)];
      set_to(response, std::size_t(3));
      EXPECT_CALL(rfd, read_loop(_,_,_)).
          WillOnce(SetArrayArgument<0>(&response[0], response + sizeof(size_t)));
      r(crpcut::comm::exit_fail, "apa");
    }

    TEST(reporter_forwards_contents_from_ostringstream, fix)
    {
      char request[sizeof(crpcut::comm::type) + sizeof(size_t) + 3];
      void *addr = set_to(request, crpcut::comm::fail);
      char *p = static_cast<char*>(set_to(addr, std::size_t(3)));
      *p++='a'; *p++='p'; *p++='a';
      EXPECT_CALL(wfd, write_loop(_, sizeof(request),_)).
          With(Args<0,1>(ElementsAreArray(request)));
      char response[sizeof(std::size_t)];
      set_to(response, std::size_t(3));
      EXPECT_CALL(rfd, read_loop(_,_,_)).
          WillOnce(SetArrayArgument<0>(&response[0], response + sizeof(size_t)));
      std::ostringstream os;
      os << "apa";
      r(crpcut::comm::fail, os);
    }

    TEST(reporter_forwards_contents_from_oastream, fix)
    {
      char request[sizeof(crpcut::comm::type) + sizeof(size_t) + 3];
      void *addr = set_to(request, crpcut::comm::info);
      char *p = static_cast<char*>(set_to(addr, std::size_t(3)));
      *p++='a'; *p++='p'; *p++='a';
      EXPECT_CALL(wfd, write_loop(_, sizeof(request),_)).
          With(Args<0,1>(ElementsAreArray(request)));
      char response[sizeof(std::size_t)];
      set_to(response, std::size_t(3));
      EXPECT_CALL(rfd, read_loop(_,_,_)).
          WillOnce(SetArrayArgument<0>(&response[0], response + sizeof(size_t)));

      crpcut::stream::toastream<3> os;
      os << "apa";
      r(crpcut::comm::info, os);
    }

    TEST(reporter_aborts_on_wrong_response, fix,
         EXPECT_SIGNAL_DEATH(SIGABRT),
         NO_CORE_FILE)
    {
      EXPECT_CALL(wfd, write_loop(_,_,_));
      char response[sizeof(std::size_t)];
      set_to(response, std::size_t(4));
      EXPECT_CALL(rfd, read_loop(_,_,_)).
        WillOnce(SetArrayArgument<0>(&response[0], response + sizeof(size_t)));
      r(crpcut::comm::fail, "apa");
    }
  }
}
