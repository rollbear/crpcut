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

#include "../../cli/param.hpp"
#include <crpcut.hpp>

TESTSUITE(cli)
{
  TESTSUITE(param)
  {
    struct check
    {
      check()
      : root(),
        param('c', "check",
            "a toy parameter without values with the express purpose of testing"
            "\nvarious relatively basic aspects of command line parameter"
            "\nparsing",
            root)
      {
      }
      crpcut::cli::param_list root;
      crpcut::cli::param      param;
    };

    TEST(empty_list_does_not_match, check)
    {
      static const char *cli[] = { 0 };
      const char *const *p = param.match(cli);
      ASSERT_FALSE(p);
    }

    TEST(non_flag_list_does_not_match, check)
    {
      static const char *cli[] = { "apa" };
      const char *const *p = param.match(cli);
      ASSERT_FALSE(p);
    }

    TEST(short_form_without_trailing_params_matches, check)
    {
      static const char *cli[] = { "-c", 0 };
      const char * const *p = param.match(cli);
      ASSERT_TRUE(p == cli + 1);
    }

    TEST(long_form_without_trailing_param_matches, check)
    {
      static const char *cli[] = { "--check", 0 };
      const char *const *p = param.match(cli);
      ASSERT_TRUE(p == cli + 1);
    }

    TEST(short_form_with_extra_char_does_not_match, check)
    {
      static const char *cli[] = { "-cc", 0 };
      const char *const *p = param.match(cli);
      ASSERT_FALSE(p);
    }

    TEST(lone_dash_does_not_match, check)
    {
      static const char *cli[] = { "-", 0 };
      const char *const *p = param.match(cli);
      ASSERT_FALSE(p);
    }

    TEST(lone_double_dash_does_not_match, check)
    {
      static const char *cli[] = { "--", 0 };
      const char *const *p = param.match(cli);
      ASSERT_FALSE(p);
    }

    TEST(long_form_but_one_does_not_match, check)
    {
      static const char *cli[] = { "--chec", 0 };
      const char *const *p = param.match(cli);
      ASSERT_FALSE(p);
    }

    TEST(long_form_plus_one_does_not_match, check)
    {
      static const char *cli[] = { "--checkq", 0 };
      const char *const *p = param.match(cli);
      ASSERT_FALSE(p);
    }

    TEST(short_form_with_trailing_param_matches_without_consumption, check)
    {
      static const char *cli[] = { "-c", "-q", 0 };
      const char * const *p = param.match(cli);
      ASSERT_TRUE(p == cli + 1);
    }

    TEST(long_form_with_trailing_param_matches_without_consumption, check)
    {
      static const char *cli[] = { "--check", "--quiet", 0 };
      const char *const *p = param.match(cli);
      ASSERT_TRUE(p == cli + 1);
    }

    TEST(short_form_with_trailing_data_matches_without_consumption, check)
    {
      static const char *cli[] = { "-c", "0", 0 };
      const char *const *p = param.match(cli);
      ASSERT_TRUE(p == cli + 1);
    }

    TEST(long_form_with_naked_assign_throws, check)
    {
      static const char *cli[] = { "--check=", 0 };
      ASSERT_THROW(param.match(cli), crpcut::cli::param::exception,
                   "-c / --check does not accept any value");
    }

    TEST(long_form_with_value_throws, check)
    {
      static const char *cli[] = { "--check=3", 0 };
      ASSERT_THROW(param.match(cli), crpcut::cli::param::exception,
                   "-c / --check does not accept any value");
    }

    TEST(description_of_value_less_parameter_has_correct_format, check)
    {
      std::ostringstream os;
      os << param;
      static const char pattern[] =
          "   -c / --check\n"
          "        a toy parameter without values with the express purpose of testing"
          "\n        various relatively basic aspects of command line parameter"
          "\n        parsing";

      ASSERT_TRUE(os.str() == pattern);
    }

    TEST(description_of_a_parameter_with_value_has_correct_format)
    {
      crpcut::cli::param_list list;
      using P = crpcut::cli::param;
      P param('v', "verbose",
              "level", crpcut::cli::param::mandatory,
              "Another toy parameter intended to show how values are added to the"
              "\nsyntax description of the parameter in the list",
              list);
      std::ostringstream os;
      os << param;
      static const char pattern[] =
          "   -v level / --verbose=level\n"
          "        Another toy parameter intended to show how values are added to the"
          "\n        syntax description of the parameter in the list";
      ASSERT_TRUE(os.str() == pattern);
    }

    TEST(parameter_with_0_short_form_matches_long_form)
    {
      crpcut::cli::param_list list;
      using P = crpcut::cli::param;
      P param(0, "long", "a toy value just to test the idea",
              list);
      static const char *cli[] = { "--long", 0 };
      static char const * const *p = param.match(cli);
      ASSERT_TRUE(p == cli + 1);
    }

    TEST(parameter_with_0_short_form_does_not_match_single_dash)
    {
      crpcut::cli::param_list list;
      using P = crpcut::cli::param;
      P param(0, "long", "a toy value just to test the idea",
              list);
      static const char *cli[] = { "-", 0 };
      const char *const *p = param.match(cli);
      ASSERT_FALSE(p);
    }

    TEST(parameter_with_0_short_form_describes_only_long_form)
    {
      crpcut::cli::param_list list;
      using P = crpcut::cli::param;
      P param(0, "long", "a toy value just to test the idea",
              list);
      std::ostringstream os;
      os << param;
      static const char pattern[] =
        "   --long\n"
        "        a toy value just to test the idea";
      ASSERT_TRUE(os.str() == pattern);
    }
  }
}





