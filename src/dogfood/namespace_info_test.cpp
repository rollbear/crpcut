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

TESTSUITE(namespace_info)
{
  TEST(named_namespace_without_root_matches_anything)
  {
    crpcut::namespace_info info{"apa", nullptr};
    static const char name[] = "orm";
    const char *match = info.match_name(name);
    ASSERT_TRUE(match == name);
  }

  TEST(named_namespace_with_empty_parent_matches_beginning)
  {
    crpcut::namespace_info root{nullptr, nullptr};
    crpcut::namespace_info apa{"apa", &root};
    static const char name[] = "apa::katt";
    const char *match = apa.match_name(name);
    ASSERT_TRUE(match == name + 3);
    ASSERT_TRUE(apa.full_name_len() == 5U);
  }

  TEST(named_namespace_with_empty_parent_returns_null_on_mismatch)
  {
    crpcut::namespace_info root{nullptr, nullptr};
    crpcut::namespace_info apa{"apa", &root};
    static const char name[] = "katt";
    const char *match = apa.match_name(name);
    ASSERT_FALSE(match);
    std::ostringstream os;
    os << apa;
    ASSERT_TRUE(os.str() == "apa::");
  }

  TEST(nested_namespace_matches_beginning_of_name)
  {
    crpcut::namespace_info root{nullptr, nullptr};
    crpcut::namespace_info apa{"apa", &root};
    crpcut::namespace_info katt{"katt", &apa};
    ASSERT_TRUE(katt.full_name_len() == 11U);
    const char name[] = "apa";
    const char *match = katt.match_name(name);
    ASSERT_TRUE(match == name + 3);
    std::ostringstream os;
    os << katt;
    ASSERT_TRUE(os.str() == "apa::katt::");
  }

  TEST(nested_namespace_returns_null_on_mismatch_in_upper_part)
  {
    crpcut::namespace_info root{nullptr, nullptr};
    crpcut::namespace_info apa{"apa", &root};
    crpcut::namespace_info katt{"katt", &apa};
    ASSERT_FALSE(katt.match_name("apa::orm"));
  }
}







