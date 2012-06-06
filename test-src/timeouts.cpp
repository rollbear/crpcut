/*
 * Copyright 2009-2011 Bjorn Fahller <bjorn@fahller.se>
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
extern "C" {
#include <sys/times.h>
#include <unistd.h>
}

DEFINE_TEST_TAG(slow);

TESTSUITE(timeouts)
{
  TEST(should_succeed_slow_realtime_deadline,
       DEADLINE_REALTIME_MS(200),
       NO_CORE_FILE)
  {
    usleep(50000*crpcut::timeout_multiplier());
  }

  TEST(should_fail_slow_realtime_deadline,
       DEADLINE_REALTIME_MS(100),
       NO_CORE_FILE,
       WITH_TEST_TAG(slow))
  {
    usleep(200000*crpcut::timeout_multiplier());
  }

  TEST(should_succeed_slow_cputime_deadline, DEADLINE_CPU_MS(100), NO_CORE_FILE)
  {
    usleep(300000*crpcut::timeout_multiplier());
    // should usleep busy-wait, this test would fail miserably
  }

  TEST(should_fail_slow_cputime_deadline,
       DEADLINE_CPU_MS(100),
       DEADLINE_REALTIME_MS(8000),
       NO_CORE_FILE,
       WITH_TEST_TAG(slow))
  {
    const clock_t ticks_per_sec = sysconf(_SC_CLK_TCK);
    tms t;
    times(&t);
    const size_t factor = crpcut::timeout_multiplier();
    clock_t deadline = (t.tms_utime + t.tms_stime + ticks_per_sec/5)*factor;
    for (;;)
      {
        for (volatile int n = 0; n < 100000; ++n)
          ;
        times(&t);
        if (t.tms_utime + t.tms_stime > deadline) break;
      }
  }

  TEST(should_fail_slow_cputime_deadline_by_death,
       DEADLINE_CPU_MS(100),
       DEADLINE_REALTIME_MS(8000),
       NO_CORE_FILE,
       WITH_TEST_TAG(slow))
  {
    for (;;)
      {
      }
  }

  TEST(should_fail_slow_realtime_deadline_by_death,
       DEADLINE_REALTIME_MS(100),
       NO_CORE_FILE,
       WITH_TEST_TAG(slow))
  {
    sleep(2*crpcut::timeout_multiplier());
  }

  template <bool constructor_cond, bool destructor_cond>
  class stuck_fixture
  {
  public:
    stuck_fixture() { while (constructor_cond) ; }
    ~stuck_fixture() { while (destructor_cond) ; }
  };

  TEST(should_fail_slow_save_from_stuck_constructor,
       WITH_TEST_TAG(slow),
       stuck_fixture<true, false>)
  {
  }

  TEST(should_fail_quick_save_from_stuck_constructor,
       FIXTURE_CONSTRUCTION_DEADLINE_REALTIME_MS(10),
       WITH_TEST_TAG(slow),
       stuck_fixture<true, false>)
  {
  }

  TEST(should_fail_slow_save_from_stuck_destructor,
       WITH_TEST_TAG(slow),
       stuck_fixture<false, true>)
  {
  }

  TEST(should_fail_quick_save_from_stuck_destructor,
       FIXTURE_DESTRUCTION_DEADLINE_REALTIME_MS(10),
       WITH_TEST_TAG(slow),
       stuck_fixture<false, true>)
  {
  }

  TESTSUITE(expected)
  {
    TEST(should_succeed_sleep,
         EXPECT_REALTIME_TIMEOUT_MS(100),
         WITH_TEST_TAG(slow))
      {
        sleep(2*crpcut::timeout_multiplier());
      }

    TEST(should_fail_early_return,
         EXPECT_REALTIME_TIMEOUT_MS(100))
    {
    }

    TEST(should_fail_cputime,
         EXPECT_REALTIME_TIMEOUT_MS(100),
         DEADLINE_CPU_MS(3),
         WITH_TEST_TAG(slow))
    {
      for (;;)
        ;
    }

    TEST(should_succeed_cputime,
         EXPECT_REALTIME_TIMEOUT_MS(100),
         DEADLINE_CPU_MS(10),
         WITH_TEST_TAG(slow))
    {
      sleep(3*crpcut::timeout_multiplier());
    }

  }

  TESTSUITE(scoped)
  {

    TEST(should_succeed_realtime_short_sleep)
    {
      ASSERT_SCOPE_MAX_REALTIME_MS(10)
      {
        usleep(1000*crpcut::timeout_multiplier());
      }
    }

    TEST(should_fail_realtime_short_sleep, WITH_TEST_TAG(slow))
    {
      ASSERT_SCOPE_MAX_REALTIME_MS(15)
      {
        usleep(20000*crpcut::timeout_multiplier());
      }
    }

    TEST(should_succeed_oversleep)
    {
      ASSERT_SCOPE_MIN_REALTIME_MS(35)
      {
        ASSERT_SCOPE_MAX_CPUTIME_MS(5)
        {
          usleep(40000*crpcut::timeout_multiplier());
        }
      }
    }

    TEST(should_fail_cputime_long,
         DEADLINE_REALTIME_MS(2000),
         WITH_TEST_TAG(slow))
    {
      const clock_t clocks_per_tick = sysconf(_SC_CLK_TCK);
      tms t;
      times(&t);
      const size_t factor = crpcut::timeout_multiplier();
      clock_t deadline = (t.tms_utime + t.tms_stime + clocks_per_tick)*factor;
      ASSERT_SCOPE_MAX_CPUTIME_MS(900)
      {
        ASSERT_SCOPE_MIN_REALTIME_MS(1000)
        {
          for (;;)
            {
              for (volatile int n = 0; n < 100000; ++n)
                ;
              times(&t);
              if (t.tms_utime + t.tms_stime > deadline) break;
            }
        }
      }
    }

    TEST(should_succeed_verify_realtime_short_sleep)
    {
      VERIFY_SCOPE_MAX_REALTIME_MS(10)
      {
        usleep(1000 * crpcut::timeout_multiplier());
      }
      INFO << "after";
    }

    TEST(should_fail_verify_realtime_short_sleep, WITH_TEST_TAG(slow))
    {
      VERIFY_SCOPE_MAX_REALTIME_MS(15)
      {
        usleep(20000*crpcut::timeout_multiplier());
      }
      INFO << "after";
    }

    TEST(should_succeed_verify_oversleep)
    {
      VERIFY_SCOPE_MIN_REALTIME_MS(35)
      {
        VERIFY_SCOPE_MAX_CPUTIME_MS(5)
        {
          usleep(40000*crpcut::timeout_multiplier());
        }
      }
      INFO << "after";
    }

    TEST(should_fail_verify_cputime_long, WITH_TEST_TAG(slow))
    {
      const clock_t clocks_per_tick = sysconf(_SC_CLK_TCK);
      tms t;
      times(&t);
      const size_t factor = crpcut::timeout_multiplier();
      clock_t deadline = (t.tms_utime + t.tms_stime + clocks_per_tick) * factor;
      VERIFY_SCOPE_MAX_CPUTIME_MS(900)
      {
        VERIFY_SCOPE_MIN_REALTIME_MS(1000)
        {
          for (;;)
            {
              for (volatile int n = 0; n < 100000; ++n)
                ;
              times(&t);
              if (t.tms_utime + t.tms_stime > deadline) break;
            }
        }
      }
      INFO << "after";
    }

  }

}
