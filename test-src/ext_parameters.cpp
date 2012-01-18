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

DEFINE_TEST_TAG(istream_ext_parameter);

TESTSUITE(ext_parameters)
{
  TEST(should_succeed_expected_value)
  {
    const char *p = crpcut::get_parameter("apa");
    INFO << p;
    ASSERT_EQ(p, std::string("katt"));
  }

  TEST(should_succeed_no_value)
  {
    const char *p = crpcut::get_parameter("orm");
    ASSERT_FALSE(p);
  }

  TEST(should_succeed_no_value_with_too_long_name)
  {
    const char *p = crpcut::get_parameter("apan");
    ASSERT_FALSE(p);
  }

  TEST(should_succeed_value_interpret)
  {
    std::string s(crpcut::get_parameter<std::string>("apa"));
    ASSERT_EQ(s, "katt");
  }

  TEST(should_fail_value_interpret)
  {
    crpcut::get_parameter<int>("apa");
  }

  TEST(should_fail_no_value_interpret)
  {
    crpcut::get_parameter<std::string>("orm");
  }

  TEST(should_fail_istream_nonexisting_parameter,
       WITH_TEST_TAG(istream_ext_parameter))
  {
    std::string s;
    crpcut::get_parameter<std::istream>("orm") >> s;
  }

  TEST(should_succeed_istream_string_value,
       WITH_TEST_TAG(istream_ext_parameter))
  {
    std::string s;
    crpcut::get_parameter<std::istream>("apa") >> s;
    ASSERT_TRUE(s == "katt");
  }

  TEST(should_fail_istream_parameter_with_wrong_type,
       WITH_TEST_TAG(istream_ext_parameter))
  {
    double d;
    crpcut::get_parameter<std::istream>("apa") >> d;
  }

  TEST(should_succeed_reading_multiple_values,
       WITH_TEST_TAG(istream_ext_parameter))
  {
    char b[5] = { 0, 0, 0, 0, 0 };
    crpcut::get_parameter<std::istream>("apa") >> b[0] >> b[1] >> b[2] >> b[3];
    ASSERT_TRUE(b == std::string("katt"));
  }

  TEST(should_fail_reading_too_many_values,
       WITH_TEST_TAG(istream_ext_parameter))
  {
    std::string s1, s2;
    crpcut::get_parameter<std::istream>("apa") >> s1 >> s2;
  }

  TEST(should_succeed_reading_stream_as_hex_value,
       WITH_TEST_TAG(istream_ext_parameter))
  {
    int n;
    crpcut::get_parameter<std::istream>("numeric") >> std::hex >> n;
    ASSERT_TRUE(n == 16);
  }

  TEST(should_succeed_reading_stream_as_octal_value,
       WITH_TEST_TAG(istream_ext_parameter))
  {
    int n;
    crpcut::get_parameter<std::istream>("numeric") >> std::oct >> n;
    ASSERT_TRUE(n == 8);
  }

  TEST(should_succeed_reading_stream_as_decimal_value,
       WITH_TEST_TAG(istream_ext_parameter))
  {
    int n;
    crpcut::get_parameter<std::istream>("numeric") >> std::dec >> n;
    ASSERT_TRUE(n == 10);
  }

  TEST(should_succeed_reading_stream_as_interpreted_base_value,
       WITH_TEST_TAG(istream_ext_parameter))
  {
    int n;
    crpcut::get_parameter<std::istream>("numeric") >> std::setbase(0) >> n;
    ASSERT_TRUE(n == 8);
  }

  TEST(should_fail_reading_relaxed_stream_value_of_wrong_type,
       WITH_TEST_TAG(istream_ext_parameter))
  {
    int n = 5;
    if (!(crpcut::get_parameter<crpcut::relaxed<std::istream> >("apa") >> n))
      {
        FAIL << "reading apa as int failed";
      }
  }

  TEST(should_succeed_reading_relaxed_stream_value_of_right_type,
       WITH_TEST_TAG(istream_ext_parameter))
  {
    int n = 5;
    if (!(crpcut::get_parameter<crpcut::relaxed<std::istream> >("numeric") >> n))
      {
        FAIL << "reading numeric as int failed";
      }
  }
}
