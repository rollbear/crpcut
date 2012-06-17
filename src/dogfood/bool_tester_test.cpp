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

#define CHECK(name, expr, r)                                            \
  do {                                                                  \
    crpcut::bool_tester<crpcut::comm::fail>tester_obj("location", r);   \
    tester_obj.name((expr), #expr);                                     \
  } while (0)

namespace {
  struct fix
  {
    fix() : reporter(os) {}
    crpcut::comm::reporter reporter;
    std::ostringstream os;
  };

  template <bool v>
  struct truth
  {
    void secret_func() const {};
  public:
    typedef void (truth::*bool_type)() const;
    operator bool_type() const { return v ? &truth::secret_func : 0; }
    friend
    std::ostream &operator<<(std::ostream &os, const truth&)
    {
      static const char* s[] = { "false", "true" };
      return os << "truth<" << s[v] << ">";
    }
  };
}
TESTSUITE(bool_tester)
{
  TESTSUITE(assert_true)
  {
    TEST(null_ptr_is_false, fix)
    {
      char *p = 0;
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_TRUE(p)\n"
        "  is evaluated as:\n"
        "    ";
      CHECK(check_true, p, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(zero_integer_is_false, fix)
    {
      const int n = 0;
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_TRUE(n)\n"
        "  is evaluated as:\n"
        "    0";
      CHECK(check_true, n, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(non_null_char_ptr_is_true, fix)
    {
      const char *p = "apa";
      CHECK(check_true, p, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(non_zero_int_is_true, fix)
    {
      int n = 5;
      CHECK(check_true, n, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(true_object_is_true, fix)
    {
      truth<true> obj;
      CHECK(check_true, obj, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(false_object_is_false, fix)
    {
      truth<false> obj;
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_TRUE(obj)\n"
        "  is evaluated as:\n"
        "    truth<false>";
      CHECK(check_true, obj, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }
  }

  TESTSUITE(assert_false)
  {
    TEST(null_ptr_is_false, fix)
    {
      const char *p = 0;
      CHECK(check_false, p, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(zero_integer_is_false, fix)
    {
      int n = 0;
      CHECK(check_false, n, reporter);
      ASSERT_TRUE(os.str() == "");
    }

    TEST(non_null_char_ptr_is_true, fix)
    {
      const char *p = "apa";
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_FALSE(p)\n"
        "  is evaluated as:\n"
        "    apa";
      CHECK(check_false, p, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(non_zero_int_is_true, fix)
    {
      int n = 5;
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_FALSE(n)\n"
        "  is evaluated as:\n"
        "    5";
      CHECK(check_false, n, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(true_object_is_true, fix)
    {
      truth<true> obj;
      const char violation_report[] =
        "\nlocation\n"
        "VERIFY_FALSE(obj)\n"
        "  is evaluated as:\n"
        "    truth<true>";
      CHECK(check_false, obj, reporter);
      ASSERT_TRUE(os.str() == violation_report);
    }

    TEST(false_object_is_false, fix)
    {
      truth<false> obj;
      CHECK(check_false, obj, reporter);
      ASSERT_TRUE(os.str() == "");
    }
  }
}
