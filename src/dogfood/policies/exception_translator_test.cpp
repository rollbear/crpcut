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

TESTSUITE(policies)
{
  TESTSUITE(exception_translator)
  {
    class root_class : public crpcut::policies::crpcut_exception_translator
    {
    public:
      using crpcut_exception_translator::do_try_all;
      root_class() : crpcut_exception_translator(0) {}
    };

    template <typename T>
    void assert_match(root_class &r, const char *expected_string)
    {
      try {
          throw T();
      }
      catch (...)
      {
          ASSERT_TRUE(r.do_try_all() == expected_string);
          return;
      }
      FAIL << "shouldn't get here";
    }

    TEST(an_empty_list_reports_ellipsis_caught)
    {
      root_class root;
      assert_match<int>(root, "...");
    }

    class djur {};
    class apa : public djur {};
    class orm : public djur {};
    class husdjur : public djur {};
    class katt : public husdjur {};
    class stol {};
    class fisk : djur {};

    CRPCUT_DECLARE_EXCEPTION_TRANSLATOR(djur_translator, djur &d);
    std::string djur_translator::crpcut_do_translate(djur &)
    {
      return "djur";
    }

    CRPCUT_DECLARE_EXCEPTION_TRANSLATOR(husdjur_translator, husdjur &d);
    std::string husdjur_translator::crpcut_do_translate(husdjur &)
    {
      return "husdjur";
    }

    CRPCUT_DECLARE_EXCEPTION_TRANSLATOR(apa_translator, apa &d);
    std::string apa_translator::crpcut_do_translate(apa &)
    {
      return "apa";
    }

    CRPCUT_DECLARE_EXCEPTION_TRANSLATOR(katt_translator, katt &d);
    std::string katt_translator::crpcut_do_translate(katt &)
    {
      return "katt";
    }

    CRPCUT_DECLARE_EXCEPTION_TRANSLATOR(orm_translator, orm &d);
    std::string orm_translator::crpcut_do_translate(orm &)
    {
      return "orm";
    }

    struct fix
    {
      fix() : root(), orm(root), katt(root), husdjur(root), djur(root) {}
      root_class root;
      orm_translator orm;
      katt_translator katt;
      husdjur_translator husdjur;
      djur_translator djur;
    };

    TEST(list_returns_first_matching_exception, fix)
    {
      assert_match<apa>(root, "djur");
    }

    TEST(list_returns_ellipsis_when_nothing_matches, fix)
    {
      assert_match<stol>(root, "...");
    }

  }
}







