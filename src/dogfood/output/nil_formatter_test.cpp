/*
 * Copyright 2012-2013 Bjorn Fahller <bjorn@fahller.se>
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
#include "../../output/nil_formatter.hpp"
#include "stream_buffer_mock.hpp"
#include "tag_mocks.hpp"

#define _ "[[:space:]]*"
#define s crpcut::datatypes::fixed_string::make


TESTSUITE(output)
{

  TESTSUITE(nil_formatter)
  {
    TEST(never_outputs_anything)
    {
      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        static const char* vec[] = { "apa", "katt", "orm", nullptr };
        mock::tag_list tags;
        mock::stream_buffer buffer;
        {
          crpcut::output::nil_formatter f(buffer,
                                          "test id",
                                          vec,
                                          tags,
                                          100,
                                          10);
          f.begin_case("tupp::lemur", true, true, 100);
          f.print(s("info"), s("apa"), s("apa.cpp\n"));
          f.terminate(crpcut::running, s("failed"), s("apa.cpp"), std::string());
          f.end_case();
          f.begin_case("ko::orm", true, true, 100);
          f.terminate(crpcut::destroying, s("ouch"), s("apa.cpp"), "apa");
          f.end_case();
          f.nonempty_dir("/ttt");
          f.blocked_test(crpcut::tag::critical, "n");
          f.blocked_test(crpcut::tag::non_critical, "o");
          f.statistics(5,2);
        }
        ASSERT_TRUE(buffer.os.str() == "");
      }
    }
  }
}
