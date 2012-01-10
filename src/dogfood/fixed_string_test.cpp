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


TESTSUITE(fixed_string)
{
  static const char common_val[] = "lemur";
  static const crpcut::datatypes::fixed_string zero = { 0, 0 };
  static const crpcut::datatypes::fixed_string no_len = { "apa", 0 };
  static const crpcut::datatypes::fixed_string lemur = { common_val, 5 };
  static const crpcut::datatypes::fixed_string lem = { common_val, 3 };

  TEST(zero_initialized_string_is_false)
  {
    ASSERT_FALSE(zero);
  }

  TEST(zero_len_string_is_false)
  {
    ASSERT_FALSE(no_len);
  }

  TEST(two_empty_strings_are_equal)
  {
    ASSERT_TRUE(zero == no_len);
    ASSERT_FALSE(zero != no_len);
  }

  TEST(substrings_are_inequal)
  {
    ASSERT_FALSE(lemur == lem);
    ASSERT_TRUE(lemur != lem);
    ASSERT_FALSE(lemur == zero);
    ASSERT_TRUE(lemur != zero);
    ASSERT_FALSE(lemur == no_len);
    ASSERT_TRUE(lemur != no_len);
    ASSERT_FALSE(lem == zero);
    ASSERT_TRUE(lem != zero);
    ASSERT_FALSE(lem == no_len);
    ASSERT_TRUE(lem != no_len);
  }
}
