/*
 * Copyright 2009 Bjorn Fahller <bjorn@fahller.se>
 * All rights reserved

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
extern "C"
{
#include <signal.h>
}

DEFINE_TEST_TAG(filesystem)

TESTSUITE(output)
{
  TEST(should_succeed_with_stdout)
  {
    std::cout << "hello";
  }

  TEST(should_succeed_with_stderr)
  {
    std::cerr << "hello";
  }

  TEST(should_fail_with_death_due_to_assert_on_stderr, NO_CORE_FILE)
  {
    assert("apa" == 0);
  }

  TEST(should_fail_with_death_and_left_behind_core_dump,
       EXPECT_SIGNAL_DEATH(SIGABRT),
       WITH_TEST_TAG(filesystem))
  {
    assert("apa" == 0);
  }

  TEST(should_succeed_with_info)
  {
    const char msg[] = "apa";
    INFO << msg << "=" << std::hex << 3;
  }

  TEST(should_fail_with_info)
  {
    const char msg[] = "apa";
    INFO << msg << "=" << 31;
    exit(1);
  }

  TEST(should_fail_with_terminate)
  {
    const char msg[] = "apa";
    FAIL << msg << "=" << std::hex << 31;
  }

  TEST(should_succeed_with_info_endl)
  {
    INFO << "apa" << std::endl << "katt";
  }

  struct S {
    double d[2];
    float  f;
  };
  TEST(should_succeed_with_big_unstreamable_obj)
  {
    S s = { { 1.3, 3.14 }, 1.1f };
    INFO << s;
  }

  TEST(string_with_illegal_chars_should_succeed)
  {
    std::string s;
    for (int i = 0; i < 256; ++i)
      {
        s+=(char(i));
      }
    INFO << s;
  }
}
