/*
 * Copyright 2012-2013 Bjorn Fahller <bjorn@fahller.se>
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

#define CHECK(name, lh, rh, r)                                  \
  crpcut::tester<crpcut::comm::fail,                            \
                 CRPCUT_IS_ZERO_LIT(lh), decltype(lh),   \
                 CRPCUT_IS_ZERO_LIT(rh), decltype(rh)>   \
  ("location", #name, r, nullptr).name(lh, #lh, rh, #rh);
namespace {
  struct fix
  {
    fix() : reporter(os) {}
    crpcut::comm::reporter reporter;
    std::ostringstream os;
  };
}
TESTSUITE(binary_tester)
{
  TESTSUITE(equal)
  {
    TEST(c_str_to_std_string_with_same_chars_is_true, fix)
    {
      const char *p = "apa";
      std::string s = "apa";
      CHECK(EQ, p, s, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(std_string_to_c_str_with_same_chars_is_true, fix)
    {
      std::string s = "apa";
      const char *p = "apa";
      CHECK(EQ, s, p, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(c_str_to_greater_std_string_is_false, fix)
    {
      const char *p = "apa";
      std::string s = "katt";
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_EQ(p, s)\n"
        "  where p = apa\n"
        "        s = katt\n";
      CHECK(EQ, p, s, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(std_string_to_greater_c_str_is_false, fix)
    {
      std::string s = "apa";
      const char *p = "katt";
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_EQ(s, p)\n"
        "  where s = apa\n"
        "        p = katt\n";
      CHECK(EQ, s, p, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(valid_c_str_with_zero_literal_is_false, fix)
    {
      const char *p = "apa";
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_EQ(p, 0)\n"
        "  where p = apa\n";
      CHECK(EQ, p, nullptr, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(null_c_str_with_zero_literal_is_true, fix)
    {
      const char *p = nullptr;
      CHECK(EQ, p, nullptr, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(valid_c_str_with_same_ptr_value_is_true, fix)
    {
      const char *p = "apa";
      const char *q = p;
      CHECK(EQ, p, q, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(zero_literal_equal_test_with_null_c_str_is_true, fix)
    {
      const char *p = nullptr;
      CHECK(EQ, nullptr, p, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(zero_literal_with_valid_c_stris_false, fix)
    {
      const char *p = "apa";
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_EQ(0, p)\n"
        "  where p = apa\n";
      CHECK(EQ, nullptr, p, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(valid_c_str_with_other_c_str_to_same_string_is_false, fix)
    {
      const char *p = "begapa" + 3;
      const char *q = "rapa" + 1;
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_EQ(p, q)\n"
        "  where p = apa\n"
        "        q = apa\n";
      CHECK(EQ, p, q, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }
  }

  TESTSUITE(not_equal)
  {
    TEST(c_str_to_std_string_with_same_chars_is_false, fix)
    {
      const char *p = "apa";
      std::string s = "apa";
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_NE(p, s)\n"
        "  where p = apa\n"
        "        s = apa\n";
      CHECK(NE, p, s, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(std_string_to_c_str_with_same_chars_is_false, fix)
    {
      std::string s = "apa";
      const char *p = "apa";
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_NE(s, p)\n"
        "  where s = apa\n"
        "        p = apa\n";
      CHECK(NE, s, p, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(c_str_to_greater_std_string_is_true, fix)
    {
      const char *p = "apa";
      std::string s = "katt";
      CHECK(NE, p, s, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(std_string_to_greater_c_str_is_true, fix)
    {
      std::string s = "apa";
      const char *p = "katt";
      CHECK(NE, s, p, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(valid_c_str_with_zero_literal_is_true, fix)
    {
      const char *p = "apa";
      CHECK(NE, p, nullptr, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(null_c_str_with_zero_literal_is_false, fix)
    {
      const char *p = nullptr;
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_NE(p, 0)\n"
        "  where p = \n";
      CHECK(NE, p, nullptr, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(valid_c_str_with_same_ptr_value_is_false, fix)
    {
      const char *p = "apa";
      const char *q = p;
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_NE(p, q)\n"
        "  where p = apa\n"
        "        q = apa\n";
      CHECK(NE, p, q, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(zero_literal_with_null_c_str_is_false, fix)
    {
      const char *p = nullptr;
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_NE(0, p)\n"
        "  where p = \n";
      CHECK(NE, nullptr, p, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(zero_literal_with_valid_c_stris_true, fix)
    {
      const char *p = "apa";
      CHECK(NE, nullptr, p, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(valid_c_str_with_other_c_str_to_same_string_is_true, fix)
    {
      const char *p = "begapa" + 3;
      const char *q = "rapa" + 1;
      CHECK(NE, p, q, reporter);
      ASSERT_TRUE(os.str() == "");
    }
  }

  TESTSUITE(greater_than)
  {
    TEST(literal_zero_with_int_three_is_false, fix)
    {
      int n = 3;
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_GT(0, n)\n"
        "  where n = 3\n";
      CHECK(GT, nullptr, n, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(int_minus_three_with_literal_zero_is_false, fix)
    {
      int n = -3;
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_GT(n, 0)\n"
        "  where n = -3\n";
      CHECK(GT, n, nullptr, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(c_str_to_std_string_with_same_chars_is_false, fix)
    {
      const char *p = "apa";
      std::string s = "apa";
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_GT(p, s)\n"
        "  where p = apa\n"
        "        s = apa\n";
      CHECK(GT, p, s, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(std_string_to_c_str_with_same_chars_is_false, fix)
    {
      std::string s = "apa";
      const char *p = "apa";
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_GT(s, p)\n"
        "  where s = apa\n"
        "        p = apa\n";
      CHECK(GT, s, p, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(c_str_to_greater_std_string_is_false, fix)
    {
      const char *p = "apa";
      std::string s = "katt";
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_GT(p, s)\n"
        "  where p = apa\n"
        "        s = katt\n";
      CHECK(GT, p, s, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(std_string_to_greater_c_str_is_false, fix)
    {
      std::string s = "apa";
      const char *p = "katt";
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_GT(s, p)\n"
        "  where s = apa\n"
        "        p = katt\n";
      CHECK(GT, s, p, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }
  }

  TESTSUITE(greater_or_equal)
  {
    TEST(literal_zero_with_int_three_is_false, fix)
    {
      int n = 3;
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_GE(0, n)\n"
        "  where n = 3\n";
      CHECK(GE, nullptr, n, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(int_minus_three_with_literal_zero_is_false, fix)
    {
      int n = -3;
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_GE(n, 0)\n"
        "  where n = -3\n";
      CHECK(GE, n, nullptr, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(c_str_to_std_string_with_same_chars_is_true, fix)
    {
      const char *p = "apa";
      std::string s = "apa";
      CHECK(GE, p, s, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(std_string_to_c_str_with_same_chars_is_true, fix)
    {
      std::string s = "apa";
      const char *p = "apa";
      CHECK(GE, s, p, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(c_str_to_greater_std_string_is_false, fix)
    {
      const char *p = "apa";
      std::string s = "katt";
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_GE(p, s)\n"
        "  where p = apa\n"
        "        s = katt\n";
      CHECK(GE, p, s, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(std_string_to_greater_c_str_is_false, fix)
    {
      std::string s = "apa";
      const char *p = "katt";
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_GE(s, p)\n"
        "  where s = apa\n"
        "        p = katt\n";
      CHECK(GE, s, p, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }
  }

  TESTSUITE(less_than)
  {
    TEST(literal_zero_with_int_minus_three_is_false, fix)
    {
      int n = -3;
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_LT(0, n)\n"
        "  where n = -3\n";
      CHECK(LT, nullptr, n, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(int_three_with_literal_zero_is_false, fix)
    {
      int n = 3;
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_LT(n, 0)\n"
        "  where n = 3\n";
      CHECK(LT, n, nullptr, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(c_str_to_std_string_with_same_chars_is_false, fix)
    {
      const char *p = "apa";
      std::string s = "apa";
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_LT(p, s)\n"
        "  where p = apa\n"
        "        s = apa\n";
      CHECK(LT, p, s, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(std_string_to_c_str_with_same_chars_is_false, fix)
    {
      std::string s = "apa";
      const char *p = "apa";
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_LT(s, p)\n"
        "  where s = apa\n"
        "        p = apa\n";
      CHECK(LT, s, p, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(c_str_to_greater_std_string_is_true, fix)
    {
      const char *p = "apa";
      std::string s = "katt";
      CHECK(LT, p, s, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(std_string_to_greater_c_str_is_true, fix)
    {
      std::string s = "apa";
      const char *p = "katt";
      CHECK(LT, s, p, reporter);
      ASSERT_TRUE(os.str() == "");
    }
  }

  TESTSUITE(less_than_or_equal)
  {
    TEST(literal_zero_with_int_minus_three_is_false, fix)
    {
      int n = -3;
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_LE(0, n)\n"
        "  where n = -3\n";
      CHECK(LE, nullptr, n, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(int_three_with_literal_zero_is_false, fix)
    {
      int n = 3;
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_LE(n, 0)\n"
        "  where n = 3\n";
      CHECK(LE, n, nullptr, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(c_str_to_std_string_with_same_chars_is_true, fix)
    {
      const char *p = "apa";
      std::string s = "apa";
      CHECK(LE, p, s, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(std_string_to_c_str_with_same_chars_is_true, fix)
    {
      std::string s = "apa";
      const char *p = "apa";
      CHECK(LE, s, p, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(c_str_to_greater_std_string_is_true, fix)
    {
      const char *p = "apa";
      std::string s = "katt";
      CHECK(LE, p, s, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(std_string_to_greater_c_str_is_true, fix)
    {
      std::string s = "apa";
      const char *p = "katt";
      CHECK(LE, s, p, reporter);
      ASSERT_TRUE(os.str() == "");
    }
  }
}
