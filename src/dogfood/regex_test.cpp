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

TESTSUITE(regex)
{
  crpcut::regex move_regex(crpcut::regex r)
  {
    return r;
  }

  TEST(default_flagged_from_c_string_can_match_std_string)
  {
    ASSERT_SCOPE_HEAP_LEAK_FREE
    {
      crpcut::regex r("apa.*katt");
      ASSERT_TRUE(r(std::string("en apa och en katt!")));
      std::ostringstream os;
      os << r;
      ASSERT_TRUE(os.str() == "");
    }
  }

  TEST(default_flagged_from_std_string_can_match__string)
  {
    ASSERT_SCOPE_HEAP_LEAK_FREE
    {
      crpcut::regex r(std::string("apa.*katt"));
      ASSERT_TRUE(r("en apa och en katt!"));
      std::ostringstream os;
      os << r;
      ASSERT_TRUE(os.str() == "");
    }
  }

  TEST(default_flagged_with_error_never_matches_and_gives_right_message)
  {
    ASSERT_SCOPE_HEAP_LEAK_FREE
    {
      crpcut::regex r("[apa");
      std::ostringstream os;
      os << r;
      ASSERT_TRUE(os.str() != "");
      ASSERT_TRUE(os.str() != "did not match");
      ASSERT_FALSE(r(""));
    }
  }

  TEST(default_flagged_without_match_gives_does_not_match_message)
  {
    ASSERT_SCOPE_HEAP_LEAK_FREE
    {
      crpcut::regex r("apa.*katt");
      ASSERT_FALSE(r("orm"));
      std::ostringstream os;
      os << r;
      ASSERT_TRUE(os.str() == "\"orm\" does not match");
    }
  }

  TEST(moved_regex_matches_and_mismatches_as_normal)
  {
    ASSERT_SCOPE_HEAP_LEAK_FREE
    {
      crpcut::regex r = move_regex(crpcut::regex("apa.*(katt){2}",
                                                 crpcut::regex::e));
      std::ostringstream os;
      os << r;
      ASSERT_TRUE(os.str() == "");
      ASSERT_TRUE(r("en apa och en kattkatt!"));
      os << r;
      ASSERT_TRUE(os.str() == "");
      ASSERT_FALSE(r("katt"));
      os << r;
      ASSERT_TRUE(os.str() == "\"katt\" does not match");
    }
  }

}







