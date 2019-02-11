/*
 * Copyright 2012-2013,2016 Bjorn Fahller <bjorn@fahller.se>
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

namespace {
  using fs = crpcut::datatypes::fixed_string;
  static const fs location{"apa.cpp:32"};
}
TESTSUITE(comm)
{
  TESTSUITE(reporter)
  {
    class writer_mock : public crpcut::comm::data_writer
    {
    public:
      MAKE_CONST_MOCK3(write_loop,
                       const crpcut::comm::data_writer&(const char*,
                                                        std::size_t,
                                                        const char*));
      const data_writer& write_loop(const void *addr, std::size_t len,
                                    const char *context) const
      {
         write_loop(static_cast<const char*>(addr), len, context);
         return *this;
      }
      MAKE_CONST_MOCK2(write, ssize_t(const void*, std::size_t));
    };

    class monitor_mock : public crpcut::crpcut_test_monitor
    {
    public:
      MAKE_MOCK1(set_timeout, void(unsigned long));
      MAKE_CONST_MOCK0(duration_us, unsigned long());
      MAKE_CONST_MOCK0(deadline_is_set, bool());
      MAKE_MOCK0(clear_deadline, void());
      MAKE_MOCK1(crpcut_register_success, void(bool));
      MAKE_MOCK1(set_phase, void(crpcut::test_phase));
      MAKE_MOCK0(kill, void());
      MAKE_CONST_MOCK0(crpcut_failed, bool());
      MAKE_MOCK1(set_cputime_at_start, void(const struct timeval&));
      MAKE_CONST_MOCK3(send_to_presentation, void(crpcut::comm::type, size_t, const char*));
      MAKE_MOCK0(set_death_note, void());
      MAKE_MOCK0(activate_reader, void());
      MAKE_MOCK0(deactivate_reader, void());
      MAKE_CONST_MOCK0(has_active_readers, bool());
      MAKE_MOCK0(manage_death, void());
      MAKE_CONST_MOCK0(is_naughty_child, bool());
      MAKE_CONST_MOCK0(freeze, void());
      MAKE_CONST_MOCK0(get_location, crpcut::datatypes::fixed_string());
    };

    using trompeloeil::_;
    using trompeloeil::ne;
    
    TEST(reporter_without_current_process_prints_on_stream)
    {
       std::ostringstream os;
       {
         crpcut::comm::reporter r(os);
         const crpcut::crpcut_test_monitor *no_monitor = nullptr;
         r(crpcut::comm::info, "apa", location, no_monitor);
       }
       ASSERT_TRUE(os.str() == "\napa.cpp:32\napa\n");
    }

    TEST(reporter_without_current_process_prints_and_aborts_for_exit_fail,
         EXPECT_SIGNAL_DEATH(SIGABRT),
         NO_CORE_FILE)
    {
      std::ostringstream os;
      crpcut::comm::reporter r(os);
      const crpcut::crpcut_test_monitor *no_monitor = nullptr;
      r(crpcut::comm::exit_fail, "apa", location, no_monitor);
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
      writer_mock            wfd;
      monitor_mock           mon;
      std::ostringstream     os;
      crpcut::comm::reporter r(os);

      r.set_writer(&wfd);

      using namespace crpcut::comm;
      char request[sizeof(type) + 2*sizeof(size_t) + 3 + 10];
      void *addr = set_to(request, type(fail | kill_me));
      char *p = static_cast<char*>(set_to(addr, sizeof(size_t) + 3 + 10));
      p = static_cast<char*>(set_to(p, size_t(10)));
      memcpy(p, "apa.cpp:32apa", 10 + 3);

      REQUIRE_CALL(mon, is_naughty_child())
        .RETURN(true);

      REQUIRE_CALL(wfd, write_loop(ne<const char*>(nullptr), sizeof(request),_))
        .WITH(memcmp(_1, request, _2) == 0)
        .LR_RETURN(std::ref(wfd));
      r(crpcut::comm::fail, "apa", location, &mon);
    }


    struct fix
    {
      fix()
        : wfd(),
          child_x(NAMED_ALLOW_CALL(mon, is_naughty_child())
                  .RETURN(false)),
          r(fake_cout)
      {
        r.set_writer(&wfd);
      }
      monitor_mock                              mon;
      writer_mock                               wfd;
      std::ostringstream                        fake_cout;
      std::unique_ptr<trompeloeil::expectation> child_x;
      crpcut::comm::reporter                    r;
    };

    TEST(reporter_exits_on_exit_fail,
         EXPECT_EXIT(0), fix)
    {
      char request[sizeof(crpcut::comm::type) + 2*sizeof(size_t) + 3 + 10];
      void *addr = set_to(request, crpcut::comm::exit_fail);
      char *p = static_cast<char*>(set_to(addr, sizeof(size_t) + 3 + 10));
      p = static_cast<char*>(set_to(p, std::size_t(10)));
      memcpy(p, "apa.cpp:32apa", 3 + 10);

      REQUIRE_CALL(wfd, write_loop(ne<const char*>(nullptr), sizeof(request),_))
        .WITH(memcmp(_1, request, _2) == 0)
        .RETURN(std::ref(wfd));
      r(crpcut::comm::exit_fail, "apa", location, &mon);
    }

    TEST(reporter_forwards_contents_from_ostringstream, fix)
    {
      char request[sizeof(crpcut::comm::type) + 2*sizeof(size_t) + 3 + 10];
      void *addr = set_to(request, crpcut::comm::fail);
      char *p = static_cast<char*>(set_to(addr, sizeof(size_t) + 3 + 10));
      p = static_cast<char*>(set_to(p, size_t(10)));
      memcpy(p, "apa.cpp:32apa", 10 + 3);

      REQUIRE_CALL(wfd, write_loop(ne<const char*>(nullptr), sizeof(request),_))
        .WITH(memcmp(_1, request, _2) == 0)
        .RETURN(std::ref(wfd));

      std::ostringstream os;
      os << "apa";
      r(crpcut::comm::fail, os, location, &mon);
    }

    TEST(reporter_forwards_contents_from_oastream, fix)
    {
      char request[sizeof(crpcut::comm::type) + 2*sizeof(size_t) + 3 + 10];
      void *addr = set_to(request, crpcut::comm::info);
      char *p = static_cast<char*>(set_to(addr, sizeof(size_t) + 3 + 10));
      p = static_cast<char*>(set_to(p, size_t(10)));
      memcpy(p, "apa.cpp:32apa", 10 + 3);

      REQUIRE_CALL(wfd, write_loop(ne<const char*>(nullptr), sizeof(request),_))
        .WITH(memcmp(_1, request, _2) == 0)
        .RETURN(std::ref(wfd));

      crpcut::stream::toastream<3> os;
      os << "apa";
      r(crpcut::comm::info, os, location, &mon);
    }

    TEST(reporter_forwards_contents_from_char_array, fix)
    {
      char request[sizeof(crpcut::comm::type) + 2*sizeof(size_t) + 3 + 10];
      void *addr = set_to(request, crpcut::comm::info);
      char *p = static_cast<char*>(set_to(addr, sizeof(size_t) + 3 + 10));
      p = static_cast<char*>(set_to(p, size_t(10)));
      memcpy(p, "apa.cpp:32apa", 10 + 3);

      REQUIRE_CALL(wfd, write_loop(ne<const char*>(nullptr), sizeof(request), _))
        .WITH(memcmp(_1, request, _2) == 0)
        .RETURN(std::ref(wfd));

      static const char apa[] = "apa";
      r(crpcut::comm::info, apa, location, &mon);
    }
  }
}
