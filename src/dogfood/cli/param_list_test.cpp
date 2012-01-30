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
#include "../../cli/param.hpp"
#include "../../cli/activation_param.hpp"
#include "../../cli/boolean_flip.hpp"
#include "../../cli/named_param.hpp"
#include "../../cli/value_param.hpp"

TESTSUITE(cli)
{
  TESTSUITE(param_list)
  {
    struct fix
    {
      fix()
      : list(),
        verbose('v', "verbose", "show results from passed tests", list),
        single_shot('s', "single-shot", "run one test without forking", list),
        xml('x', "xml", "control XML mode", list),
        named('p', "param", "set a named parameter", list),
        children('c', "children", "int", "number of test processes in paralll", list),
        working_directory('d', "directory", "path", "where to run", list)
      {
      }
      crpcut::cli::param_list                list;
      crpcut::cli::activation_param          verbose;
      crpcut::cli::activation_param          single_shot;
      crpcut::cli::boolean_flip              xml;
      crpcut::cli::named_param               named;
      crpcut::cli::value_param<int>          children;
      crpcut::cli::value_param<const char *> working_directory;
    };

    TEST(empty_argv_matches_nothing, fix)
    {
      static const char *argv[] = { 0 };
      const char *const *p = list.match_all(argv);
      ASSERT_TRUE(p == argv);
      ASSERT_FALSE(verbose);
      ASSERT_FALSE(single_shot);
      ASSERT_TRUE(xml.get_value(true));
      ASSERT_FALSE(xml.get_value(false));
      ASSERT_FALSE(children);
      ASSERT_FALSE(working_directory);
    }

    TEST(argv_without_parameters_matches_nothing, fix)
    {
      static const char *argv[] = { "apa", "katt", "ko", 0 };
      const char *const *p = list.match_all(argv);
      ASSERT_TRUE(p == argv);
      ASSERT_FALSE(verbose);
      ASSERT_FALSE(single_shot);
      ASSERT_TRUE(xml.get_value(true));
      ASSERT_FALSE(xml.get_value(false));
      ASSERT_FALSE(children);
      ASSERT_FALSE(working_directory);
    }

    TEST(value_for_all_params_matches_with_tail_returned, fix)
    {
      static const char *argv[] = { "-x", "true", "--verbose", "-s",
                                    "--param=apa=katt", "--children=8",
                                    "-d", "/tmp",
                                    "apa", "katt", "ko", 0 };
      const char *const *p = list.match_all(argv);
      ASSERT_TRUE(p == argv + 8);
      ASSERT_TRUE(xml.get_value(true));
      ASSERT_TRUE(xml.get_value(false));
      ASSERT_TRUE(verbose);
      ASSERT_TRUE(single_shot);
      ASSERT_TRUE(children);
      ASSERT_TRUE(children.get_value() == 8);
      ASSERT_TRUE(working_directory);
      ASSERT_TRUE(std::string("/tmp") == working_directory.get_value());
      ASSERT_TRUE(std::string("katt") == named.value_for(argv, "apa"));
    }

    TEST(faulty_named_param_in_argv_causes_exception, fix)
    {
      static const char *argv[] = { "--xml",
                                    "--single-shot",
                                    "--param=apa",
                                    "-d",
                                    "/tmp" };
      ASSERT_THROW(list.match_all(argv), crpcut::cli::param::exception,
                   "-p name=value / --param=name=value missing a value after \"apa\"");
    }
  }
}
