/*
 * Copyright 2009 Bjorn Fahller <bjorn@fahller.se>
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

#include "symtable.hpp"
#include <crpcut.hpp>

DEFINE_TEST_TAG(insert);
DEFINE_TEST_TAG(lookup);

TESTSUITE(construct_and_destroy)
{
  TEST(construct)
  {
    (void)new symtable; // intentional leak
  }

  TEST(destroy, DEPENDS_ON(construct))
  {
    symtable s;
  }
}

TESTSUITE(normal_access, DEPENDS_ON(ALL_TESTS(construct_and_destroy)))
{
  TEST(insert_one, WITH_TEST_TAG(insert))
  {
    symtable s;
    s.add("one", 1);
  }
  TEST(insert_several, WITH_TEST_TAG(insert),
                       DEPENDS_ON(insert_one))
  {
    symtable s;
    s.add("one", 1);
    s.add("two", 2);
    s.add("three", 3);
  }
  TEST(lookup, WITH_TEST_TAG(lookup),
               DEPENDS_ON(insert_several))
  {
    symtable s;
    s.add("one", 1);
    s.add("two", 2);
    int v = s.lookup("one");
    ASSERT_EQ(v, 1);
    v = s.lookup("two");
    ASSERT_EQ(v, 2);
  }

}

TESTSUITE(abnormal, DEPENDS_ON(ALL_TESTS(normal_access)))
{
  TEST(lookup_nonexisting, WITH_TEST_TAG(lookup),
                           EXPECT_EXCEPTION(std::out_of_range))
   {
     symtable s;
     s.add("one", 1);
     s.lookup("two");
   }

  TEST(add_null, WITH_TEST_TAG(insert),
                 EXPECT_SIGNAL_DEATH(SIGABRT),
                 NO_CORE_FILE)
  {
    symtable s;
    s.add(0, 1);
  }

  TEST(lookup_null, WITH_TEST_TAG(lookup),
                    EXPECT_SIGNAL_DEATH(SIGABRT),
                    NO_CORE_FILE)
  {
    symtable s;
    s.add("one", 1);
    s.lookup(0);
  }

  TEST(overwrite_throws, WITH_TEST_TAG(insert))
  {
    symtable s;
    s.add("one", 1);
    ASSERT_THROW(s.add("one", 1), std::overflow_error);
  }
}

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
