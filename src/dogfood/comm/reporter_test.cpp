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
#include "../../current_process.hpp"

TESTSUITE(comm)
{
  TESTSUITE(reporter)
  {
    using namespace testing;
    class writer_mock : public crpcut::comm::data_writer
    {
    public:
      MOCK_CONST_METHOD3(write_loop,
                         const crpcut::comm::data_writer&(const char*,
                                                          std::size_t,
                                                          const char*));
      const data_writer& write_loop(const void *addr, std::size_t len,
                                    const char *context) const
      {
         write_loop(static_cast<const char*>(addr), len, context);
         return *this;
      }
      MOCK_CONST_METHOD2(write, ssize_t(const void*, std::size_t));
    };

    TEST(reporter_without_current_process_prints_on_stream)
    {
       std::ostringstream os;
       {
         crpcut::comm::reporter r(os);
         r(crpcut::comm::info, "apa");
       }
       ASSERT_TRUE(os.str() == "\napa");
    }

    TEST(reporter_without_current_process_prints_and_aborts_for_exit_fail,
         EXPECT_SIGNAL_DEATH(SIGABRT),
         NO_CORE_FILE)
    {
      std::ostringstream os;
      crpcut::comm::reporter r(os);
      r(crpcut::comm::exit_fail, "apa");
    }

    template <typename T>
    void *set_to(void *addr, T t)
    {
      *static_cast<T*>(addr) = t;
      return static_cast<char*>(addr) + sizeof(T);
    }

    TEST(naughty_children_requests_testicide_end_exits,
         EXPECT_EXIT(1))
    {
      class env : public crpcut::current_process
      {
      public:
        env()
          : current_process(crpcut::datatypes::fixed_string::make("apa:3"))
        {
        }
        bool is_naughty_child() const { return true; }
        void freeze() const { exit(1); }
      };

      env                     e;
      StrictMock<writer_mock> wfd;
      std::ostringstream      os;
      crpcut::comm::reporter  r(os);

      r.set_process_control(&e);
      r.set_writer(&wfd);

      using namespace crpcut::comm;
      char request[sizeof(type) + sizeof(size_t) + 3];
      void *addr = set_to(request, type(fail | kill_me));
      char *p = static_cast<char*>(set_to(addr, std::size_t(3)));
      *p++='a'; *p++='p'; *p++='a';
      EXPECT_CALL(wfd, write_loop(_, sizeof(request),_))
        .With(Args<0,1>(ElementsAreArray(request)))
        .WillOnce(ReturnRef(wfd));
      r(crpcut::comm::fail, "apa");
    }


    struct fix
    {
      fix()
        : e(crpcut::datatypes::fixed_string::make("apa:3")),
            wfd(),
            r(fake_cout)
      {
        r.set_process_control(&e);
        r.set_writer(&wfd);
      }
      crpcut::current_process  e;
      StrictMock<writer_mock>  wfd;
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
      EXPECT_CALL(wfd, write_loop(_, sizeof(request),_))
        .With(Args<0,1>(ElementsAreArray(request)))
        .WillOnce(ReturnRef(wfd));
      r(crpcut::comm::exit_fail, "apa");
    }

    TEST(reporter_forwards_contents_from_ostringstream, fix)
    {
      char request[sizeof(crpcut::comm::type) + sizeof(size_t) + 3];
      void *addr = set_to(request, crpcut::comm::fail);
      char *p = static_cast<char*>(set_to(addr, std::size_t(3)));
      *p++='a'; *p++='p'; *p++='a';
      EXPECT_CALL(wfd, write_loop(_, sizeof(request),_))
        .With(Args<0,1>(ElementsAreArray(request)))
        .WillOnce(ReturnRef(wfd));
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
      EXPECT_CALL(wfd, write_loop(_, sizeof(request),_))
        .With(Args<0,1>(ElementsAreArray(request)))
        .WillOnce(ReturnRef(wfd));

      crpcut::stream::toastream<3> os;
      os << "apa";
      r(crpcut::comm::info, os);
    }

    TEST(reporter_forwards_contents_from_char_array, fix)
    {
      char request[sizeof(crpcut::comm::type) + sizeof(size_t) + 3];
      void *addr = set_to(request, crpcut::comm::info);
      char *p = static_cast<char*>(set_to(addr, std::size_t(3)));
      *p++='a'; *p++='p'; *p++='a';
      EXPECT_CALL(wfd, write_loop(_, sizeof(request), _))
        .With(Args<0,1>(ElementsAreArray(request)))
        .WillOnce(ReturnRef(wfd));
      static const char apa[] = "apa";
      r(crpcut::comm::info, apa);
    }
  }
}
