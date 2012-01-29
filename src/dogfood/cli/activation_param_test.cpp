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


#include "../../cli/activation_param.hpp"
#include <crpcut.hpp>

TESTSUITE(cli)
{
  TESTSUITE(activation_param)
  {
    struct fix
    {
      fix()
      : param('v', "verbose",
              "verbose mode, print results from passed tests")
      {
      }
      crpcut::cli::activation_param param;
    };
    TEST(non_matched_parameter_is_false, fix)
    {
      ASSERT_FALSE(param);
    }

    TEST(mismatched_parameter_is_false, fix)
    {
      static const char *cli[] = { "--debug", 0 };
      const char * const *p = param.match(cli);
      ASSERT_FALSE(p);
      ASSERT_FALSE(param);
    }

    TEST(matched_parameter_is_true, fix)
    {
      static const char *cli[] = { "--verbose", 0 };
      const char *const *p = param.match(cli);
      ASSERT_TRUE(p == cli + 1);
      ASSERT_TRUE(param);
    }

    TEST(double_matched_paratemer_throws, fix)
    {
      static const char *cli[] = { "--verbose", "-v", 0 };
      const char *const *p = param.match(cli);
      ASSERT_THROW(param.match(p), crpcut::cli::param::exception,
                   "-v / --verbose can only be used once");
    }

  }

}






