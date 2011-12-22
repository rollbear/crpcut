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

DEFINE_TEST_TAG(blocked)

TESTSUITE(suite_deps)
{
  TESTSUITE(simple_all_ok)
  {
    TEST(should_succeed)
    {
    }
    TEST(should_also_succeed)
    {
    }
  }
  TESTSUITE(simple_all_fail)
  {
    TEST(should_succeed)
    {
    }
    TEST(should_fail)
    {
      ASSERT_TRUE(false);
    }
  }
  TEST(should_succeed, DEPENDS_ON(ALL_TESTS(simple_all_ok)))
    {
    }

  TEST(should_not_run_success,
       DEPENDS_ON(ALL_TESTS(simple_all_fail)),
       WITH_TEST_TAG(blocked))
    {
    }

  TESTSUITE(should_run_suite, DEPENDS_ON(ALL_TESTS(simple_all_ok)))
  {
    TEST(should_succeed)
    {
    }
  }
  TESTSUITE(blocked_suite, DEPENDS_ON(ALL_TESTS(simple_all_fail)))
  {
    TEST(should_not_run_success, WITH_TEST_TAG(blocked))
    {
    }
  }
  TESTSUITE(blocked_case, DEPENDS_ON(simple_all_fail::should_fail))
  {
    TEST(should_not_run_success, WITH_TEST_TAG(blocked))
    {
    }
    TESTSUITE(nested_blocked)
    {
      TEST(should_not_run_success, WITH_TEST_TAG(blocked))
      {
      }
    }
  }
  TESTSUITE(should_run_case, DEPENDS_ON(simple_all_ok::should_succeed))
  {
    TEST(should_succeed)
    {
    }
    TESTSUITE(nested_run)
    {
      TEST(should_succeed)
      {
      }
    }
  }
}

