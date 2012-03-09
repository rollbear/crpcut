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

