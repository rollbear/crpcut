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

#include <gmock/gmock.h>
#include <crpcut.hpp>
#include "../../output/text_formatter.hpp"
#include "../../output/buffer.hpp"
#include "../../output/text_modifier.hpp"
#include <sstream>

namespace {
  static const char rules[] =
    "|"
    "<>|"
    "PASSED=<P>|"
    "FAILED=<F>|"
    "NCFAILED=<NF>|"
    "NCPASSED=<NP>|"
    "BLOCKED=<B>|"
    "PASSED_SUM=<PS>|"
    "FAILED_SUM=<FS>|"
    "NCPASSED_SUM=<NPS>|"
    "NCFAILED_SUM=<NFS>|"
    "BLOCKED_SUM=<BS>|";
  crpcut::output::text_modifier test_modifier(rules);
  crpcut::output::text_modifier empty_modifier(0);

  using namespace testing;

  struct stream_buffer : public crpcut::output::buffer
  {
  public:
    typedef std::pair<const char*, std::size_t> buff;
    MOCK_CONST_METHOD0(get_buffer, buff());
    MOCK_METHOD0(advance, void());
    virtual ssize_t write(const char *buff, std::size_t len)
    {
      os.write(buff, len);
      return len;
    }
    MOCK_CONST_METHOD0(is_empty, bool());
    std::ostringstream os;
  };

  class fix
  {
  protected:
    StrictMock<stream_buffer> test_buffer;
  };

  template <std::size_t N>
  inline crpcut::datatypes::fixed_string s(const char (&f)[N])
  {
    crpcut::datatypes::fixed_string rv = { f, N - 1 };
    return rv;
  }
}

#define S "[[:space:]]*"


TESTSUITE(output)
{

  TESTSUITE(text_formatter)
  {
    static const char* vec[] = { "apa", "katt", "orm", 0 };

    TEST(construction_and_immediate_destruction_has_no_effect, fix)
    {
      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        {
          crpcut::output::text_formatter obj(test_buffer,
                                             "one",
                                             1,
                                             vec,
                                             test_modifier);
        }
        ASSERT_TRUE(test_buffer.os.str() == "");
      }
    }

    TEST(stats_without_test_list_shows_only_pass_line_for_only_pass_results)
    {
      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<stream_buffer> test_buffer;
        {
          crpcut::output::text_formatter obj(test_buffer,
                                             "one",
                                             1,
                                             vec,
                                             test_modifier);
          obj.tag_summary(s(""), 13, 0, true);
          obj.statistics(15, 14, 13, 0);
        }

        static const char re[] =
          "14 test cases selected\n\n"
              "Total"    S ":" S "Sum" S "Critical" S "Non-critical\n"
          "<PS>PASSED"   S ":" S "13"  S "13"       S "0<>\n"
          "<BS>UNTESTED" S ":" S "1<>\n$";

        ASSERT_PRED(crpcut::regex(re, crpcut::regex::m),
                    test_buffer.os.str());
      }
    }

    TEST(stats_without_test_list_shows_only_fail_line_for_only_fail_results)
    {
      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<stream_buffer> test_buffer;
        {
          crpcut::output::text_formatter obj(test_buffer,
                                             "one",
                                             1,
                                             vec,
                                             test_modifier);
          obj.tag_summary(s(""), 0, 13, true);
          obj.statistics(15, 13, 13, 13);
        }
        static const char re[] =
              "13 test cases selected\n\n"
              "Total"    S ":" S "Sum" S "Critical" S "Non-critical\n"
          "<FS>FAILED"   S ":" S "13"  S "13"       S "0<>\n$";

        ASSERT_PRED(crpcut::regex(re, crpcut::regex::m),
                    test_buffer.os.str());
      }
    }

    TEST(stats_with_mixed_crit_ncrit_pass_only_shows_pass_sum)
    {
      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<stream_buffer> test_buffer;
        {
          crpcut::output::text_formatter obj(test_buffer,
                                             "one",
                                             1,
                                             vec,
                                             test_modifier);
          obj.tag_summary(s(""), 0, 0, true);
          obj.tag_summary(s("katt"), 3, 0, false);
          obj.tag_summary(s("apa"), 5, 0, true);
          obj.statistics(8, 8, 8, 0);
        }
        static const char re[] =
              "8 test cases selected\n"
               " tag" S "run" S "passed" S "failed\n"
           "<P>!apa"  S "5"   S "5"      S "0<>\n"
          "<NP>?katt" S "3"   S "3"      S "0<>\n\n"
              "Total"    S ":" S "Sum" S "Critical" S "Non-critical\n"
          "<PS>PASSED"   S ":" S "8"   S "5"        S "3<>\n$"
          ;
        ASSERT_PRED(crpcut::regex(re, crpcut::regex::m),
                    test_buffer.os.str());
      }
    }

    TEST(stats_with_mixed_crit_ncrit_fail_only_shows_fail_sum)
    {
      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<stream_buffer> test_buffer;
        {
          crpcut::output::text_formatter obj(test_buffer,
                                             "one",
                                             1,
                                             vec,
                                             test_modifier);
          obj.tag_summary(s(""), 0, 0, true);
          obj.tag_summary(s("katt"), 0, 3, false);
          obj.tag_summary(s("apa"), 0, 5, true);
          obj.statistics(8, 8, 8, 8);
        }
        static const char re[] =
              "8 test cases selected\n"
               " tag" S "run" S "passed" S "failed\n"
           "<F>!apa"  S "5"   S "0"      S "5<>\n"
          "<NF>?katt" S "3"   S "0"      S "3<>\n\n"
              "Total"    S ":" S "Sum" S "Critical" S "Non-critical\n"
          "<FS>FAILED"   S ":" S "8"   S "5"        S "3<>\n$"
          ;
        ASSERT_PRED(crpcut::regex(re, crpcut::regex::m),
                    test_buffer.os.str());
      }
    }

    TEST(stats_with_mixed_crit_ncrit_fail_pass_only_shows_crit_sum)
    {
      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<stream_buffer> test_buffer;
        {
          crpcut::output::text_formatter obj(test_buffer,
                                             "one",
                                             1,
                                             vec,
                                             test_modifier);
          obj.tag_summary(s(""), 0, 0, true);
          obj.tag_summary(s("katt"), 1, 2, false);
          obj.tag_summary(s("apa"), 2, 3, true);
          obj.statistics(8, 8, 8, 5);
        }
        static const char re[] =
              "8 test cases selected\n"
               " tag" S "run" S "passed" S "failed\n"
           "<F>!apa"  S "5"   S "2"      S "3<>\n"
          "<NF>?katt" S "3"   S "1"      S "2<>\n\n"
              "Total"    S ":" S "Sum" S "Critical" S "Non-critical\n"
          "<PS>PASSED"   S ":" S "3"   S "2"        S "1<>\n"
          "<FS>FAILED"   S ":" S "5"   S "3"        S "2<>\n$"
          ;
        ASSERT_PRED(crpcut::regex(re, crpcut::regex::m),
                    test_buffer.os.str());
      }
    }

    TEST(stats_with_mixed_nconly_shows_nc_sum)
    {
      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<stream_buffer> test_buffer;
        {
          crpcut::output::text_formatter obj(test_buffer,
                                             "one",
                                             1,
                                             vec,
                                             test_modifier);
          obj.tag_summary(s(""), 0, 0, true);
          obj.tag_summary(s("katt"), 1, 2, false);
          obj.tag_summary(s("apa"), 2, 3, false);
          obj.statistics(8, 8, 8, 5);
        }
        static const char re[] =
              "8 test cases selected\n"
               " tag" S "run" S "passed" S "failed\n"
          "<NF>?apa"  S "5"   S "2"      S "3<>\n"
          "<NF>?katt" S "3"   S "1"      S "2<>\n\n"
              "Total"    S ":" S "Sum" S "Critical" S "Non-critical\n"
          "<NPS>PASSED"   S ":" S "3"   S "0"        S "3<>\n"
          "<NFS>FAILED"   S ":" S "5"   S "0"        S "5<>\n$"
          ;
        ASSERT_PRED(crpcut::regex(re, crpcut::regex::m),
                    test_buffer.os.str());
      }
    }

    TEST(passed_critical_has_correct_decoration)
    {
      StrictMock<stream_buffer> test_buffer;
      {
        crpcut::output::text_formatter obj(test_buffer,
                                           "one",
                                           1,
                                           vec,
                                           test_modifier);
        obj.begin_case(s("apa"), true, true);
        obj.end_case();
        ASSERT_PRED(crpcut::regex("^<P>PASSED!: apa\n<>=*\n"),
                    test_buffer.os.str());
      }
    }

    TEST(passed_noncritical_has_correct_decoration)
    {
      StrictMock<stream_buffer> test_buffer;
      {
        crpcut::output::text_formatter obj(test_buffer,
                                           "one",
                                           1,
                                           vec,
                                           test_modifier);
        obj.begin_case(s("apa"), true, false);
        obj.end_case();
        ASSERT_PRED(crpcut::regex("^<NP>PASSED?: apa\n<>=*\n"),
                    test_buffer.os.str());
      }
    }

    TEST(failed_critical_has_correct_decoration)
    {
      StrictMock<stream_buffer> test_buffer;
      {
        crpcut::output::text_formatter obj(test_buffer,
                                           "one",
                                           1,
                                           vec,
                                           test_modifier);
        obj.begin_case(s("apa"), false, true);
        obj.end_case();
        ASSERT_PRED(crpcut::regex("^<F>FAILED!: apa\n<>=*\n"),
                    test_buffer.os.str());
      }
    }

    TEST(failed_noncritical_has_correct_decoration)
    {
      StrictMock<stream_buffer> test_buffer;
      {
        crpcut::output::text_formatter obj(test_buffer,
                                           "one",
                                           1,
                                           vec,
                                           test_modifier);
        obj.begin_case(s("apa"), false, false);
        obj.end_case();
        ASSERT_PRED(crpcut::regex("^<NF>FAILED?: apa\n<>=*\n"),
                    test_buffer.os.str());
      }
    }
  }
}
