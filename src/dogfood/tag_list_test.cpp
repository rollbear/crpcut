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
#include <sstream>

#define S(x) crpcut::datatypes::fixed_string::make(x)

namespace {
  class test_list : public crpcut::tag_list_root
  {
    virtual crpcut::datatypes::fixed_string get_name() const
    {
      return S("");
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

  struct fix
  {
    fix() : apa("apa", &list), lemur("lemur", &list), ko("ko", &list),
            longish("a_rather_longish_and_unpleasant_kind_of_name", &list){}
    test_list list;
    test_tag apa;
    test_tag lemur;
    test_tag ko;
    test_tag longish;
  };


  struct tag_result {
    crpcut::tag             *t;
    crpcut::tag::importance  i;
  };
  template <size_t N>
  void verify_tags(tag_result (&result)[N], crpcut::tag_list_root &list)
  {
    crpcut::tag_list_root::iterator it = list.begin();
    for (size_t i = 0; i < N; ++i)
      {
        INFO << "i=" << i;
        ASSERT_TRUE(result[i].t == &*it);
        ASSERT_TRUE(result[i].i == it->get_importance());
        ++it;
      }
  }
}


TESTSUITE(tag_list)
{
  TESTSUITE(printing)
  {
    TEST(empty_list_outputs_nothing)
    {
      std::ostringstream os;
      {
        test_list list;
        list.print_to(os);
      }
      ASSERT_TRUE(os.str() == "");
    }

    TEST(populated_list_is_printed_by_line_in_order_of_registration)
      {
        std::ostringstream os;
      {
        fix f;
        f.list.print_to(os);
      }
      static const char expected[] =
          "apa\n"
          "lemur\n"
          "ko\n"
          "a_rather_longish_and_unpleasant_kind_of_name\n";
      ASSERT_TRUE(os.str() == expected);
    }
  }


  TESTSUITE(configuring)
  {
    TEST(null_specification_makes_all_tags_critical, fix)
    {
      list.configure_importance(nullptr);
      tag_result r[] =
      {
       { &apa, crpcut::tag::critical },
       { &lemur, crpcut::tag::critical },
       { &ko, crpcut::tag::critical },
       { &longish, crpcut::tag::critical },
       { &list, crpcut::tag::critical }
      };
      verify_tags(r, list);
    }

    TEST(selected_tags_are_critical_the_rest_ignored, fix)
    {
      list.configure_importance("ko,apa");
      tag_result r[] =
      {
        { &apa, crpcut::tag::critical },
        { &lemur, crpcut::tag::ignored },
        { &ko, crpcut::tag::critical },
        { &longish, crpcut::tag::ignored },
        { &list, crpcut::tag::ignored }
      };
      verify_tags(r, list);
    }

    TEST(deselected_tags_are_ignored_the_rest_are_critical, fix)
    {
      list.configure_importance("-apa,ko");
      tag_result r[] =
      {
       { &apa, crpcut::tag::ignored },
       { &lemur, crpcut::tag::critical },
       { &ko, crpcut::tag::ignored },
       { &longish, crpcut::tag::critical },
       { &list, crpcut::tag::critical }
      };
      verify_tags(r, list);
    }

    TEST(marked_non_critical_are_non_critical_the_rest_as_selected, fix)
    {
      list.configure_importance("-apa/ko,lemur");
      tag_result r[] =
      {
        { &apa, crpcut::tag::ignored },
        { &lemur, crpcut::tag::non_critical },
        { &ko, crpcut::tag::non_critical },
        { &longish, crpcut::tag::critical },
        { &list, crpcut::tag::critical }
      };
      verify_tags(r, list);
    }

    TEST(unmarked_non_critical_are_as_selected_the_rest_non_critical, fix)
    {
      list.configure_importance("-apa/-ko,lemur");
      tag_result r[] =
      {
        { &apa, crpcut::tag::ignored },
        { &lemur, crpcut::tag::critical },
        { &ko, crpcut::tag::critical },
        { &longish, crpcut::tag::non_critical },
        { &list, crpcut::tag::critical }
      };
      verify_tags(r, list);
    }

    TEST(illegal_specification_throws, fix)
    {
      ASSERT_THROW(list.configure_importance("//"),
                   crpcut::tag_filter::spec_error&);
    }
  }
}






