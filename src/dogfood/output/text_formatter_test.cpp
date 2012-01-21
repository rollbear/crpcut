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
#include "../../output/text_modifier.hpp"
#include "stream_buffer_mock.hpp"
#include "tag_mocks.hpp"
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
    virtual ssize_t write(const char *data, std::size_t len)
    {
      os.write(data, std::streamsize(len));
      return ssize_t(len);
    }
    MOCK_CONST_METHOD0(is_empty, bool());
    std::ostringstream os;
  };

  class tag_list : public crpcut::tag_list_root
  {
  public:
    MOCK_METHOD0(fail, void());
    MOCK_METHOD0(pass, void());
    MOCK_CONST_METHOD0(num_failed, size_t());
    MOCK_CONST_METHOD0(num_passed, size_t());
    MOCK_CONST_METHOD0(get_name, crpcut::datatypes::fixed_string());
    MOCK_CONST_METHOD0(longest_tag_name, int());
    MOCK_METHOD1(set_importance, void(crpcut::tag::importance));
    MOCK_CONST_METHOD0(get_importance, crpcut::tag::importance());
  };

  class test_tag : public crpcut::tag
  {
  public:
    template <size_t N>
    test_tag(const char (&f)[N], tag_list *root)
      : crpcut::tag(N, root),
        name_(crpcut::datatypes::fixed_string::make(f))
    {
      EXPECT_CALL(*this, get_name()).WillOnce(Return(name_));
    }
    MOCK_METHOD0(fail, void());
    MOCK_METHOD0(pass, void());
    MOCK_CONST_METHOD0(num_failed, size_t());
    MOCK_CONST_METHOD0(num_passed, size_t());
    MOCK_CONST_METHOD0(get_name, crpcut::datatypes::fixed_string());
    MOCK_METHOD1(set_importance, void(crpcut::tag::importance));
    MOCK_CONST_METHOD0(get_importance, crpcut::tag::importance());
    crpcut::datatypes::fixed_string name_;
  };
  static crpcut::datatypes::fixed_string empty_string = { "", 0 };


  class fix
  {
  protected:
    fix()
    {
      EXPECT_CALL(tags, get_name()).WillRepeatedly(Return(empty_string));
      EXPECT_CALL(tags, longest_tag_name()).WillRepeatedly(Return(0));
    }
    StrictMock<mock::tag_list> tags;
  };

}

#define _ "[[:space:]]*"
#define s crpcut::datatypes::fixed_string::make


TESTSUITE(output)
{

  TESTSUITE(text_formatter)
  {
    static const char* vec[] = { "apa", "katt", "orm", 0 };

    TEST(construction_and_immediate_destruction_has_no_effect, fix)
    {
      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<mock::stream_buffer> test_buffer;
        {
          crpcut::output::text_formatter obj(test_buffer,
                                             "one",
                                             1,
                                             vec,
                                             tags,
                                             test_modifier);
        }
        ASSERT_TRUE(test_buffer.os.str() == "");
      }
    }

    TEST(stats_without_test_list_shows_only_pass_line_for_only_pass_results,
         fix)
    {
      EXPECT_CALL(tags, num_passed()).WillOnce(Return(13));
      EXPECT_CALL(tags, num_failed()).WillOnce(Return(0));
      EXPECT_CALL(tags, get_importance()).WillOnce(Return(crpcut::tag::critical));

      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<mock::stream_buffer> test_buffer;
        {
          crpcut::output::text_formatter obj(test_buffer,
                                             "one",
                                             1,
                                             vec,
                                             tags,
                                             test_modifier);
          obj.statistics(15, 14, 13, 0);
        }

        static const char re[] =
          "14 test cases selected\n\n"
              "Total"    _ ":" _ "Sum" _ "Critical" _ "Non-critical\n"
          "<PS>PASSED"   _ ":" _ "13"  _ "13"       _ "0<>\n"
          "<BS>UNTESTED" _ ":" _ "1<>\n$";

        ASSERT_PRED(crpcut::regex(re, crpcut::regex::m),
                    test_buffer.os.str());
      }
    }

    TEST(stats_without_test_list_shows_only_fail_line_for_only_fail_results,
         fix)
    {
      EXPECT_CALL(tags, num_passed()).WillOnce(Return(0));
      EXPECT_CALL(tags, num_failed()).WillOnce(Return(13));
      EXPECT_CALL(tags, get_importance()).WillOnce(Return(crpcut::tag::critical));

      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<mock::stream_buffer> test_buffer;
        {
          crpcut::output::text_formatter obj(test_buffer,
                                             "one",
                                             1,
                                             vec,
                                             tags,
                                             test_modifier);
          obj.statistics(15, 13, 13, 13);
        }
        static const char re[] =
              "13 test cases selected\n\n"
              "Total"    _ ":" _ "Sum" _ "Critical" _ "Non-critical\n"
          "<FS>FAILED"   _ ":" _ "13"  _ "13"       _ "0<>\n$";

        ASSERT_PRED(crpcut::regex(re, crpcut::regex::m),
                    test_buffer.os.str());
      }
    }

#define MAKE_TAG(n) mock::test_tag n(#n, &tags)

    TEST(stats_with_mixed_crit_ncrit_pass_only_shows_pass_sum,
         fix)
    {
      EXPECT_CALL(tags, longest_tag_name()).WillRepeatedly(Return(15));
      EXPECT_CALL(tags, num_passed()).WillOnce(Return(0));
      EXPECT_CALL(tags, num_failed()).WillOnce(Return(0));
      EXPECT_CALL(tags, get_importance())
        .WillOnce(Return(crpcut::tag::critical));

      MAKE_TAG(apa);
      EXPECT_CALL(apa, num_passed())
        .WillRepeatedly(Return(5));

      EXPECT_CALL(apa, num_failed())
        .WillRepeatedly(Return(0));

      EXPECT_CALL(apa, get_importance())
        .WillOnce(Return(crpcut::tag::critical));

      MAKE_TAG(katt);
      EXPECT_CALL(katt, num_passed())
        .WillRepeatedly(Return(3));

      EXPECT_CALL(katt, num_failed())
        .WillRepeatedly(Return(0));

      EXPECT_CALL(katt, get_importance())
        .WillOnce(Return(crpcut::tag::non_critical));

      MAKE_TAG(a_long_tag_name);
      EXPECT_CALL(a_long_tag_name, num_passed())
        .WillRepeatedly(Return(1));

      EXPECT_CALL(a_long_tag_name, num_failed())
        .WillRepeatedly(Return(0));

      EXPECT_CALL(a_long_tag_name, get_importance())
        .WillOnce(Return(crpcut::tag::non_critical));

      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<mock::stream_buffer> test_buffer;
        {
          crpcut::output::text_formatter obj(test_buffer,
                                             "one",
                                             1,
                                            vec,
                                             tags,
                                             test_modifier);
          obj.statistics(9, 9, 9, 0);
        }
        static const char re[] =
              "9 test cases selected\n"
              " tag                 run  passed  failed\n"
           "<P>!apa                   5       5       0<>\n"
          "<NP>?katt                  3       3       0<>\n"
          "<NP>?a_long_tag_name       1       1       0<>\n"
          "\n"
              "Total"    _ ":" _ "Sum" _ "Critical" _ "Non-critical\n"
          "<PS>PASSED"   _ ":" _ "9"   _ "5"        _ "4<>\n$"
          ;
        ASSERT_PRED(crpcut::regex(re, crpcut::regex::m),
                    test_buffer.os.str());
      }
    }

    TEST(stats_with_mixed_crit_ncrit_fail_only_shows_fail_sum, fix)
    {
      EXPECT_CALL(tags, longest_tag_name()).WillRepeatedly(Return(20));
      EXPECT_CALL(tags, num_passed()).WillOnce(Return(0));
      EXPECT_CALL(tags, num_failed()).WillOnce(Return(0));
      EXPECT_CALL(tags, get_importance())
        .WillOnce(Return(crpcut::tag::critical));

      MAKE_TAG(apa);
      EXPECT_CALL(apa, num_passed())
        .WillRepeatedly(Return(0));

      EXPECT_CALL(apa, num_failed())
        .WillRepeatedly(Return(5));

      EXPECT_CALL(apa, get_importance())
        .WillOnce(Return(crpcut::tag::critical));

      MAKE_TAG(katt);
      EXPECT_CALL(katt, num_passed())
        .WillRepeatedly(Return(0));

      EXPECT_CALL(katt, num_failed())
        .WillRepeatedly(Return(3));

      EXPECT_CALL(katt, get_importance())
        .WillOnce(Return(crpcut::tag::non_critical));

      MAKE_TAG(a_very_long_tag_name_);
      EXPECT_CALL(a_very_long_tag_name_, num_passed())
        .WillRepeatedly(Return(0));

      EXPECT_CALL(a_very_long_tag_name_, num_failed())
        .WillRepeatedly(Return(1));

      EXPECT_CALL(a_very_long_tag_name_, get_importance())
        .WillOnce(Return(crpcut::tag::critical));

      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<mock::stream_buffer> test_buffer;
        {
          crpcut::output::text_formatter obj(test_buffer,
                                             "one",
                                             1,
                                             vec,
                                             tags,
                                             test_modifier);
          obj.statistics(9, 9, 9, 9);
        }
        static const char re[] =
              "9 test cases selected\n"
              " tag                      run  passed  failed\n"
           "<F>!apa                        5       0       5<>\n"
          "<NF>?katt                       3       0       3<>\n"
           "<F>!a_very_long_tag_name_      1       0       1<>\n"
          "\n"
              "Total"    _ ":" _ "Sum" _ "Critical" _ "Non-critical\n"
          "<FS>FAILED"   _ ":" _ "9"   _ "6"        _ "3<>\n$"
          ;

        ASSERT_PRED(crpcut::regex(re, crpcut::regex::m),
                    test_buffer.os.str());
      }
    }

    TEST(stats_with_mixed_crit_ncrit_fail_pass_only_shows_crit_sum, fix)
    {
      EXPECT_CALL(tags, longest_tag_name()).WillRepeatedly(Return(10));

      EXPECT_CALL(tags, num_passed()).WillOnce(Return(0));
      EXPECT_CALL(tags, num_failed()).WillOnce(Return(0));
      EXPECT_CALL(tags, get_importance())
        .WillOnce(Return(crpcut::tag::critical));

      MAKE_TAG(apa);
      EXPECT_CALL(apa, num_passed())
        .WillRepeatedly(Return(2));

      EXPECT_CALL(apa, num_failed())
        .WillRepeatedly(Return(3));

      EXPECT_CALL(apa, get_importance())
        .WillOnce(Return(crpcut::tag::critical));

      MAKE_TAG(katt);
      EXPECT_CALL(katt, num_passed())
        .WillRepeatedly(Return(1));

      EXPECT_CALL(katt, num_failed())
        .WillRepeatedly(Return(2));

      EXPECT_CALL(katt, get_importance())
        .WillOnce(Return(crpcut::tag::non_critical));

      MAKE_TAG(a_tag_name);
      EXPECT_CALL(a_tag_name, num_passed())
        .WillRepeatedly(Return(1));

      EXPECT_CALL(a_tag_name, num_failed())
        .WillRepeatedly(Return(1));

      EXPECT_CALL(a_tag_name, get_importance())
        .WillOnce(Return(crpcut::tag::critical));

      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<mock::stream_buffer> test_buffer;
        {
          crpcut::output::text_formatter obj(test_buffer,
                                             "one",
                                             1,
                                             vec,
                                             tags,
                                             test_modifier);
          obj.statistics(10, 10, 10, 6);
        }
        static const char re[] =
              "10 test cases selected\n"
              " tag            run  passed  failed\n"
           "<F>!apa              5       2       3<>\n"
          "<NF>?katt             3       1       2<>\n"
           "<F>!a_tag_name       2       1       1<>\n"
          "\n"
              "Total"    _ ":" _ "Sum" _ "Critical" _ "Non-critical\n"
          "<PS>PASSED"   _ ":" _ "4"   _ "3"        _ "1<>\n"
          "<FS>FAILED"   _ ":" _ "6"   _ "4"        _ "2<>\n$"
          ;
        ASSERT_PRED(crpcut::regex(re, crpcut::regex::m),
                    test_buffer.os.str());
      }
    }

    TEST(stats_with_mixed_nconly_shows_nc_sum, fix)
    {
      EXPECT_CALL(tags, longest_tag_name()).WillRepeatedly(Return(20));

      EXPECT_CALL(tags, num_passed()).WillOnce(Return(0));
      EXPECT_CALL(tags, num_failed()).WillOnce(Return(0));
      EXPECT_CALL(tags, get_importance())
        .WillOnce(Return(crpcut::tag::critical));

      MAKE_TAG(apa);
      EXPECT_CALL(apa, num_passed())
        .WillRepeatedly(Return(2));

      EXPECT_CALL(apa, num_failed())
        .WillRepeatedly(Return(3));

      EXPECT_CALL(apa, get_importance())
        .WillOnce(Return(crpcut::tag::non_critical));

      MAKE_TAG(katt);
      EXPECT_CALL(katt, num_passed())
        .WillRepeatedly(Return(1));

      EXPECT_CALL(katt, num_failed())
        .WillRepeatedly(Return(2));

      EXPECT_CALL(katt, get_importance())
        .WillOnce(Return(crpcut::tag::non_critical));

      MAKE_TAG(a_very_long_tag_name);
      EXPECT_CALL(a_very_long_tag_name, num_passed())
        .WillRepeatedly(Return(1));

      EXPECT_CALL(a_very_long_tag_name, num_failed())
        .WillRepeatedly(Return(1));

      EXPECT_CALL(a_very_long_tag_name, get_importance())
        .WillOnce(Return(crpcut::tag::non_critical));

      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<mock::stream_buffer> test_buffer;
        {
          crpcut::output::text_formatter obj(test_buffer,
                                             "one",
                                             1,
                                             vec,
                                             tags,
                                             test_modifier);
          obj.statistics(10, 10, 10, 6);
        }
        static const char re[] =
              "10 test cases selected\n"
              " tag                      run  passed  failed\n"
          "<NF>?apa                        5       2       3<>\n"
          "<NF>?katt                       3       1       2<>\n"
          "<NF>?a_very_long_tag_name       2       1       1<>\n"
          "\n"
              "Total"    _ ":" _ "Sum" _ "Critical" _ "Non-critical\n"
          "<NPS>PASSED"   _ ":" _ "4"   _ "0"        _ "4<>\n"
          "<NFS>FAILED"   _ ":" _ "6"   _ "0"        _ "6<>\n$"
          ;
        ASSERT_PRED(crpcut::regex(re, crpcut::regex::m),
                    test_buffer.os.str());
      }
    }


    TEST(tags_that_are_not_run_are_not_displayed_in_tag_result_summary, fix)
    {
      EXPECT_CALL(tags, longest_tag_name()).WillRepeatedly(Return(20));

      EXPECT_CALL(tags, num_passed()).WillOnce(Return(0));
      EXPECT_CALL(tags, num_failed()).WillOnce(Return(0));
      EXPECT_CALL(tags, get_importance())
        .WillOnce(Return(crpcut::tag::critical));

      MAKE_TAG(apa);
      EXPECT_CALL(apa, num_passed())
        .WillRepeatedly(Return(2));

      EXPECT_CALL(apa, num_failed())
        .WillRepeatedly(Return(3));

      EXPECT_CALL(apa, get_importance())
        .WillOnce(Return(crpcut::tag::non_critical));

      MAKE_TAG(katt);
      EXPECT_CALL(katt, num_passed())
        .WillRepeatedly(Return(1));

      EXPECT_CALL(katt, num_failed())
        .WillRepeatedly(Return(2));

      EXPECT_CALL(katt, get_importance())
        .WillOnce(Return(crpcut::tag::non_critical));

      MAKE_TAG(a_very_long_tag_name);
      EXPECT_CALL(a_very_long_tag_name, num_passed())
        .WillRepeatedly(Return(0));

      EXPECT_CALL(a_very_long_tag_name, num_failed())
        .WillRepeatedly(Return(0));

      EXPECT_CALL(a_very_long_tag_name, get_importance())
        .WillOnce(Return(crpcut::tag::non_critical));

      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<mock::stream_buffer> test_buffer;
        {
          crpcut::output::text_formatter obj(test_buffer,
                                             "one",
                                             1,
                                             vec,
                                             tags,
                                             test_modifier);
          obj.statistics(8, 8, 8, 5);
        }
        static const char re[] =
              "8 test cases selected\n"
              " tag                      run  passed  failed\n"
          "<NF>?apa                        5       2       3<>\n"
          "<NF>?katt                       3       1       2<>\n"
          "\n"
              "Total"    _ ":" _ "Sum" _ "Critical" _ "Non-critical\n"
          "<NPS>PASSED"   _ ":" _ "3"   _ "0"        _ "3<>\n"
          "<NFS>FAILED"   _ ":" _ "5"   _ "0"        _ "5<>\n$"
          ;
        ASSERT_PRED(crpcut::regex(re, crpcut::regex::m),
                    test_buffer.os.str());
      }
    }


    TEST(passed_critical_has_correct_decoration, fix)
    {
      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<mock::stream_buffer> test_buffer;
        crpcut::output::text_formatter obj(test_buffer,
                                           "one",
                                           1,
                                           vec,
                                           tags,
                                           test_modifier);
        obj.begin_case(s("apa"), true, true);
        obj.end_case();
        ASSERT_PRED(crpcut::regex("^<P>PASSED!: apa\n<>=*\n"),
                    test_buffer.os.str());
      }
    }

    TEST(passed_noncritical_has_correct_decoration, fix)
    {
      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<mock::stream_buffer> test_buffer;
        crpcut::output::text_formatter obj(test_buffer,
                                           "one",
                                           1,
                                           vec,
                                           tags,
                                           test_modifier);
        obj.begin_case(s("apa"), true, false);
        obj.end_case();
        ASSERT_PRED(crpcut::regex("^<NP>PASSED?: apa\n<>=*\n"),
                    test_buffer.os.str());
      }
    }

    TEST(failed_critical_has_correct_decoration, fix)
    {
      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<mock::stream_buffer> test_buffer;
        crpcut::output::text_formatter obj(test_buffer,
                                           "one",
                                           1,
                                           vec,
                                           tags,
                                           test_modifier);
        obj.begin_case(s("apa"), false, true);
        obj.end_case();
        ASSERT_PRED(crpcut::regex("^<F>FAILED!: apa\n<>=*\n"),
                    test_buffer.os.str());
      }
    }

    TEST(failed_noncritical_has_correct_decoration, fix)
    {
      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<mock::stream_buffer> test_buffer;
        crpcut::output::text_formatter obj(test_buffer,
                                           "one",
                                           1,
                                           vec,
                                           tags,
                                           test_modifier);
        obj.begin_case(s("apa"), false, false);
        obj.end_case();
        ASSERT_PRED(crpcut::regex("^<NF>FAILED?: apa\n<>=*\n"),
                    test_buffer.os.str());
      }
    }

    TEST(terminate_without_files_with_correct_phase_and_format, fix)
    {
      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<mock::stream_buffer> test_buffer;
        crpcut::output::text_formatter obj(test_buffer,
                                           "one",
                                           1,
                                           vec,
                                           tags,
                                           test_modifier);
        obj.begin_case(s("apa"), false, false);
        obj.terminate(crpcut::destroying, s("katt"), s(""));
        obj.end_case();
        const char re[] =
          "^<NF>FAILED?: apa\n"
          "phase=\"destroying\"" _ "-*\n"
          "katt\n"
          "-*\n"
          "<>=*\n"
          ;
        ASSERT_PRED(crpcut::regex(re, crpcut::regex::m), test_buffer.os.str());
      }
    }

    TEST(terminate_with_files_with_correct_phase_and_format, fix)
    {
      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<mock::stream_buffer> test_buffer;
        crpcut::output::text_formatter obj(test_buffer,
                                           "one",
                                           1,
                                           vec,
                                           tags,
                                           test_modifier);
        obj.begin_case(s("apa"), false, false);
        obj.terminate(crpcut::creating, s("katt"), s("/tmp/tmpfile"));
        obj.end_case();
        const char re[] =
          "^<NF>FAILED?: apa\n"
          "/tmp/tmpfile is not empty!\n"
          "phase=\"creating\"" _ "-*\n"
          "katt\n"
          "-*\n"
          "<>=*\n"
          ;
        ASSERT_PRED(crpcut::regex(re, crpcut::regex::m), test_buffer.os.str());
      }
    }

    TEST(multiple_prints_are_shown_in_sequence, fix)
    {
      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<mock::stream_buffer> test_buffer;
        crpcut::output::text_formatter obj(test_buffer,
                                           "one",
                                           1,
                                           vec,
                                           tags,
                                           test_modifier);
        obj.begin_case(s("apa"), false, false);
        obj.print(s("type"),
                  s("the quick brown fox jumps over the lazy dog"));
        obj.print(s("morse"),
                  s("jag missade hissen\ni mississippi"));
        obj.terminate(crpcut::creating, s("katt"), s(""));
        obj.end_case();
        const char re[] =
          "^<NF>FAILED?: apa\n"
          "type-*\n"
          "the quick brown fox jumps over the lazy dog\n"
          "-*\n"
          "morse-*\n"
          "jag missade hissen\n"
          "i mississippi\n"
          "-*\n"
          "phase=\"creating\"" _ "-*\n"
          "katt\n"
          "-*\n"
          "<>=*\n"
          ;
        ASSERT_PRED(crpcut::regex(re, crpcut::regex::m),
                    test_buffer.os.str());
      }
    }

    TEST(full_report_with_non_empty_dir, fix)
    {
      EXPECT_CALL(tags, longest_tag_name()).WillOnce(Return(0));
      EXPECT_CALL(tags, get_importance())
        .WillRepeatedly(Return(crpcut::tag::critical));
      EXPECT_CALL(tags, num_passed())
        .WillRepeatedly(Return(29));
      EXPECT_CALL(tags, num_failed())
        .WillRepeatedly(Return(1));
      ASSERT_SCOPE_HEAP_LEAK_FREE
      {
        StrictMock<mock::stream_buffer> test_buffer;
        crpcut::output::text_formatter obj(test_buffer,
                                           "one",
                                           1,
                                           vec,
                                           tags,
                                           test_modifier);
        obj.begin_case(s("apa"), false, false);
        obj.terminate(crpcut::creating, s("katt"), s("/tmp/tmpdir/hoppla"));
        obj.end_case();

        obj.nonempty_dir("/tmp/tmpdir");
        obj.blocked_test(s("ko"));
        obj.blocked_test(s("tupp"));
        obj.statistics(32, 32, 30, 1);
        const char re[] =
          "^<NF>FAILED?: apa\n"
          "/tmp/tmpdir/hoppla is not empty!\n"
          "phase=\"creating\"" _ "-*\n"
          "katt\n"
          "-*\n"
          "<>=*\n"
          "Files remain under /tmp/tmpdir\n"
          "The following tests were blocked from running:\n"
          "  <B>ko<>\n"
          "  <B>tupp<>\n"
          "32 test cases selected\n\n"
          "Total"      _   ":" _ "Sum" _ "Critical" _ "Non-critical\n"
          "<PS>PASSED" _   ":" _  "29" _       "29" _            "0<>\n"
          "<FS>FAILED" _   ":" _   "1" _        "1" _            "0<>\n"
          "<BS>UNTESTED" _ ":" _ "2<>\n"
          ;
        ASSERT_PRED(crpcut::regex(re, crpcut::regex::m), test_buffer.os.str());
      }
    }
  }
}
