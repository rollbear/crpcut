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

namespace {
  template <typename T, typename V>
  void assert_expr(const T& t, const V& value)
  {
    ASSERT_TRUE(crpcut::eval(t) == value);

  }
  template <typename T, typename V>
  void assert_expr(const T& t, const V& value, const char *re)
  {
    assert_expr(t, value);
    std::ostringstream os;
    crpcut::show_value(os, t);
    ASSERT_PRED(crpcut::match<crpcut::regex>(re), os.str());

  }
}

#define ASSERT_VALUE(...) assert_expr(crpcut::expr::hook()->*__VA_ARGS__)

TESTSUITE(expr)
{
  TESTSUITE(match_pseudo_op)
  {
    TEST(regex_matches_std_string)
    {
      ASSERT_VALUE(std::string("apa") =~ crpcut::regex("apa"), true);
    }

    TEST(regex_mismatch_std_string_with_informative_message)
    {
      ASSERT_VALUE(std::string("apa") =~ crpcut::regex("katt"),
                   false,
                   "apa =~ regex(\"katt\"");
    }

    TEST(regex_matches_c_string)
    {
      const char *v = "lemur";
      ASSERT_VALUE(v =~ crpcut::regex(".*ur"), true);
    }

    TEST(regex_mismatch_c_string_with_informative_message)
    {
      const char *v = "lemur";
      ASSERT_VALUE(v =~ crpcut::regex("apa"), false,
                   "lemur =~ regex(\"apa\")");
    }

    TEST(regex_matches_string_literal)
    {
      ASSERT_VALUE("apa" =~ crpcut::regex("a.a"), true);
    }

    TEST(regex_mistach_c_string_with_informative_message)
    {
      ASSERT_VALUE("katt" =~ crpcut::regex("markatta"), false,
                   "katt =~ regex(\"markatta\")");
    }

    TEST(regex_matches_concatenated_strings)
    {
      ASSERT_VALUE(std::string("apa") + "katt" =~ crpcut::regex(".*"), true);
    }

    TEST(regex_mismatches_concatenated_strings_with_informative_message)
    {
      ASSERT_VALUE("apa" + std::string("katt") =~ crpcut::regex("markatta"),
                   false,
                   "apa + katt =~ regex(\"markatta\")");
    }
  }

  TESTSUITE(arithmetic)
  {
    TEST(simple_int_addition_is_computed_and_displayed_correctly)
    {
      const int v1 = 3;
      ASSERT_VALUE(v1+8, 11, "3 + 8");
    }

    struct s
    {
      static const int n = 11;
    };

    TEST(add_int_and_static_const_int_value_is_computed_and_displayed_correctly)
    {
      ASSERT_VALUE(s::n+3, 14, "11 + 3");
    }

    TEST(add_static_const_int_value_and_int_is_computed_and_displayed_correctly)
    {
      ASSERT_VALUE(3 + s::n, 14, "3 + 11");
    }

    TEST(add_char_array_and_const_std_string_is_computed_and_displayed_correctly)
    {
      const std::string katt("katta");
      ASSERT_VALUE("mar" + katt, std::string("markatta"), "mar + katta");
    }

    TEST(add_const_std_string_and_char_array_is_computed_and_displayed_correctly)
    {
      const std::string mar("mar");
      ASSERT_VALUE(mar + "katta", std::string("markatta"), "mar + katta");
    }

    TEST(class_static_const_plus_unsigned_equals_long_yields_correct_value_and_string_rep)
    {
      const unsigned m = 3;
      ASSERT_VALUE(s::n + m == 14UL, true, "11 + 3 == 14");
    }

    TEST(unsigned_plus_class_static_const_equals_long_is_computed_and_displayed_correctly)
    {
      const unsigned m = 3;
      ASSERT_VALUE(m + s::n == 14UL, true, "3 + 11 == 14");
    }

    TEST(short_plus_int_equals_class_static_const_is_computed_and_displayed_correctly)
    {
      const short m = 3;
      ASSERT_VALUE(m + 8 == s::n, true, "3 + 8 == 11");
    }

    template <typename T>
    class noncopyable
    {
    public:
      noncopyable(T n) : n_(n) {}
      operator T() const { return n_; }
      friend std::ostream& operator<<(std::ostream &os, const noncopyable &n)
      {
        return os << "nc(" << n.n_ << ")";
      }
    private:
      noncopyable(const noncopyable&);
      noncopyable& operator=(const noncopyable&);
      T n_;
    };

    template <typename T>
    class unstreamable
    {
    public:
      unstreamable(T t) : t_(t) {}
      unstreamable(const unstreamable& t) : t_(t.t_) {}
      unstreamable& operator=(const T& t) { t_ = t.t_; return *this;}
      operator T() const { return t_; }
    private:
      T t_;
    };

    template <typename T>
    std::ostream& operator<<(std::ostream&, const unstreamable<T>&) = delete;

    TEST(noncopyable_equals_int_is_computed_and_displayed_correctly)
    {
      ASSERT_VALUE(noncopyable<int>(3) == 4, false, "nc(3) == 4");
    }

    TEST(int_equals_noncopyable_yields_correct_value_and_string)
    {
      ASSERT_VALUE(4 == noncopyable<int>(3), false, "4 == nc(3)");
    }

    TEST(short_minus_small_unstreamable_is_computed_correctly_and_displayed_as_hexdump)
    {
      short s = 3;
      unstreamable<short> u(4);
      ASSERT_VALUE(s - u, -1, "3 - [[:digit:]]-byte object <[[:xdigit:] ]*>");
    }

    TEST(large_unstreamable_greater_than_long_double_is_computed_correctly_and_displayed_as_questionmark)
    {
      unstreamable<long double> e_ish = 2.78;
      long double pi_ish = 3.1416;
      ASSERT_VALUE(e_ish > pi_ish, false, "\\? > 3.1416");
    }

    TEST(left_shift_is_comuted_and_displayed_correctly)
    {
      int a = 3;
      int b = 4;
      ASSERT_VALUE(a << b, 48, "3 << 4");
    }

    TEST(right_shift_is_computed_and_displayed_correctly)
    {
      int a = 36;
      int b = 3;
      ASSERT_VALUE(a >> b, 4, "36 >> 3");
    }

    TEST(multiplication_is_computed_and_displayed_correctly)
    {
      int a = 5;
      int b = 7;
      ASSERT_VALUE(a*b, 35, "5 \\* 7");
    }

    TEST(division_is_computed_and_dispayed_correctly)
    {
      int a = 28;
      int b = 3;
      ASSERT_VALUE(a / b, 9, "28 / 3");
    }

    TEST(remainder_is_computed_and_displayed_correctly)
    {
      int a = 28;
      int b = 5;
      ASSERT_VALUE(a%b, 3, "28 % 5");
    }

    TEST(bitand_is_computed_and_displayed_correctly)
    {
      unsigned a = 0x12345678;
      unsigned b = 0x87654321;
      ASSERT_VALUE(a & b, 0x2244220U, "305419896 & 2271560481");
    }

    TEST(bitor_is_computed_and_dispayed_correctly)
    {
      unsigned a = 0x12345678;
      unsigned b = 0x87654321;
      ASSERT_VALUE(a | b, 0x97755779U, "305419896 \\| 2271560481");
    }

    TEST(bitxor_is_computed_and_displayed_correctly)
    {
      unsigned a = 0x12345678;
      unsigned b = 0x87654321;
      ASSERT_VALUE(a ^ b, 0x95511559, "305419896 ^ 2271560481");
    }

    TEST(multi_expr_with_mul_add_sub_not_equals_is_computed_and_displayed_correctly)
    {
      int a = 3;
      int b = 4;
      int c = 5;
      int d = 6;
      int e = 12;
      ASSERT_VALUE(a*b + c - d != e, true, "3 \\* 4 + 5 - 6 != 12");
    }

    TEST(multi_expr_left_shift_less_than_is_computed_and_displayed_correctly)
    {
      int a = 3;
      int b = 4;
      int c = 5;
      ASSERT_VALUE(c << b < a, false, "5 << 4 < 3");
    }

  }
}
