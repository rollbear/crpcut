/*
 * Copyright 2011-2012 Bjorn Fahller <bjorn@fahller.se>
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


#include <string>
#include <fstream>
#include <ostream>
#include <crpcut.hpp>

DEFINE_TEST_TAG(exception_content_match);

template <int N>
struct fixture
{
  fixture() : num(N) {}
  int num ;
};


TESTSUITE(verify)
{
#ifndef CRPCUT_NO_EXCEPTION_SUPPORT
  TEST(should_succeed_verify_throw_with_correct_exception)
  {
    VERIFY_THROW(throw std::bad_alloc(), std::bad_alloc&);
    INFO << "after";
  }

  TEST(should_fail_verify_exception_with_wrong_exception)
  {
    VERIFY_THROW(throw std::bad_alloc(), std::domain_error&);
    INFO << "after";
  }


  TEST(should_fail_verify_throw_with_no_exception)
  {
    int i;
    VERIFY_THROW(i=1, std::exception&);
    INFO << "after i=" << i;
  }

  TEST(should_fail_verify_throw_with_unexpected_c_string)
  {
    VERIFY_THROW(throw "apa", std::exception&);
    INFO << "after";
  }

  TEST(should_fail_verify_throw_with_translated_invalid_argument)
  {
    VERIFY_THROW(throw std::invalid_argument("apa"), std::domain_error&);
    INFO << "after";
  }
  TEST(should_succeed_verify_no_throw)
  {
    int i;
    VERIFY_NO_THROW(i=1);
    INFO << "after i=" << i;
  }

  TEST(should_fail_verify_throw_any_with_no_exception)
  {
    int i;
    VERIFY_THROW(i=1, ...);
    INFO << "after i=" << i;
  }

  TEST(should_succeed_throw_any_with_int_exception)
  {
    VERIFY_THROW(throw 1, ...);
    INFO << "after";
  }

  TEST(should_fail_verify_no_throw_with_unknown_exception)
  {
    VERIFY_NO_THROW(throw 1);
    INFO << "after";
  }

  TEST(should_fail_verify_no_throw_with_std_exception_string_apa)
  {
    VERIFY_NO_THROW(throw std::range_error("apa"));
    INFO << "after";
  }

  TEST(should_fail_verify_no_throw_with_unexpected_c_string)
  {
    VERIFY_NO_THROW(throw "apa");
    INFO << "after";
  }

  TEST(should_fail_verify_no_throw_with_translated_exception)
  {
    VERIFY_NO_THROW(throw std::invalid_argument("apa"));
    INFO << "after";
  }

  TEST(should_succeed_verify_throw_with_exact_string_match,
       WITH_TEST_TAG(exception_content_match))
  {
    VERIFY_THROW(throw std::range_error("apa"), std::exception&, "apa");
  }

  TEST(should_fail_verify_throw_with_mismatching_string,
       WITH_TEST_TAG(exception_content_match))
  {
    VERIFY_THROW(throw std::range_error("apa"), std::exception&, "katt");
  }

  TEST(should_succeed_verify_throw_with_regexp_match,
       WITH_TEST_TAG(exception_content_match))
  {
    VERIFY_THROW(throw std::range_error("en liten apa"),
                 std::exception&,
                 crpcut::match<crpcut::regex>("liten"));
  }

  TEST(should_fail_verify_throw_with_mismatching_regexp,
       WITH_TEST_TAG(exception_content_match))
  {
    VERIFY_THROW(throw std::range_error("en liten apa"),
                 std::exception&,
                 crpcut::match<crpcut::regex>(".*x.*"));
  }
  class int_matcher
  {
  public:
    int_matcher(int i) : i_(i), mem_(0) {}
    bool operator()(int n) { mem_ = n; return n == i_; }
    friend std::ostream& operator<<(std::ostream &os, const int_matcher& m)
    {
      return os << m.mem_ << " does not match the expected " << m.i_;
    }
  private:
    int i_;
    int mem_;
  };

  TEST(should_succeed_verify_throw_with_custom_matcher,
       WITH_TEST_TAG(exception_content_match))
  {
    VERIFY_THROW(throw 5, int, int_matcher(5));
  }

  TEST(should_fail_verify_throw_with_custom_matcher,
       WITH_TEST_TAG(exception_content_match))
  {
    VERIFY_THROW(throw 5, int, int_matcher(3));
  }

#endif
  TEST(should_succeed_on_verify_eq_with_fixture, fixture<3>)
  {
    VERIFY_EQ(num, 3);
    INFO << "after";
  }

  TEST(should_fail_on_verify_eq_with_fixture, fixture<4>)
  {
    VERIFY_EQ(num, 3);
    INFO << "after";
  }

  TEST(should_succeed_on_verify_ne_with_fixture, fixture<4>)
  {
    VERIFY_NE(num, 3);
    INFO << "after";
  }

  TEST(should_fail_on_verify_ne_with_fixture, fixture<3>)
  {
    VERIFY_NE(num, 3);
    INFO << "after";
  }

  TEST(should_succeed_on_verify_gt_with_fixture, fixture<4>)
  {
    VERIFY_GT(num, 3);
    INFO << "after";
  }

  TEST(should_fail_on_verify_gt_with_fixture, fixture<3>)
  {
    VERIFY_GT(num, 3);
    INFO << "after";
  }

  TEST(should_succeed_on_verify_ge_with_fixture, fixture<3>)
  {
    VERIFY_GE(num, 3);
    INFO << "after";
  }

  TEST(should_fail_on_verify_ge_with_fixture, fixture<2>)
  {
    VERIFY_GE(num, 3);
    INFO << "after";
  }

  TEST(should_succeed_on_verify_lt_with_fixture, fixture<2>)
  {
    VERIFY_LT(num, 3);
    INFO << "after";
  }

  TEST(should_fail_on_verify_lt_with_fixture, fixture<3>)
  {
    VERIFY_LT(num, 3);
    INFO << "after";
  }

  TEST(should_succeed_on_verify_le_with_fixture, fixture<3>)
  {
    VERIFY_LE(num, 3);
    INFO << "after";
  }

  TEST(should_fail_on_verify_le_with_fixture, fixture<4>)
  {
    VERIFY_LE(num, 3);
    INFO << "after";
  }

  TEST(should_succeed_on_verify_true_with_fixture, fixture<3>)
  {
    VERIFY_TRUE(num);
    INFO << "after";
  }

  TEST(should_fail_on_verify_true_with_fixture, fixture<0>)
  {
    VERIFY_TRUE(num);
    INFO << "after";
  }

  TEST(should_succeed_on_verify_false_with_fixture, fixture<0>)
  {
    VERIFY_FALSE(num);
    INFO << "after";
  }

  TEST(should_fail_on_verify_false_with_fixture, fixture<3>)
  {
    VERIFY_FALSE(num);
    INFO << "after";
  }

  template <typename T>
    class unstreamable
  {
  public:
    unstreamable(T t) : data(t) {}
    operator T&() { return data; }
    operator const T&() const { return data; }
  private:
    T data;
  };

  template <typename T>
  std::ostream& operator<<(std::ostream&, const unstreamable<T>&) = delete;

  TEST(should_fail_on_verify_gt_with_unstreamable_param_i, fixture<3>)
  {
    unstreamable<int> i(3);
    VERIFY_GT(i, num);
    INFO << "after";
  }

  TEST(should_fail_on_verify_true_with_small_unstreamable_param)
  {
    unstreamable<int> i(0);
    VERIFY_TRUE(i);
    INFO << "after";
  }
  TEST(should_fail_on_verify_true_with_large_unstreamable_param)
  {
    unstreamable<long double> i(0);
    VERIFY_TRUE(i);
    INFO << "after";
  }
  TEST(should_succeed_pointer_eq_0)
  {
    int *pi = nullptr;
    VERIFY_EQ(pi, 0);
    INFO << "after";
  }
  TEST(should_succeed_0_eq_pointer)
  {
    int *pi = nullptr;
    VERIFY_EQ(0, pi);
    INFO << "after";
  }

  TEST(should_succeed_void_ptr_eq_ptr)
  {
    int i;
    int *pi = &i;
    void *pv = &i;
    VERIFY_EQ(pv, pi);
    INFO << "after";
  }

  TEST(should_succeed_ptr_eq_void_ptr)
  {
    int i;
    int *pi = &i;
    void *pv = &i;
    VERIFY_EQ(pi, pv);
    INFO << "after";
  }

  TEST(should_fail_pointer_eq_0)
  {
    int i;
    int *pi = &i;
    VERIFY_EQ(pi, 0);
    INFO << "after";
  }
  TEST(should_fail_0_eq_pointer)
  {
    int i;
    int *pi = &i;
    VERIFY_EQ(0, pi);
    INFO << "after";
  }

  TEST(should_fail_void_ptr_eq_ptr)
  {
    int i;
    int *pi = &i;
    void *pv = nullptr;
    VERIFY_EQ(pv, pi);
    INFO << "after";
  }

  TEST(should_fail_ptr_eq_void_ptr)
  {
    int i;
    int *pi = &i;
    void *pv = nullptr;
    VERIFY_EQ(pi, pv);
    INFO << "after";
  }

  TEST(should_fail_eq_volatile)
  {
    volatile int n = 3;
    int ref = 4;
    const volatile int &m = ref;
    VERIFY_EQ(n, m);
    INFO << "after";
  }

  TEST(should_fail_false_volatile)
  {
    volatile int n = 3;
    VERIFY_FALSE(n);
    INFO << "after";
  }

  TEST(should_fail_false_const_volatile)
  {
    const volatile int n = 3;
    VERIFY_FALSE(n);
    INFO << "after";
  }

  TEST(should_fail_true_volatile)
  {
    volatile int n = 0;
    VERIFY_TRUE(n);
    INFO << "after";
  }

  TEST(should_fail_true_const_volatile)
  {
    const volatile int n = 0;
    VERIFY_TRUE(n);
    INFO << "after";
  }
  struct local
  {
    static const int apa = 1;
  };
  TEST(should_succeed_class_const_int_member)
  {
    VERIFY_EQ(local::apa, 1);
    INFO << "after";
  }

  TEST(should_succeed_0_eq_pointer_to_member)
  {
    int local::*p = nullptr;
    VERIFY_EQ(0, p);
    INFO << "after";
  }

  TESTSUITE(expr)
  {
    TEST(should_fail_on_verify_true_with_small_unstreamable_param)
    {
      unstreamable<int> i(4);
      VERIFY_TRUE(i - 4 < unstreamable<int>(0));
      INFO << "after";
    }
    TEST(should_fail_on_verify_true_with_large_unstreamable_param)
    {
      unstreamable<long double> i(4);
      VERIFY_TRUE(i - 4 < unstreamable<long double>(0));
      INFO << "after";
    }
    TEST(should_fail_on_verify_true_with_lt, fixture<3>)
    {
      int n = 4;
      VERIFY_TRUE(n < num);
      INFO << "after";
    }
    TEST(should_fail_on_verify_true_with_add_lt, fixture<3>)
    {
      int n = 4;
      VERIFY_TRUE(n + num < 5);
      INFO << "after";
    }
    TEST(should_fail_on_verify_true_with_sub_lt, fixture<3>)
    {
      int n = 4;
      VERIFY_TRUE(n - num < 0);
      INFO << "after";
    }
  }

}
