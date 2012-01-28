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


#include <gmock/gmock.h>
#include <crpcut.hpp>


struct iface1 {
  virtual ~iface1() {}
  virtual void f1(int) = 0;
};

struct iface2 {
  virtual ~iface2() {}
  virtual void f1(const char *) = 0;
};

struct mock1 : iface1 {
  MOCK_METHOD1(f1, void(int));
};

struct mock2 : iface2 {
  MOCK_METHOD1(f1, void(const char*));
};

struct fix_base {
  mock1 m1;
  mock2 m2;
};

class fix_seq1 : public fix_base {
public:
  fix_seq1() {
    EXPECT_CALL(m1, f1(3)).InSequence(s);
    EXPECT_CALL(m2, f1("hello")).InSequence(s);
  }
private:
  testing::Sequence s; // prevent test cases from changing the sequence
};

class fix_seq2 : public fix_base {
public:
  fix_seq2() {
    EXPECT_CALL(m1, f1(3)).InSequence(s1, s2);
    EXPECT_CALL(m1, f1(4)).InSequence(s1);
  }
protected:
  testing::Sequence s1; // open-ended, for test-cases
  testing::Sequence s2; // to add more to the sequences
};


template <int N>
class to_test
{
public:
  to_test(iface1 &o1) : obj(0) { o1.f1(N); }
  ~to_test() { if (obj) { obj->f1("goodbye)"); } }
  void greet(iface2 &o) { obj = &o; o.f1("hello"); }
  void no_goodbye() { obj = 0; }
private:
  iface2 *obj;
  };

TEST(simple_3_hello, fix_seq1)
{
  to_test<3> object(m1);
  object.greet(m2);
  object.no_goodbye();
}

TEST(t2, fix_seq2)
{
  // add more to the started sequence
  EXPECT_CALL(m2, f1("hello")).InSequence(s2);
  to_test<3> object(m1);
  object.greet(m2);
}

TEST(t3, fix_seq2)
{
  // add other things to the started sequence
  EXPECT_CALL(m2, f1("goodbye")).InSequence(s2);
  to_test<3> object(m1);
}


int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
