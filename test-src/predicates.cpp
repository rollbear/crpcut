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
#include <cstring>

class ptr_deref_eq
{
public:
  template <typename T>
  class type
  {
  public:
    type(T* refp) : rp(refp), cp(0) {}
    type(const type<T>& t) : rp(t.rp), cp(t.cp) {}
    bool operator()(T* compp)
    {
      cp = compp; // store values for use by output stream operator
      return *rp == *cp;
    }
    friend std::ostream& operator<<(std::ostream &os, const type<T>& t)
    {
      os << "reference ptr=" << t.rp << " pointing to: " << *t.rp
         << "\ncompared ptr=" << t.cp << " pointing to: " << *t.cp;
      return os;
    }
  private:
    T* rp;
    T* cp;
  };
};

namespace crpcut {
  template <typename T>
  struct match_traits<ptr_deref_eq, T*>
  {
    using type = typename ptr_deref_eq::template type<T>;
  };
} // namespace crpcut

TESTSUITE(predicates)
{
  static bool is_positive(int n)
  {
    return n >= 0;
  }

  TEST(should_succeed_simple_func)
  {
    int v = 1;
    ASSERT_PRED(is_positive, v);
  }

  TEST(should_fail_simple_func)
  {
    int v = -1;
    ASSERT_PRED(is_positive, v);
  }

  TEST(should_succeed_simple_func_with_param_side_effect)
  {
    int v = 1;
    ASSERT_PRED(is_positive, v--);
    ASSERT_EQ(v, 0);
  }

  TEST(should_fail_simple_func_with_param_side_effect)
  {
    int v = 0;
    ASSERT_PRED(is_positive, --v);
  }

  TEST(should_succeed_verify_simple_func)
  {
    int v = 1;
    VERIFY_PRED(is_positive, v);
    INFO << "after";
  }

  TEST(should_fail_verify_simple_func)
  {
    int v = -1;
    VERIFY_PRED(is_positive, v);
    INFO << "after";
  }

  TEST(should_succeed_verify_simple_func_with_param_side_effect)
  {
    int v = 1;
    VERIFY_PRED(is_positive, v--);
    ASSERT_EQ(v, 0);
    INFO << "after";
  }

  TEST(should_fail_verify_simple_func_with_param_side_effect)
  {
    int v = 0;
    VERIFY_PRED(is_positive, --v);
    INFO << "after";
  }

  template <typename RC, typename RT, typename P1, typename P2>
  class bifuncwrap_t
  {
  public:
    bifuncwrap_t(RT (*func)(P1, P2), RT refval) : f(func), rv(refval) {}
    template <typename PV1, typename PV2>
    bool operator()(PV1 p1, PV2 p2) { return RC()(f(p1, p2), rv); }
  private:
    RT (*f)(P1, P2);
    RT rv;
  };
  template <typename RC, typename RT, typename P1, typename P2>
  inline  bifuncwrap_t<RC, RT, P1, P2>
  bifuncwrap(RT (*f)(P1, P2), RT rv)
  {
    return bifuncwrap_t<RC, RT, P1, P2>(f, rv);
  }

  TEST(should_succeed_func_wrap_class)
  {
    ASSERT_PRED(bifuncwrap<std::less<int> >(std::strcmp, 0), "apa", "katt");
  }

  TEST(should_fail_func_wrap_class)
  {
    ASSERT_PRED(bifuncwrap<std::less<int> >(std::strcmp, 0), "katt", "apa");
  }

  TEST(should_succeed_verify_func_wrap_class)
  {
    VERIFY_PRED(bifuncwrap<std::less<int> >(std::strcmp, 0), "apa", "katt");
    INFO << "after";
  }

  TEST(should_fail_verify_func_wrap_class)
  {
    VERIFY_PRED(bifuncwrap<std::less<int> >(std::strcmp, 0), "katt", "apa");
    INFO << "after";
  }

  class string_equal
  {
  public:
    string_equal(const char *ref) : r(ref) {}
    bool operator()(const char *v) const { return std::strcmp(v, r) == 0; }
    friend std::ostream& operator<<(std::ostream &os, const string_equal &e)
    {
      return os << "compare string equal to \"" << e.r << "\"";
    }
  private:
    const char *r;
  };

  TEST(should_succeed_streamable_pred)
  {
    ASSERT_PRED(string_equal("apa"), "apa");
  }

  TEST(should_fail_streamable_pred)
  {
    ASSERT_PRED(string_equal("apa"), "katt");
  }

  TEST(should_succeed_ptr_deref_eq)
  {
    int n = 3;
    int m = 3;
    int *p = &m;
    ASSERT_PRED(crpcut::match<ptr_deref_eq>(&n), p);
  }

  TEST(should_fail_ptr_deref_eq)
  {
    int n = 4;
    int m = 3;
    int *p = &m;
    ASSERT_PRED(crpcut::match<ptr_deref_eq>(&n), p);
  }

  TEST(should_succeed_verify_streamable_pred)
  {
    VERIFY_PRED(string_equal("apa"), "apa");
    INFO << "after";
  }

  TEST(should_fail_verify_streamable_pred)
  {
    VERIFY_PRED(string_equal("apa"), "katt");
    INFO << "after";
  }

  TEST(should_succeed_verify_ptr_deref_eq)
  {
    int n = 3;
    int m = 3;
    int *p = &m;
    VERIFY_PRED(crpcut::match<ptr_deref_eq>(&n), p);
    INFO << "after";
  }

  TEST(should_fail_verify_ptr_deref_eq)
  {
    int n = 4;
    int m = 3;
    int *p = &m;
    VERIFY_PRED(crpcut::match<ptr_deref_eq>(&n), p);
    INFO << "after";
  }

  TESTSUITE(match_operator)
  {
    template <typename T, typename U>
    class in_range_t : public crpcut::predicate
    {
    public:
      in_range_t(T low, U high) : lower_bound(low), upper_bound(high) {}
      template <typename V>
      bool operator()(const V& v) const
      {
        return lower_bound <= v && v <= upper_bound;
      }
      friend std::ostream &operator<<(std::ostream &os, const in_range_t &r)
      {
        return
            os << "in_range(" << r.lower_bound << ", " << r.upper_bound << ")";
      }
    private:
      T lower_bound;
      U upper_bound;
    };

    template <typename T, typename U>
    in_range_t<T, U> in_range(T low, U high)
    {
      return in_range_t<T, U>(low, high);
    }

    TEST(should_succeed_int_range_check)
    {
      int n = 3;
      ASSERT_TRUE(n =~ in_range(3,8));
    }

    TEST(should_fail_float_range_check)
    {
      float f = 3.14F;
      ASSERT_TRUE(f =~ in_range(3.141592, 4));
    }
  }
}
