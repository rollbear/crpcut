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

#include "../../cli/boolean_flip.hpp"
#include <crpcut.hpp>



TESTSUITE(cli)
{
  TESTSUITE(param) {}

  TESTSUITE(boolean_flip, DEPENDS_ON(ALL_TESTS(cli::param)))
  {
     struct fix
     {
       fix()
       : param('x', "xml", "{boolean value}",
               "Test parameter used to check the functionality of optional\n"
               "boolean parameter")
       {
       }
       crpcut::cli::boolean_flip param;
     };

#define ID "-x {boolean value} / --xml={boolean value}"
     TEST(uninitialized_parameter_mirrors_truth_value, fix)
     {
       ASSERT_TRUE(param.get_value(true));
       ASSERT_FALSE(param.get_value(false));
     }

     TEST(short_form_without_value_flips_without_consuming, fix)
     {
       static const char *cli[] = { "-x", 0 };
       const char *const *p = param.match(cli);
       ASSERT_TRUE(p == cli + 1);
       ASSERT_FALSE(param.get_value(true));
       ASSERT_TRUE(param.get_value(false));
       p = param.match(cli);
       ASSERT_TRUE(p == cli + 1);
       ASSERT_TRUE(param.get_value(true));
       ASSERT_FALSE(param.get_value(false));
     }

     TEST(long_form_without_value_flips, fix)
     {
       static const char *cli[] = { "--xml", 0 };
       const char *const *p = param.match(cli);
       ASSERT_TRUE(p == cli + 1);
       ASSERT_FALSE(param.get_value(true));
       ASSERT_TRUE(param.get_value(false));
       p = param.match(cli);
       ASSERT_TRUE(p == cli + 1);
       ASSERT_TRUE(param.get_value(true));
       ASSERT_FALSE(param.get_value(false));
     }

     TEST(long_form_with_truth_value_sets_fixed_true, fix)
     {
       static const char *cli[] = { "--xml=Yes", 0 };
       const char *const *p = param.match(cli);
       ASSERT_TRUE(p == cli + 1);
       ASSERT_TRUE(param.get_value(true));
       ASSERT_TRUE(param.get_value(false));
     }

     TEST(long_form_with_false_value_sets_fixed_false, fix)
     {
       static const char *cli[] = { "--xml=off", 0 };
       const char *const *p = param.match(cli);
       ASSERT_TRUE(p == cli + 1);
       ASSERT_FALSE(param.get_value(true));
       ASSERT_FALSE(param.get_value(false));
     }

     TEST(long_form_with_illegal_value_throws, fix)
     {
       static const char *cli[] = { "--xml=active", 0 };
       ASSERT_THROW(param.match(cli), crpcut::cli::param::exception,
                    ID " - can't interpret \"active\" as boolean value");
     }

     TEST(long_form_without_value_after_fixed_true_throws, fix)
     {
       static const char *cli[] = { "--xml=On", "--xml", 0 };
       const char *const *p = param.match(cli);
       ASSERT_THROW(param.match(p), crpcut::cli::param::exception,
                    ID " is already set");
     }

     TEST(long_form_without_value_after_fixed_false_throws, fix)
     {
       static const char *cli[] = { "--xml=False", "--xml", 0 };
       const char *const *p = param.match(cli);
       ASSERT_THROW(param.match(p), crpcut::cli::param::exception,
                    ID " is already set");
     }

     TEST(long_form_with_true_value_overrides_flip, fix)
     {
       static const char *cli[] = { "--xml", "--xml=true", 0 };
       const char * const *p = param.match(cli);
       p = param.match(p);
       ASSERT_TRUE(p == cli + 2);
       ASSERT_TRUE(param.get_value(true));
       ASSERT_TRUE(param.get_value(false));
     }

     TEST(short_form_without_value_flips, fix)
     {
       static const char *cli[] = { "-x", 0 };
       const char *const *p = param.match(cli);
       ASSERT_TRUE(p == cli + 1);
       ASSERT_FALSE(param.get_value(true));
       ASSERT_TRUE(param.get_value(false));
       p = param.match(cli);
       ASSERT_TRUE(p == cli + 1);
       ASSERT_TRUE(param.get_value(true));
       ASSERT_FALSE(param.get_value(false));
     }

     TEST(short_form_with_illegal_value_flips_without_consuming, fix)
     {
       static const char *cli[] = { "-x", "-y", 0 };
       const char *const *p = param.match(cli);
       ASSERT_TRUE(p == cli + 1);
       ASSERT_FALSE(param.get_value(true));
       ASSERT_TRUE(param.get_value(false));
       p = param.match(cli);
       ASSERT_TRUE(p == cli + 1);
       ASSERT_TRUE(param.get_value(true));
       ASSERT_FALSE(param.get_value(false));
     }

     TEST(short_form_with_true_value_sets_fixed_true, fix)
     {
       static const char *cli[] = { "-x", "yes", 0 };
       const char *const *p = param.match(cli);
       ASSERT_TRUE(p == cli + 2);
       ASSERT_TRUE(param.get_value(true));
       ASSERT_TRUE(param.get_value(false));
     }

     TEST(short_form_with_false_value_sets_fixed_false, fix)
     {
       static const char *cli[] = { "-x", "OFF", 0 };
       const char *const *p = param.match(cli);
       ASSERT_TRUE(p == cli + 2);
       ASSERT_FALSE(param.get_value(true));
       ASSERT_FALSE(param.get_value(false));
     }

     TEST(short_form_with_false_value_overrides_flip, fix)
     {
       static const char *cli[] = { "-x", "-x", "false", 0 };
       const char *const *p = param.match(cli);
       ASSERT_TRUE(p == cli + 1);
       p = param.match(p);
       ASSERT_TRUE(p == cli + 3);
       ASSERT_FALSE(param.get_value(true));
       ASSERT_FALSE(param.get_value(false));
     }

     TEST(short_form_with_true_value_overrides_flip, fix)
     {
       static const char *cli[] = { "-x", "-x", "YES", 0 };
       const char *const *p = param.match(cli);
       ASSERT_TRUE(p == cli + 1);
       p = param.match(p);
       ASSERT_TRUE(p == cli + 3);
       ASSERT_TRUE(param.get_value(true));
       ASSERT_TRUE(param.get_value(false));
     }
  }
}







