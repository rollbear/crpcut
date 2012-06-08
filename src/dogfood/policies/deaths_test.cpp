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

TESTSUITE(policies)
{
  TESTSUITE(deaths)
  {
    TESTSUITE(crpcut_none)
    {
      TEST(deadline_is_suggested_plus_one_sec)
      {
        const crpcut::policies::deaths::crpcut_none obj;
        for (unsigned long i = 10; i < 2010; i+=100)
          {
            ASSERT_TRUE(obj.crpcut_calc_deadline(i) == i + 1000);
          }
      }

      TEST(is_expected_exit_is_always_false)
      {
        const crpcut::policies::deaths::crpcut_none obj;
        for (int i = 0; i < 128; ++i)
          {
            ASSERT_FALSE(obj.crpcut_is_expected_exit(i));
          }
      }

      TEST(is_expected_signal_is_always_false)
      {
        const crpcut::policies::deaths::crpcut_none obj;
        for (int i = 0; i < 128; ++i)
          {
            ASSERT_FALSE(obj.crpcut_is_expected_signal(i));
          }
      }

      TEST(expected_death_is_normal_exit)
      {
        std::ostringstream os;
        const crpcut::policies::deaths::crpcut_none obj;
        obj.crpcut_expected_death(os);
        ASSERT_TRUE(os.str() == "normal exit");
      }
    }
  }
}







