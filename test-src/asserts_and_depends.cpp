/*
 * Copyright 2009-2012 Bjorn Fahller <bjorn@fahller.se>
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


#include <string>
#include <fstream>
#include <ostream>
extern "C"
{
#  include <unistd.h>
}
#include <crpcut.hpp>
template <int N>
struct fixture
{
  fixture() : num(N) {}
  int num ;
};

DEFINE_TEST_TAG(slow);
DEFINE_TEST_TAG(filesystem);
DEFINE_TEST_TAG(blocked);
DEFINE_TEST_TAG(exception_content_match);
TEST(default_success)
{
}

TEST(very_slow_success, WITH_TEST_TAG(slow))
{
  sleep(1);
}

TEST(should_fail_after_delay, WITH_TEST_TAG(slow))
{
  sleep(1);
  exit(1);
}




TESTSUITE(asserts)
{
#ifndef CRPCUT_NO_EXCEPTION_SUPPORT
  TEST(should_succeed_assert_throw_with_correct_exception)
  {
    ASSERT_THROW(throw std::bad_alloc(), std::bad_alloc);
  }

  TEST(should_fail_assert_exception_with_wrong_exception)
  {
    ASSERT_THROW(throw std::bad_alloc(), std::domain_error);
  }


  TEST(should_fail_assert_throw_with_no_exception)
  {
    int i;
    ASSERT_THROW(i=1, std::exception);
    std::cout << "i=" << i; // won't run
  }

  TEST(should_succeed_assert_no_throw)
  {
    int i;
    ASSERT_NO_THROW(i=1);
    std::cout << "i=" << i;
  }

  TEST(should_fail_assert_throw_any_with_no_exception)
  {
    int i;
    ASSERT_THROW(i=1, ...);
    std::cout << "i=" << i;
  }
  TEST(should_succeed_throw_any_with_int_exception)
  {
    ASSERT_THROW(throw 1, ...);
  }

  TEST(should_fail_assert_no_throw_with_unknown_exception)
  {
    ASSERT_NO_THROW(throw 1);
  }

  TEST(should_fail_assert_no_throw_with_std_exception_string_apa)
  {
    ASSERT_NO_THROW(throw std::range_error("apa"));
  }

  TEST(should_succeed_assert_throw_with_exact_string_match,
       WITH_TEST_TAG(exception_content_match))
  {
    ASSERT_THROW(throw std::range_error("apa"), std::exception, "apa");
  }

  TEST(should_fail_assert_throw_with_mismatching_string,
       WITH_TEST_TAG(exception_content_match))
  {
    ASSERT_THROW(throw std::range_error("apa"), std::exception, "katt");
  }

  TEST(should_succeed_assert_throw_with_regexp_match,
       WITH_TEST_TAG(exception_content_match))
  {
    ASSERT_THROW(throw std::range_error("en liten apa"),
                 std::exception,
                 crpcut::match<crpcut::regex>("liten"));
  }

  TEST(should_fail_assert_throw_with_mismatching_regexp,
       WITH_TEST_TAG(exception_content_match))
  {
    ASSERT_THROW(throw std::range_error("en liten apa"),
                 std::exception,
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

  TEST(should_succeed_assert_throw_with_custom_matcher,
       WITH_TEST_TAG(exception_content_match))
  {
    ASSERT_THROW(throw 5, int, int_matcher(5));
  }

  TEST(should_fail_assert_throw_with_custom_matcher,
       WITH_TEST_TAG(exception_content_match))
  {
    ASSERT_THROW(throw 5, int, int_matcher(3));
  }
  #endif
  TEST(should_succeed_on_assert_eq_with_fixture, fixture<3>)
  {
    ASSERT_EQ(num, 3);
  }

  TEST(should_fail_on_assert_eq_with_fixture, fixture<4>)
  {
    ASSERT_EQ(num, 3);
  }

  TEST(should_succeed_on_assert_ne_with_fixture, fixture<4>)
  {
    ASSERT_NE(num, 3);
  }

  TEST(should_fail_on_assert_ne_with_fixture, fixture<3>)
  {
    ASSERT_NE(num, 3);
  }

  TEST(should_succeed_on_assert_gt_with_fixture, fixture<4>)
  {
    ASSERT_GT(num, 3);
  }

  TEST(should_fail_on_assert_gt_with_fixture, fixture<3>)
  {
    ASSERT_GT(num, 3);
  }

  TEST(should_succeed_on_assert_ge_with_fixture, fixture<3>)
  {
    ASSERT_GE(num, 3);
  }

  TEST(should_fail_on_assert_ge_with_fixture, fixture<2>)
  {
    ASSERT_GE(num, 3);
  }

  TEST(should_succeed_on_assert_lt_with_fixture, fixture<2>)
  {
    ASSERT_LT(num, 3);
  }

  TEST(should_fail_on_assert_lt_with_fixture, fixture<3>)
  {
    ASSERT_LT(num, 3);
  }

  TEST(should_succeed_on_assert_le_with_fixture, fixture<3>)
  {
    ASSERT_LE(num, 3);
  }

  TEST(should_fail_on_assert_le_with_fixture, fixture<4>)
  {
    ASSERT_LE(num, 3);
  }

  TEST(should_succeed_on_assert_true_with_fixture, fixture<3>)
  {
    ASSERT_TRUE(num);
  }

  TEST(should_fail_on_assert_true_with_fixture, fixture<0>)
  {
    ASSERT_TRUE(num);
  }

  TEST(should_succeed_on_assert_false_with_fixture, fixture<0>)
  {
    ASSERT_FALSE(num);
  }

  TEST(should_fail_on_assert_false_with_fixture, fixture<3>)
  {
    ASSERT_FALSE(num);
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

  TEST(should_fail_on_assert_gt_with_unstreamable_param_i, fixture<3>)
  {
    unstreamable<int> i(3);
    ASSERT_GT(i, num);
  }

  TEST(should_fail_on_assert_true_with_small_unstreamable_param)
  {
    unstreamable<int> i(0);
    ASSERT_TRUE(i);
  }
  TEST(should_fail_on_assert_true_with_large_unstreamable_param)
  {
    unstreamable<long double> i(0);
    ASSERT_TRUE(i);
  }
  TEST(should_succeed_pointer_eq_0)
  {
    int *pi = 0;
    ASSERT_EQ(pi, 0);
  }
  TEST(should_succeed_0_eq_pointer)
  {
    int *pi = 0;
    ASSERT_EQ(0, pi);
  }

  TEST(should_succeed_void_ptr_eq_ptr)
  {
    int i;
    int *pi = &i;
    void *pv = &i;
    ASSERT_EQ(pv, pi);
  }

  TEST(should_succeed_ptr_eq_void_ptr)
  {
    int i;
    int *pi = &i;
    void *pv = &i;
    ASSERT_EQ(pi, pv);
  }

  TEST(should_fail_pointer_eq_0)
  {
    int i;
    int *pi = &i;
    ASSERT_EQ(pi, 0);
  }
  TEST(should_fail_0_eq_pointer)
  {
    int i;
    int *pi = &i;
    ASSERT_EQ(0, pi);
  }

  TEST(should_fail_void_ptr_eq_ptr)
  {
    int i;
    int *pi = &i;
    void *pv = 0;
    ASSERT_EQ(pv, pi);
  }

  TEST(should_fail_ptr_eq_void_ptr)
  {
    int i;
    int *pi = &i;
    void *pv = 0;
    ASSERT_EQ(pi, pv);
  }

  TEST(should_fail_eq_volatile)
  {
    volatile int n = 3;
    int ref = 4;
    const volatile int &m = ref;
    ASSERT_EQ(n, m);
  }

  TEST(should_fail_false_volatile)
  {
    volatile int n = 3;
    ASSERT_FALSE(n);
  }

  TEST(should_fail_false_const_volatile)
  {
    const volatile int n = 3;
    ASSERT_FALSE(n);
  }

  TEST(should_fail_true_volatile)
  {
    volatile int n = 0;
    ASSERT_TRUE(n);
  }

  TEST(should_fail_true_const_volatile)
  {
    const volatile int n = 0;
    ASSERT_TRUE(n);
  }
  struct local
  {
    static const int apa = 1;
  };
  TEST(should_succeed_class_const_int_member)
  {
    ASSERT_EQ(local::apa, 1);
  }

  TEST(should_succeed_0_eq_pointer_to_member)
  {
    int local::*p = 0;
    ASSERT_EQ(0, p);
  }

  TESTSUITE(expr)
  {
    TEST(should_fail_on_assert_true_with_small_unstreamable_param)
    {
      unstreamable<int> i(4);
      ASSERT_TRUE(i - 4 < unstreamable<int>(0));
    }
    TEST(should_fail_on_assert_true_with_large_unstreamable_param)
    {
      unstreamable<long double> i(4);
      ASSERT_TRUE(i - 4 < unstreamable<long double>(0));
    }
    TEST(should_fail_on_assert_true_with_lt, fixture<3>)
    {
      int n = 4;
      ASSERT_TRUE(n < num);
    }
    TEST(should_fail_on_assert_true_with_add_lt, fixture<3>)
    {
      int n = 4;
      ASSERT_TRUE(n + num < 5);
    }
    TEST(should_fail_on_assert_true_with_sub_lt, fixture<3>)
    {
      int n = 4;
      ASSERT_TRUE(n - num < 0);
    }
  }

}

DISABLED_TEST(should_never_run, fixture<3>)
{
  ASSERT_FALSE(num);
}


TESTSUITE(depends)
{
  TEST(should_succeed_after_success_dependencies,
       DEPENDS_ON(default_success,
                  asserts::should_succeed_on_assert_eq_with_fixture,
                  very_slow_success,
                  should_never_run))
  {
  }

  TEST(should_not_run_due_to_one_failed_dependency_success_otherwise,
       DEPENDS_ON(default_success,
                  asserts::should_succeed_on_assert_eq_with_fixture,
                  should_fail_after_delay),
       WITH_TEST_TAG(blocked))
  {
  }
}



TEST(should_fail_due_to_left_behind_files, WITH_TEST_TAG(filesystem))
{
  std::ofstream of("apa");
  of << "katt";
}

TEST(should_succeed_reading_file_in_start_dir, WITH_TEST_TAG(filesystem))
{
  std::string s(crpcut::get_start_dir());
  s+= "/apafil";
  std::ifstream in(s.c_str());
  std::string content;
  in >> content;
  INFO << "in.rdstate()=" <<  in.rdstate();
  ASSERT_EQ(content, "apa");
}

TEST(should_not_run_due_to_failed_left_behind_files_success_otherwise,
     DEPENDS_ON(should_fail_due_to_left_behind_files),
     WITH_TEST_TAG(blocked))
{
}


