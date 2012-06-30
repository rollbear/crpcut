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
#include <sstream>

namespace {
  class test_list : public crpcut::tag_list_root
  {
    virtual crpcut::datatypes::fixed_string get_name() const
    {
      abort();
      return crpcut::datatypes::fixed_string::make("");
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

}
TESTSUITE(tag_list)
{
  TEST(print_of_empty_tag_list_outputs_nothing)
  {
    std::ostringstream os;
    {
      test_list list;
      list.print_to(os);
    }
    ASSERT_TRUE(os.str() == "");
  }

  TEST(print_of_populated_tags_prints_them_by_line_in_order_of_registration)
  {
    std::ostringstream os;
    {
      test_list list;
      test_tag apa("apa", &list);
      test_tag lemur("lemur", &list);
      test_tag ko("ko", &list);
      list.print_to(os);
    }
    static const char expected[] =
        "apa\n"
        "lemur\n"
        "ko\n";
    ASSERT_TRUE(os.str() == expected);
  }
}






