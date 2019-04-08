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
#include "../tag_filter.hpp"

namespace {
  class test_tag_list_root : public crpcut::tag_list_root
  {
    crpcut::datatypes::fixed_string get_name() const
    {
      static crpcut::datatypes::fixed_string n = { "", 0 };
      return n;
    }
  };

  class test_tag : public crpcut::tag
  {
  public:
    template <std::size_t N>
    test_tag(const char (&f)[N], crpcut::tag_list_root* root)
      : crpcut::tag(int(N-1), root)
    {
      name_.str = f;
      name_.len = N - 1;
    }
    virtual crpcut::datatypes::fixed_string get_name() const { return name_; }
  private:
    crpcut::datatypes::fixed_string name_;
  };

  class test_tag_list
  {
  protected:
    test_tag_list()
      : root(),
        apa("apa", &root),
        katt("katt", &root),
        ko("ko", &root),
        lemur("lemur", &root)
    {}
    test_tag_list_root root;
    test_tag           apa;
    test_tag           katt;
    test_tag           ko;
    test_tag           lemur;
  };

  template <std::size_t N>
  crpcut::datatypes::fixed_string s(const char (&f)[N])
  {
    crpcut::datatypes::fixed_string rv = { f, N - 1 };
    return rv;
  }
}

TESTSUITE(tag_filter)
{
  TEST(a_default_initialized_filter_matches_everything_as_critical,
       test_tag_list)
  {
    crpcut::tag_filter filter;
    ASSERT_TRUE(filter.lookup(s("apa")) == crpcut::tag::critical);
    filter.assert_names(root);
  }

  TEST(one_name_selects_exact_match_only,
       test_tag_list)
  {
    crpcut::tag_filter filter("apa");
    ASSERT_TRUE(filter.lookup(s("apa")) == crpcut::tag::critical);
    ASSERT_TRUE(filter.lookup(s("ap")) == crpcut::tag::ignored);
    ASSERT_TRUE(filter.lookup(s("apan")) == crpcut::tag::ignored);
    filter.assert_names(root);
  }

  TEST(comma_separated_names_selects_matches,
       test_tag_list)
  {
    crpcut::tag_filter filter("apa,katt,ko");
    ASSERT_TRUE(filter.lookup(s("apa")) == crpcut::tag::critical);
    ASSERT_TRUE(filter.lookup(s("katt")) == crpcut::tag::critical);
    ASSERT_TRUE(filter.lookup(s("ko")) == crpcut::tag::critical);
    ASSERT_TRUE(filter.lookup(s("ap")) == crpcut::tag::ignored);
    ASSERT_TRUE(filter.lookup(s("apan")) == crpcut::tag::ignored);
    ASSERT_TRUE(filter.lookup(s("kat")) == crpcut::tag::ignored);
    ASSERT_TRUE(filter.lookup(s("katta")) == crpcut::tag::ignored);
    ASSERT_TRUE(filter.lookup(s("k")) == crpcut::tag::ignored);
    ASSERT_TRUE(filter.lookup(s("koo")) == crpcut::tag::ignored);
    filter.assert_names(root);
  }

  TEST(negative_comma_separated_list_ignores,
       test_tag_list)
  {
    crpcut::tag_filter filter("-apa,katt,ko");
    ASSERT_TRUE(filter.lookup(s("apa")) == crpcut::tag::ignored);
    ASSERT_TRUE(filter.lookup(s("katt")) == crpcut::tag::ignored);
    ASSERT_TRUE(filter.lookup(s("ko")) == crpcut::tag::ignored);
    ASSERT_TRUE(filter.lookup(s("ap")) == crpcut::tag::critical);
    ASSERT_TRUE(filter.lookup(s("apan")) == crpcut::tag::critical);
    ASSERT_TRUE(filter.lookup(s("kat")) == crpcut::tag::critical);
    ASSERT_TRUE(filter.lookup(s("katta")) == crpcut::tag::critical);
    ASSERT_TRUE(filter.lookup(s("k")) == crpcut::tag::critical);
    ASSERT_TRUE(filter.lookup(s("koo")) == crpcut::tag::critical);
    filter.assert_names(root);
  }

  TEST(noncritical_comma_separated_from_select_all,
       test_tag_list)
  {
    crpcut::tag_filter filter("/apa,katt,ko");
    ASSERT_TRUE(filter.lookup(s("apa")) == crpcut::tag::non_critical);
    ASSERT_TRUE(filter.lookup(s("ap")) == crpcut::tag::critical);
    ASSERT_TRUE(filter.lookup(s("apan")) == crpcut::tag::critical);
    ASSERT_TRUE(filter.lookup(s("katt")) == crpcut::tag::non_critical);
    ASSERT_TRUE(filter.lookup(s("kat")) == crpcut::tag::critical);
    ASSERT_TRUE(filter.lookup(s("katta")) == crpcut::tag::critical);
    ASSERT_TRUE(filter.lookup(s("ko")) == crpcut::tag::non_critical);
    ASSERT_TRUE(filter.lookup(s("k")) == crpcut::tag::critical);
    ASSERT_TRUE(filter.lookup(s("koo")) == crpcut::tag::critical);
    filter.assert_names(root);
  }

  TEST(noncritical_negative_comma_separated_from_select_all,
       test_tag_list)
  {
    crpcut::tag_filter filter("/-apa,katt,ko");
    ASSERT_TRUE(filter.lookup(s("apa")) == crpcut::tag::critical);
    ASSERT_TRUE(filter.lookup(s("ap")) == crpcut::tag::non_critical);
    ASSERT_TRUE(filter.lookup(s("apan")) == crpcut::tag::non_critical);
    ASSERT_TRUE(filter.lookup(s("katt")) == crpcut::tag::critical);
    ASSERT_TRUE(filter.lookup(s("kat")) == crpcut::tag::non_critical);
    ASSERT_TRUE(filter.lookup(s("katta")) == crpcut::tag::non_critical);
    ASSERT_TRUE(filter.lookup(s("ko")) == crpcut::tag::critical);
    ASSERT_TRUE(filter.lookup(s("k")) == crpcut::tag::non_critical);
    ASSERT_TRUE(filter.lookup(s("koo")) == crpcut::tag::non_critical);
    filter.assert_names(root);
  }

  TEST(combined_lists_selects_critical_and_non_critical,
       test_tag_list)
  {
    crpcut::tag_filter filter("apa,katt,ko,lemur/-katt,lemur");
    ASSERT_TRUE(filter.lookup(s("apa")) == crpcut::tag::non_critical);
    ASSERT_TRUE(filter.lookup(s("katt")) == crpcut::tag::critical);
    ASSERT_TRUE(filter.lookup(s("ko")) == crpcut::tag::non_critical);
    ASSERT_TRUE(filter.lookup(s("lemur")) == crpcut::tag::critical);
  }

  TEST(non_existing_tag_names_throws,
       test_tag_list)
  {
    ASSERT_THROW(crpcut::tag_filter("apa,katt,ko,tupp").assert_names(root),
                 crpcut::tag_filter::spec_error&);
    ASSERT_THROW(crpcut::tag_filter("tupp,katt,ko").assert_names(root),
                 crpcut::tag_filter::spec_error&);
    ASSERT_THROW(crpcut::tag_filter("/-tupp").assert_names(root),
                 crpcut::tag_filter::spec_error&);
    ASSERT_THROW(crpcut::tag_filter("/apa,katt,ko,tupp").assert_names(root),
                 crpcut::tag_filter::spec_error&);
    ASSERT_THROW(crpcut::tag_filter("//").assert_names(root),
                 crpcut::tag_filter::spec_error&);
  }
}
