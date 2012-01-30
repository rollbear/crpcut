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

#include "../../cli/value_param.hpp"
#include <crpcut.hpp>

#define ID "-i \"id value\" / --identity=\"id value\""

TESTSUITE(cli)
{
  TESTSUITE(param) {}
  TESTSUITE(value_param, DEPENDS_ON(ALL_TESTS(cli::param)))
  {
    template <typename T>
    struct fix
    {
      fix()
      : root(),
        param('i', "identity",
              "\"id value\"",
              "nonsense parameter used to test that values are interpreted\n"
              "correctly",
              root)
      {
      }
      crpcut::cli::param_list     root;
      crpcut::cli::value_param<T> param;
    };

    TEST(long_form_without_value_throws, fix<int>)
    {
      static const char * cli[] = { "--identity", 0 };
      ASSERT_THROW(param.match(cli), crpcut::cli::param::exception,
                    ID " expects a value");
    }

    TEST(long_form_with_naked_assign_throws, fix<int>)
    {
      static const char *cli[] = { "--identity=", 0 };
      ASSERT_THROW(param.match(cli), crpcut::cli::param::exception,
                   ID " expects a value");
    }

    TEST(long_form_with_wrong_value_type_throws, fix<int>)
    {
      static const char *cli[] = { "--identity=apa", 0 };
      ASSERT_THROW(param.match(cli), crpcut::cli::param::exception,
                   ID " - can't interpret \"apa\"");
    }

    TEST(long_form_with_tail_after_value_throws, fix<int>)
    {
      static const char *cli[] = { "--identity=3z", 0 };
      ASSERT_THROW(param.match(cli), crpcut::cli::param::exception,
                   ID " - can't interpret \"3z\"");
    }

    TEST(unmatched_parameter_is_inactive, fix<int>)
    {
      static const char *cli[] = { "--xpr", 0 };
      const char *const * p = param.match(cli);
      ASSERT_FALSE(p);
      ASSERT_FALSE(param);
    }

    TEST(unmatched_string_parameter_is_inactive, fix<const char *>)
    {
      static const char *cli[] = { "--xpr", 0 };
      const char *const * p = param.match(cli);
      ASSERT_FALSE(p);
      ASSERT_FALSE(param);
    }

    TEST(matched_long_form_parameter_is_active, fix<int>)
    {
      static const char *cli[] = { "--identity=385", 0 };
      const char *const *p = param.match(cli);
      ASSERT_TRUE(p == cli + 1);
      ASSERT_TRUE(param);
      ASSERT_TRUE(param.get_value() == 385);
    }

    TEST(matched_long_form_string_parameter_keeps_pointer, fix<const char*>)
    {
      static const char *cli[] = { "--identity=\"en liten katt\"", 0 };
      const char *const *p = param.match(cli);
      ASSERT_TRUE(p == cli + 1);
      ASSERT_TRUE(param);
      ASSERT_TRUE(param.get_value() == cli[0]+11);
    }


    TEST(short_form_without_value_throws, fix<int>)
    {
      static const char * cli[] = { "-i", 0 };
      ASSERT_THROW(param.match(cli), crpcut::cli::param::exception,
                    ID " expects a value");
    }

    TEST(short_form_with_following_param_assign_throws, fix<int>)
    {
      static const char *cli[] = { "-i", "-p", 0 };
      ASSERT_THROW(param.match(cli), crpcut::cli::param::exception,
                   ID " - can't interpret \"-p\"");
    }

    TEST(short_form_with_wrong_value_type_throws, fix<int>)
    {
      static const char *cli[] = { "-i", "apa", 0 };
      ASSERT_THROW(param.match(cli), crpcut::cli::param::exception,
                   ID " - can't interpret \"apa\"");
    }

    TEST(short_form_with_tail_after_value_throws, fix<int>)
    {
      static const char *cli[] = { "-i", "3z", 0 };
      ASSERT_THROW(param.match(cli), crpcut::cli::param::exception,
                   ID " - can't interpret \"3z\"");
    }


    TEST(matched_short_form_parameter_is_active, fix<int>)
    {
      static const char *cli[] = { "-i", "385", 0 };
      const char *const *p = param.match(cli);
      ASSERT_TRUE(p == cli + 2);
      ASSERT_TRUE(param);
      ASSERT_TRUE(param.get_value() == 385);
    }

    TEST(matched_short_form_string_parameter_keeps_pointer, fix<const char*>)
    {
      static const char *cli[] = { "-i", "\"en liten katt\"", 0 };
      const char *const *p = param.match(cli);
      ASSERT_TRUE(p == cli + 2);
      ASSERT_TRUE(param);
      ASSERT_TRUE(param.get_value() == cli[1]);
    }


  }
}







