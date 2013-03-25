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

#include <crpcut.hpp>

TESTSUITE(scope)
{
  TESTSUITE(time)
  {
    class test_clock {
    public:
      static const char *name() { return  "TEST_TIME"; }
      static unsigned long now() { return ts_us_absolute; }
      static void set_timestamp_us(unsigned long us) { ts_us_absolute = us; }
    private:
      static unsigned long ts_us_absolute;
    };

    unsigned long test_clock::ts_us_absolute = 0;

    struct fix
    {
      fix() : r(os) {}
      std::ostringstream os;
      crpcut::comm::reporter r;
    };

    TEST(when_no_time_has_passed_a_zero_max_time_limit_gives_no_report,
         fix)
    {
      {
        using crpcut::comm::fail;
        using crpcut::scope::time;
        typedef crpcut::scope::time_base::max max;
        time<fail, max, test_clock> t(0, "apa", 3, r, 0);
      }
      ASSERT_TRUE(os.str() == "");
    }

    TEST(when_no_time_has_passed_a_zero_min_time_limit_gives_no_report,
         fix)
    {
      {
        using crpcut::comm::fail;
        using crpcut::scope::time;
        typedef crpcut::scope::time_base::min min;
        time<fail, min, test_clock> t(0, "apa", 3, r, 0);
      }
      ASSERT_TRUE(os.str() == "");
    }

    TEST(when_1ms_has_passed_a_zero_max_time_limit_gives_a_report,
         fix)
    {
      {
        using crpcut::comm::fail;
        using crpcut::scope::time;
        typedef crpcut::scope::time_base::max max;
        time<fail, max, test_clock> t(0, "apa", 3, r, 0);
        test_clock::set_timestamp_us(1000UL);
      }
      static const char report[] =
          "\napa:3\n"
          "VERIFY_SCOPE_MAX_TEST_TIME_MS(0)\n"
          "Actual time used was 1ms\n";
      ASSERT_TRUE(os.str() == report);
    }

    TEST(when_1ms_has_passed_a_zero_min_time_limit_gives_no_report,
         fix)
    {
      {
        using crpcut::comm::fail;
        using crpcut::scope::time;
        typedef crpcut::scope::time_base::min min;
        time<fail, min, test_clock> t(0, "apa", 3, r, 0);
        test_clock::set_timestamp_us(1000UL);
      }
      ASSERT_TRUE(os.str() == "");
    }

    TEST(when_0ms_has_passed_a_1ms_min_time_limit_gives_a_report,
         fix)
    {
      {
        using crpcut::comm::fail;
        using crpcut::scope::time;
        typedef crpcut::scope::time_base::min min;
        time<fail, min, test_clock> t(1000, "apa", 3, r, 0);
      }
      static const char report[] =
          "\napa:3\n"
          "VERIFY_SCOPE_MIN_TEST_TIME_MS(1)\n"
          "Actual time used was 0ms\n";
      ASSERT_TRUE(os.str() == report);
    }

    TEST(copying_of_a_busted_limit_gives_only_one_report,
         fix)
    {
      {
        using crpcut::comm::fail;
        using crpcut::scope::time;
        typedef crpcut::scope::time_base::max max;
        time<fail, max, test_clock> t(0, "apa", 3, r, 0);
        test_clock::set_timestamp_us(1000UL);
        crpcut::scope::time<fail, max, test_clock> t2(t);
        crpcut::scope::time<fail, max, test_clock> t3(t2);
      }
      static const char report[] =
          "\napa:3\n"
          "VERIFY_SCOPE_MAX_TEST_TIME_MS(0)\n"
          "Actual time used was 1ms\n";
      ASSERT_TRUE(os.str() == report);
    }

    TEST(name_of_realtime_clock_is_REALTIME)
    {
      using crpcut::scope::time_base;
      ASSERT_TRUE(std::string(time_base::realtime::name()) == "REALTIME");
    }

    TEST(name_of_cputime_clock_is_CPUTIME)
    {
      using crpcut::scope::time_base;
      ASSERT_TRUE(std::string(time_base::cputime::name()) == "CPUTIME");
    }
  }
}





