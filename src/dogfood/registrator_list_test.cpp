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

#include <trompeloeil.hpp>
#include <crpcut.hpp>
#include "../poll.hpp"
#include "../registrator_list.hpp"
#include <vector>
#include <algorithm>
using trompeloeil::_;

namespace {

  class mock_poll : public crpcut::poll<crpcut::fdreader>
  {
  public:
    MAKE_MOCK3(do_add_fd, void(int, crpcut::fdreader*, int));
    MAKE_MOCK1(do_del_fd, void(int));
    MAKE_MOCK1(do_wait, descriptor(int));
    MAKE_CONST_MOCK0(do_num_fds, std::size_t());
  };

  typedef crpcut::crpcut_test_case_registrator reg;
  class mock_test_reg : public reg
  {
  public:
    typedef crpcut::tag::importance importance;
    mock_test_reg(const char *name, crpcut::namespace_info *ns)
    : reg(name, ns) {}
    MAKE_MOCK4(setup, void(crpcut::poll<crpcut::fdreader>&,
                           int, int, int));
    MAKE_MOCK0(run_test_case, void());
    MAKE_CONST_MOCK0(crpcut_tag, crpcut::tag&());
    MAKE_CONST_MOCK0(get_importance, importance());
    MAKE_CONST_MOCK0(is_naughty_child, bool());
    MAKE_CONST_MOCK0(freeze, void());
  };

  class test_tag_list_root : public crpcut::tag_list_root
  {
  public:
    crpcut::datatypes::fixed_string get_name() const
    {
      static crpcut::datatypes::fixed_string n = { "", 0 };
      return n;
    }
    importance get_importance() const { return critical; }
  };

  template <crpcut::tag::importance I>
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
    virtual importance get_importance() const { return I; }
  private:
    crpcut::datatypes::fixed_string name_;
  };


  typedef crpcut::registrator_list list;

  struct fix
  {
    fix()
    : unnamed_namespace(0,0),
      reptil_namespace("reptil", &unnamed_namespace),
      insekt_namespace("insekt", &unnamed_namespace),
      apa("apa", &unnamed_namespace),
      katt("katt", &unnamed_namespace),
      ko("ko", &unnamed_namespace),
      lemur("lemur", &unnamed_namespace),
      reptil_orm("orm", &reptil_namespace),
      reptil_krokodil("krokodil", &reptil_namespace),
      insekt_mygga("mygga", &insekt_namespace),
      insekt_bi("bi", &insekt_namespace),
      trilobit("trilobit", &unnamed_namespace),
      unnamed_tag(),
      tamdjur("tam", &unnamed_tag),
      giftiga("giftiga", &unnamed_tag),
      ignored("ignorerad", &unnamed_tag),
      anonymous_disabled("", &unnamed_tag),
      ignored_disabled("disabled", &unnamed_tag),
      apa_imp_x(NAMED_ALLOW_CALL(apa, get_importance()).RETURN(unnamed_tag.get_importance())),
      apa_tag_x(NAMED_ALLOW_CALL(apa, crpcut_tag()).RETURN(std::ref(unnamed_tag))),
      katt_imp_x(NAMED_ALLOW_CALL(katt, get_importance()).RETURN(tamdjur.get_importance())),
      katt_tag_x(NAMED_ALLOW_CALL(katt, crpcut_tag()).RETURN(std::ref(tamdjur))),
      ko_imp_x(NAMED_ALLOW_CALL(ko, get_importance()).RETURN(tamdjur.get_importance())),
      ko_tag_x(NAMED_ALLOW_CALL(ko, crpcut_tag()).RETURN(std::ref(tamdjur))),
      lemur_imp_x(NAMED_ALLOW_CALL(lemur, get_importance()).RETURN(crpcut::tag::disabled)),
      lemur_tag_x(NAMED_ALLOW_CALL(lemur, crpcut_tag()).RETURN(std::ref(anonymous_disabled))),
      orm_imp_x(NAMED_ALLOW_CALL(reptil_orm, get_importance())
                .RETURN(giftiga.get_importance())),
      orm_tag_x(NAMED_ALLOW_CALL(reptil_orm, crpcut_tag())
                .RETURN(std::ref(giftiga))),
      krokodil_imp_x(NAMED_ALLOW_CALL(reptil_krokodil, get_importance())
                     .RETURN(ignored.get_importance())),
      krokodil_tag_x(NAMED_ALLOW_CALL(reptil_krokodil, crpcut_tag())
                     .RETURN(std::ref(ignored))),
      mygga_imp_x(NAMED_ALLOW_CALL(insekt_mygga, get_importance())
                  .RETURN(giftiga.get_importance())),
      mygga_tag_x(NAMED_ALLOW_CALL(insekt_mygga, crpcut_tag())
                  .RETURN(std::ref(giftiga))),
      bi_imp_x(NAMED_ALLOW_CALL(insekt_bi, get_importance())
               .RETURN(giftiga.get_importance())),
      bi_tag_x(NAMED_ALLOW_CALL(insekt_bi, crpcut_tag())
               .RETURN(std::ref(giftiga))),
      trilobit_imp_x(NAMED_ALLOW_CALL(trilobit, get_importance())
                     .RETURN(crpcut::tag::disabled)),
      trilobit_tag_x(NAMED_ALLOW_CALL(trilobit, crpcut_tag())
                     .RETURN(std::ref(ignored_disabled)))
    {
      apa.link_before(the_list);
      katt.link_before(the_list);
      ko.link_before(the_list);
      lemur.link_before(the_list);
      reptil_orm.link_before(the_list);
      reptil_krokodil.link_before(the_list);
      insekt_mygga.link_before(the_list);
      insekt_bi.link_before(the_list);
      trilobit.link_before(the_list);

    }
    list the_list;
    crpcut::namespace_info unnamed_namespace;
    crpcut::namespace_info reptil_namespace;
    crpcut::namespace_info insekt_namespace;
    mock_test_reg apa;
    mock_test_reg katt;
    mock_test_reg ko;
    mock_test_reg lemur;
    mock_test_reg reptil_orm;
    mock_test_reg reptil_krokodil;
    mock_test_reg insekt_mygga;
    mock_test_reg insekt_bi;
    mock_test_reg trilobit;
    test_tag_list_root        unnamed_tag;
    test_tag<crpcut::tag::non_critical> tamdjur;
    test_tag<crpcut::tag::critical>     giftiga;
    test_tag<crpcut::tag::ignored>      ignored;
    test_tag<crpcut::tag::critical>     anonymous_disabled;
    test_tag<crpcut::tag::ignored>      ignored_disabled;
    std::unique_ptr<trompeloeil::expectation> apa_imp_x;
    std::unique_ptr<trompeloeil::expectation> apa_tag_x;
    std::unique_ptr<trompeloeil::expectation> katt_imp_x;
    std::unique_ptr<trompeloeil::expectation> katt_tag_x;
    std::unique_ptr<trompeloeil::expectation> ko_imp_x;
    std::unique_ptr<trompeloeil::expectation> ko_tag_x;
    std::unique_ptr<trompeloeil::expectation> lemur_imp_x;
    std::unique_ptr<trompeloeil::expectation> lemur_tag_x;
    std::unique_ptr<trompeloeil::expectation> orm_imp_x;
    std::unique_ptr<trompeloeil::expectation> orm_tag_x;
    std::unique_ptr<trompeloeil::expectation> krokodil_imp_x;
    std::unique_ptr<trompeloeil::expectation> krokodil_tag_x;
    std::unique_ptr<trompeloeil::expectation> mygga_imp_x;
    std::unique_ptr<trompeloeil::expectation> mygga_tag_x;
    std::unique_ptr<trompeloeil::expectation> bi_imp_x;
    std::unique_ptr<trompeloeil::expectation> bi_tag_x;
    std::unique_ptr<trompeloeil::expectation> trilobit_imp_x;
    std::unique_ptr<trompeloeil::expectation> trilobit_tag_x;
  };


  template <typename T, typename U>
  void match_and_remove(T& t, U u)
  {
    typename T::iterator i = std::find(t.begin(), t.end(), u);
    if (i == t.end()) FAIL << "Couldn't find " << *u;
    std::swap(*i, t.back());
    t.pop_back();
  }

  template <typename T>
  void match_all(T& t, list &r)
  {
    for (reg *i = r.first(); i; i = r.next_after(i))
      {
        match_and_remove(t, i);
      }
    if (t.size())
      {
        std::ostringstream os;
        for (typename T::iterator i = t.begin(); i != t.end(); ++i)
          {
            os << **i << " ";
          }
        FAIL << os.str() << " were not counted for";
      }
  }
}

TESTSUITE(registrator_list)
{

  TEST(construction_does_nothing)
  {
    list l;
  }


  TESTSUITE(filtering)
  {
    TEST(empty_name_list_filters_out_ignored_only, fix)
    {
      std::ostringstream os;
      const char * const p = 0;
      std::pair<unsigned, unsigned> rv =
          the_list.filter_out_or_throw(&p, os, 1);
      ASSERT_TRUE(rv.first == 9U);
      ASSERT_TRUE(rv.second == 9U);
      std::vector<reg*> expected;
      expected.push_back(&apa);
      expected.push_back(&katt);
      expected.push_back(&ko);
      expected.push_back(&reptil_orm);
      expected.push_back(&insekt_mygga);
      expected.push_back(&insekt_bi);
      expected.push_back(&lemur);
      match_all(expected, the_list);
    }

    TEST(single_suite_name_picks_only_tests_in_that_suite, fix)
    {
      std::ostringstream os;
      const char *p[] = { "insekt", 0 };
      std::pair<unsigned, unsigned> rv =
          the_list.filter_out_or_throw(p, os, 1);
      ASSERT_TRUE(rv.first == 2U);
      ASSERT_TRUE(rv.second == 9U);
      std::vector<reg*> expected;
      expected.push_back(&insekt_mygga);
      expected.push_back(&insekt_bi);
      match_all(expected, the_list);
    }

    TEST(suite_and_test_picks_all_in_suite_plus_singled_out, fix)
    {
      std::ostringstream os;
      const char *p[] = { "insekt", "reptil::orm", 0 };
      std::pair<unsigned, unsigned> rv =
          the_list.filter_out_or_throw(p, os, 1);
      ASSERT_TRUE(rv.first == 3U);
      ASSERT_TRUE(rv.second == 9U);
      std::vector<reg*> expected;
      expected.push_back(&reptil_orm);
      expected.push_back(&insekt_mygga);
      expected.push_back(&insekt_bi);
      match_all(expected, the_list);
    }

    TEST(lookup_disabled_ignored_throws, fix)
    {
      std::ostringstream os;
      const char *p[] = { "trilobit", 0 };
      ASSERT_THROW(the_list.filter_out_or_throw(p, os, 1), int);
    }

    TEST(multiple_names_excludes_non_matching_disabled, fix)
    {
      std::ostringstream os;
      const char *p[] = { "insekt", "lemur", "reptil::orm", 0 };
      std::pair<unsigned, unsigned> rv =
          the_list.filter_out_or_throw(p, os, 1);
      ASSERT_TRUE(rv.first == 4U);
      ASSERT_TRUE(rv.second == 9U);
      std::vector<reg*> expected;
      expected.push_back(&insekt_mygga);
      expected.push_back(&insekt_bi);
      expected.push_back(&reptil_orm);
      expected.push_back(&lemur);
      match_all(expected, the_list);
    }

    TEST(unmatched_name_throws_with_singularis_msg_form, fix)
    {
      std::ostringstream os;
      const char *p[] = { "insekt", "tupp", 0 };
      ASSERT_THROW(the_list.filter_out_or_throw(p, os, 1), int);
      ASSERT_TRUE(os.str() == "tupp does not match any test names\n");
    }

    TEST(unmatched_names_throws_with_pluralis_msg_form, fix)
    {
      std::ostringstream os;
      const char *p[] = { "reptil", "insekt", "tupp", "daggmask", 0 };
      ASSERT_THROW(the_list.filter_out_or_throw(p, os, 1), int);
      ASSERT_TRUE(os.str() == "tupp, daggmask do not match any test names\n");
    }
  }

  TESTSUITE(listing)
  {
    TEST(list_all_tests_unfiltered_gives_full_header_and_tag_names, fix)
    {
      std::ostringstream os;
      the_list.list_tests_to(os, 8U);
      static const char result[] =
          "      tag : test-name\n"
          "=====================\n"
          "!         : apa\n"
          "?     tam : katt\n"
          "?     tam : ko\n"
          "-         : lemur\n"
          "! giftiga : reptil::orm\n"
    //    "*         : reptil::krokodil\n"
          "! giftiga : insekt::mygga\n"
          "! giftiga : insekt::bi\n"
          "-disabled : trilobit\n";
       ASSERT_TRUE(os.str() == result);
    }

    TEST(list_all_tests_unfiltered_with_no_tag_margin_gives_name_list_only,
         fix)
    {
      std::ostringstream os;
      the_list.list_tests_to(os, 0U);
      static const char result[] =
          "!apa\n"
          "?katt\n"
          "?ko\n"
          "-lemur\n"
          "!reptil::orm\n"
    //    "*reptil::krokodil\n"
          "!insekt::mygga\n"
          "!insekt::bi\n"
          "-trilobit\n";
       ASSERT_TRUE(os.str() == result);

    }
  }
}







