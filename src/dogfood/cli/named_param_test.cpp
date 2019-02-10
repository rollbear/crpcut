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

#include "../../cli/named_param.hpp"
#include <crpcut.hpp>

TESTSUITE(cli)
{
  TESTSUITE(param) {}
  TESTSUITE(named_param, DEPENDS_ON(ALL_TESTS(param)))
  {
    struct fix
    {
      fix()
      : root(),
        param('p', "parameter",
              "well, a bit of this and that",
              root)
      {
      }
      crpcut::cli::param_list  root;
      crpcut::cli::named_param param;
    };

#define ID "-p name=value / --parameter=name=value"
    TEST(long_form_syntax_validation_passes_correct_syntax, fix)
    {
      static const char *cli[] = { "--parameter=name=value", nullptr };
      const char *const *p = param.match(cli);
      ASSERT_TRUE(p == cli + 1);
    }

    TEST(long_form_syntax_validation_throws_with_missing_assign, fix)
    {
      static const char *cli[] = { "--parameter", nullptr };
      ASSERT_THROW(param.match(cli), crpcut::cli::param::exception&,
                   ID " missing a parameter name and a value");
    }

    TEST(long_form_syntax_validation_throws_with_missing_name, fix)
    {
      static const char *cli[] = { "--parameter=", nullptr };
      ASSERT_THROW(param.match(cli), crpcut::cli::param::exception&,
                   ID " missing a parameter name and a value");
    }

    TEST(long_form_syntax_validation_throws_with_missing_assign_after_name, fix)
    {
      static const char *cli[] = { "--parameter=apa", nullptr };
      ASSERT_THROW(param.match(cli), crpcut::cli::param::exception&,
                   ID " missing a value after \"apa\"");
    }

    TEST(long_form_syntax_validation_throws_with_missing_value, fix)
    {
      static const char *cli[] = { "--parameter=apa=", nullptr };
      ASSERT_THROW(param.match(cli), crpcut::cli::param::exception&,
                   ID " missing a value after \"apa\"");
    }

    TEST(short_form_syntax_validation_passes_correct_syntax, fix)
    {
      static const char *cli[] = { "-p", "name=value", nullptr };
      const char *const *p = param.match(cli);
      ASSERT_TRUE(p == cli + 2);
    }

    TEST(short_form_syntax_validation_throws_with_missing_name, fix)
    {
      static const char *cli[] = { "-p", nullptr };
      ASSERT_THROW(param.match(cli), crpcut::cli::param::exception&,
                   ID " missing a parameter name and a value");
    }

    TEST(short_form_syntax_validation_throws_with_missing_assign_after_name, fix)
    {
      static const char *cli[] = { "-p", "apa", nullptr };
      ASSERT_THROW(param.match(cli), crpcut::cli::param::exception&,
                   ID " missing a value after \"apa\"");
    }

    TEST(short_form_syntax_validation_throws_with_missing_value, fix)
    {
      static const char *cli[] = { "-p", "apa=", nullptr };
      ASSERT_THROW(param.match(cli), crpcut::cli::param::exception&,
                   ID " missing a value after \"apa\"");
    }

    TEST(long_form_lone_value_is_found, fix)
    {
      static const char *cli[] = { "-x", "--apa=katt", "--parameter=name=value" };
      const char *p = param.value_for(cli, "name");
      ASSERT_TRUE(p == cli[2] + 17);
    }

    TEST(long_form_correct_value_from_many_is_found, fix)
    {
      static const char *cli[] = { "--parameter=apa=katt",
                                   "--parameter=katt=orm",
                                   "--parameter=name=value",
                                   "--parameter=orm=ko",
                                   nullptr };
      const char *p = param.value_for(cli, "name");
      ASSERT_TRUE(p == cli[2] + 17);
      p = param.value_for(cli, "katt");
      ASSERT_TRUE(p == cli[1] + 17);
      p = param.value_for(cli, "apa");
      ASSERT_TRUE(p == cli[0] + 16);
      p = param.value_for(cli, "orm");
      ASSERT_TRUE(p == cli[3] + 16);
      p = param.value_for(cli, "lemur");
      ASSERT_FALSE(p);
    }

    TEST(short_form_lone_value_is_found, fix)
    {
      static const char *cli[] = { "-x", "--apa=katt", "-p", "name=value" };
      const char *p = param.value_for(cli, "name");
      ASSERT_TRUE(p == cli[3] + 5);
    }

    TEST(short_form_correct_value_from_many_is_found, fix)
    {
      static const char *cli[] = { "-p", "apa=katt",
                                   "-p", "katt=orm",
                                   "-p", "name=value",
                                   "-p", "orm=ko",
                                   nullptr };
      const char *p = param.value_for(cli, "name");
      ASSERT_TRUE(p == cli[5] + 5);
      p = param.value_for(cli, "katt");
      ASSERT_TRUE(p == cli[3] + 5);
      p = param.value_for(cli, "apa");
      ASSERT_TRUE(p == cli[1] + 4);
      p = param.value_for(cli, "orm");
      ASSERT_TRUE(p == cli[7] + 4);
      p = param.value_for(cli, "lemur");
      ASSERT_FALSE(p);
    }
  }
}







