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

TESTSUITE(pred_test)
{
  class predicate
  {
  public:
    predicate(int n) : n_(n) {}
    int operator()(int a, int b = 0, int c = 0, int d = 0, int e = 0,
                   int f = 0, int g = 0, int h = 0, int i = 0) const
    {
      return n_ + a + b + c + d + e + f + g + h + i;
    }
    friend std::ostream &operator<<(std::ostream &os, const predicate& p)
    {
      return os << "predicate(" << p.n_ << ")";
    }
  private:
    int n_;
  };

  TEST(failed_1ary_lists_its_param_in_fail_message)
  {
    std::string msg;
    const char result_string[] =
      "  param1 = -1\n"
      "for pred: predicate(1)\n";
    bool b = crpcut::match_pred(msg, "pred", predicate(1), crpcut::params(-1));
    ASSERT_FALSE(b);
    ASSERT_TRUE(msg  == result_string);
  }

  TEST(successful_1ary_outputs_nothing)
  {
    std::string msg;
    bool b = crpcut::match_pred(msg, "pred", predicate(1), crpcut::params(1));
    ASSERT_TRUE(b);
    ASSERT_TRUE(msg == "");
  }

  TEST(failed_2ary_lists_its_params_in_fail_message)
  {
    std::string msg;
    const char result_string[] =
      "  param1 = -1\n"
      "  param2 = -2\n"
      "for pred: predicate(3)\n";
    bool b = crpcut::match_pred(msg, "pred", predicate(3),
                                crpcut::params(-1, -2));
    ASSERT_FALSE(b);
    ASSERT_TRUE(msg  == result_string);
  }

  TEST(successful_2ary_outputs_nothing)
  {
    std::string msg;
    bool b = crpcut::match_pred(msg, "pred", predicate(1),
                                crpcut::params(1,2));
    ASSERT_TRUE(b);
    ASSERT_TRUE(msg == "");
  }

  TEST(failed_3ary_lists_its_params_in_fail_message)
  {
    std::string msg;
    const char result_string[] =
      "  param1 = -1\n"
      "  param2 = -2\n"
      "  param3 = -3\n"
      "for pred: predicate(6)\n";
    bool b = crpcut::match_pred(msg, "pred", predicate(6),
                                crpcut::params(-1, -2, -3));
    ASSERT_FALSE(b);
    ASSERT_TRUE(msg  == result_string);
  }

  TEST(successful_3ary_outputs_nothing)
  {
    std::string msg;
    bool b = crpcut::match_pred(msg, "pred", predicate(1),
                                crpcut::params(1,2, 3));
    ASSERT_TRUE(b);
    ASSERT_TRUE(msg == "");
  }

  TEST(failed_4ary_lists_its_params_in_fail_message)
  {
    std::string msg;
    const char result_string[] =
      "  param1 = -1\n"
      "  param2 = -2\n"
      "  param3 = -3\n"
      "  param4 = -4\n"
      "for pred: predicate(10)\n";
    bool b = crpcut::match_pred(msg, "pred", predicate(10),
                                crpcut::params(-1, -2, -3, -4));
    ASSERT_FALSE(b);
    ASSERT_TRUE(msg  == result_string);
  }

  TEST(successful_4ary_outputs_nothing)
  {
    std::string msg;
    bool b = crpcut::match_pred(msg, "pred", predicate(1),
                                crpcut::params(1, 2, 3, 4));
    ASSERT_TRUE(b);
    ASSERT_TRUE(msg == "");
  }

  TEST(failed_5ary_lists_its_params_in_fail_message)
  {
    std::string msg;
    const char result_string[] =
      "  param1 = -1\n"
      "  param2 = -2\n"
      "  param3 = -3\n"
      "  param4 = -4\n"
      "  param5 = -5\n"
      "for pred: predicate(15)\n";
    bool b = crpcut::match_pred(msg, "pred", predicate(15),
                                crpcut::params(-1, -2, -3, -4, -5));
    ASSERT_FALSE(b);
    ASSERT_TRUE(msg  == result_string);
  }

  TEST(successful_5ary_outputs_nothing)
  {
    std::string msg;
    bool b = crpcut::match_pred(msg, "pred", predicate(1),
                                crpcut::params(1, 2, 3, 4, 5));
    ASSERT_TRUE(b);
    ASSERT_TRUE(msg == "");
  }

  TEST(failed_6ary_lists_its_params_in_fail_message)
  {
    std::string msg;
    const char result_string[] =
      "  param1 = -1\n"
      "  param2 = -2\n"
      "  param3 = -3\n"
      "  param4 = -4\n"
      "  param5 = -5\n"
      "  param6 = -6\n"
      "for pred: predicate(21)\n";
    bool b = crpcut::match_pred(msg, "pred", predicate(21),
                                crpcut::params(-1, -2, -3, -4, -5, -6));
    ASSERT_FALSE(b);
    ASSERT_TRUE(msg  == result_string);
  }

  TEST(successful_6ary_outputs_nothing)
  {
    std::string msg;
    bool b = crpcut::match_pred(msg, "pred", predicate(1),
                                crpcut::params(1, 2, 3, 4, 5, 6));
    ASSERT_TRUE(b);
    ASSERT_TRUE(msg == "");
  }

  TEST(failed_7ary_lists_its_params_in_fail_message)
  {
    std::string msg;
    const char result_string[] =
      "  param1 = -1\n"
      "  param2 = -2\n"
      "  param3 = -3\n"
      "  param4 = -4\n"
      "  param5 = -5\n"
      "  param6 = -6\n"
      "  param7 = -7\n"
      "for pred: predicate(28)\n";
    bool b = crpcut::match_pred(msg, "pred", predicate(28),
                                crpcut::params(-1, -2, -3, -4, -5, -6, -7));
    ASSERT_FALSE(b);
    ASSERT_TRUE(msg  == result_string);
  }

  TEST(successful_7ary_outputs_nothing)
  {
    std::string msg;
    bool b = crpcut::match_pred(msg, "pred", predicate(1),
                                crpcut::params(1, 2, 3, 4, 5, 6, 7));
    ASSERT_TRUE(b);
    ASSERT_TRUE(msg == "");
  }

  TEST(failed_8ary_lists_its_params_in_fail_message)
  {
    std::string msg;
    const char result_string[] =
      "  param1 = -1\n"
      "  param2 = -2\n"
      "  param3 = -3\n"
      "  param4 = -4\n"
      "  param5 = -5\n"
      "  param6 = -6\n"
      "  param7 = -7\n"
      "  param8 = -8\n"
      "for pred: predicate(36)\n";
    bool b = crpcut::match_pred(msg, "pred", predicate(36),
                                crpcut::params(-1, -2, -3, -4, -5, -6, -7, -8));
    ASSERT_FALSE(b);
    ASSERT_TRUE(msg  == result_string);
  }

  TEST(successful_8ary_outputs_nothing)
  {
    std::string msg;
    bool b = crpcut::match_pred(msg, "pred", predicate(1),
                                crpcut::params(1, 2, 3, 4, 5, 6, 7, 8));
    ASSERT_TRUE(b);
    ASSERT_TRUE(msg == "");
  }

  TEST(failed_9ary_lists_its_params_in_fail_message)
  {
    std::string msg;
    const char result_string[] =
      "  param1 = -1\n"
      "  param2 = -2\n"
      "  param3 = -3\n"
      "  param4 = -4\n"
      "  param5 = -5\n"
      "  param6 = -6\n"
      "  param7 = -7\n"
      "  param8 = -8\n"
      "  param9 = -9\n"
      "for pred: predicate(45)\n";
    bool b = crpcut::match_pred(msg, "pred", predicate(45),
                                crpcut::params(-1, -2, -3, -4, -5, -6, -7, -8, -9));
    ASSERT_FALSE(b);
    ASSERT_TRUE(msg  == result_string);
  }

  TEST(successful_9ary_outputs_nothing)
  {
    std::string msg;
    bool b = crpcut::match_pred(msg, "pred", predicate(1),
                                crpcut::params(1, 2, 3, 4, 5, 6, 7, 8, 9));
    ASSERT_TRUE(b);
    ASSERT_TRUE(msg == "");
  }

}




