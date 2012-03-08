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

DEFINE_TEST_TAG(a_tag);
DEFINE_TEST_TAG(another_tag);

struct A
{
  A() {}
  ~A() { abort(); }
  int func() const { return 3; }
  void do_something() const {}
};

TESTSUITE(basics)
{
  DISABLED_TEST(will_not_run)
  {
    FAIL << "shall never ever run!";
  }

  TEST(construct, WITH_TEST_TAG(a_tag))
  {
    A *p = new A(); // leak
    INFO << "created an A, addr=" << p;
  }

  TEST(call_func, A, DEPENDS_ON(construct), WITH_TEST_TAG(a_tag))
  {
    ASSERT_TRUE(func() == 1);
  }

  TEST(destroy, A, DEPENDS_ON(construct), WITH_TEST_TAG(another_tag))
  {
    INFO << "running test body";
  }
}

TESTSUITE(toy, DEPENDS_ON(ALL_TESTS(basics)))
{
  TEST(work)
  {
    A obj;
    obj.do_something();
  }
}

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
