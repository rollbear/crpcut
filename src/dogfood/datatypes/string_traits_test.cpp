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

TESTSUITE(datatypes)
{
  TESTSUITE(string_traits)
  {
    TEST(get_c_str_from_std_string_gives_internal_rep)
    {
      std::string s = "apa";
      ASSERT_TRUE(crpcut::datatypes::string_traits::get_c_str(s) == s.c_str());
    }

    TEST(get_c_str_from_char_array_gives_ptr_to_first)
    {
      static const char s[] = "apa";
      ASSERT_TRUE(crpcut::datatypes::string_traits::get_c_str(s) == &s[0]);
    }

    class test_exception : public std::exception
    {
    public:
      const char *what() const noexcept { return "apa"; }
    };
    TEST(get_c_str_from_exception_gives_what_str)
    {
      test_exception e;
      ASSERT_TRUE(crpcut::datatypes::string_traits::get_c_str(e) == &e.what()[0]);
    }
  }
}
