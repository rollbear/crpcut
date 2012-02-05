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
#include "../deadline_monitor.hpp"
#include "../clocks/clocks.hpp"

TESTSUITE(deadline_monitor)
{
  class test_clock : public crpcut::clocks::monotonic
  {
  public:
    test_clock()  {}
    virtual unsigned long now() const { return 50UL; }
  private:
  };

  class timeboxed : public crpcut::crpcut_timeboxed
  {
  public:
    timeboxed(unsigned long ms) { crpcut_set_deadline(ms); }
    void crpcut_kill() {}
  };

  struct fix
  {
    fix() : t1(101),   t2(102),  t3(103),  t4(104),
            t5(105),   t6(106),  t7(107),  t8(108),
            t9(109),  t10(110), t11(111), t12(112),
            t13(113), t14(114), t15(115), t16(116)
    {
    }
    timeboxed t1;
    timeboxed t2;
    timeboxed t3;
    timeboxed t4;
    timeboxed t5;
    timeboxed t6;
    timeboxed t7;
    timeboxed t8;
    timeboxed t9;
    timeboxed t10;
    timeboxed t11;
    timeboxed t12;
    timeboxed t13;
    timeboxed t14;
    timeboxed t15;
    timeboxed t16;
    test_clock clock;
  };
  TEST(remove_first_gives_shortest_deadline, fix)
  {

    void *buff = alloca(crpcut::deadline_monitor::space_for(16));
    crpcut::deadline_monitor m(buff, 16);
    m.insert(&t6);
    m.insert(&t12);
    m.insert(&t1);
    m.insert(&t4);
    m.insert(&t8);
    m.insert(&t14);
    m.insert(&t10);
    m.insert(&t3);
    m.insert(&t13);
    m.insert(&t5);
    m.insert(&t7);
    m.insert(&t2);
    m.insert(&t9);
    m.insert(&t16);
    m.insert(&t11);
    m.insert(&t15);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 51);
    ASSERT_TRUE(m.remove_first() == &t1);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 52);
    ASSERT_TRUE(m.remove_first() == &t2);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 53);
    ASSERT_TRUE(m.remove_first() == &t3);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 54);
    ASSERT_TRUE(m.remove_first() == &t4);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 55);
    ASSERT_TRUE(m.remove_first() == &t5);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 56);
    ASSERT_TRUE(m.remove_first() == &t6);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 57);
    ASSERT_TRUE(m.remove_first() == &t7);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 58);
    ASSERT_TRUE(m.remove_first() == &t8);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 59);
    ASSERT_TRUE(m.remove_first() == &t9);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 60);
    ASSERT_TRUE(m.remove_first() == &t10);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 61);
    ASSERT_TRUE(m.remove_first() == &t11);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 62);
    ASSERT_TRUE(m.remove_first() == &t12);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 63);
    ASSERT_TRUE(m.remove_first() == &t13);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 64);
    ASSERT_TRUE(m.remove_first() == &t14);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 65);
    ASSERT_TRUE(m.remove_first() == &t15);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 66);
    ASSERT_TRUE(m.remove_first() == &t16);

    ASSERT_TRUE(m.ms_until_deadline(clock) == -1);

  }

  TEST(remove_maintains_sort_order,
       DEPENDS_ON(remove_first_gives_shortest_deadline),
       fix)
  {


    void *buff = alloca(crpcut::deadline_monitor::space_for(16));
    crpcut::deadline_monitor m(buff, 16);
    m.insert(&t16);
    m.insert(&t15);
    m.insert(&t14);
    m.insert(&t13);
    m.insert(&t12);
    m.insert(&t11);
    m.insert(&t10);
    m.insert(&t9);
    m.insert(&t8);
    m.insert(&t7);
    m.insert(&t4);
    m.insert(&t6);
    m.insert(&t5);
    m.insert(&t3);
    m.insert(&t2);
    m.insert(&t1);

    m.remove(&t2);
    m.remove(&t15);
    m.remove(&t11);
    m.remove(&t12);
    m.remove(&t13);
    m.remove(&t3);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 51);
    ASSERT_TRUE(m.remove_first() == &t1);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 54);
    ASSERT_TRUE(m.remove_first() == &t4);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 55);
    ASSERT_TRUE(m.remove_first() == &t5);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 56);
    ASSERT_TRUE(m.remove_first() == &t6);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 57);
    ASSERT_TRUE(m.remove_first() == &t7);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 58);
    ASSERT_TRUE(m.remove_first() == &t8);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 59);
    ASSERT_TRUE(m.remove_first() == &t9);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 60);
    ASSERT_TRUE(m.remove_first() == &t10);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 64);
    ASSERT_TRUE(m.remove_first() == &t14);

    ASSERT_TRUE(m.ms_until_deadline(clock) == 66);
    ASSERT_TRUE(m.remove_first() == &t16);

    ASSERT_TRUE(m.ms_until_deadline() == -1);
  }
}






