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
#include "../../output/xml_formatter.hpp"
#include "stream_buffer_mock.hpp"
#include "tag_mocks.hpp"
namespace {
  using namespace testing;

  crpcut::datatypes::fixed_string empty_string = { "", 0 };

  class fix
  {
  protected:
    fix()
    {
      EXPECT_CALL(tags, get_importance()).
        WillRepeatedly(Return(crpcut::tag::critical));
      EXPECT_CALL(tags, get_name()).
        WillRepeatedly(Return(empty_string));
      EXPECT_CALL(tags, longest_tag_name()).
        WillRepeatedly(Return(0));
    }
    StrictMock<mock::tag_list> tags;
  };
}

#define s(...) crpcut::datatypes::fixed_string::make(#__VA_ARGS__)

#define _ "[[:space:]]*"
#define S "[[:space:]]+"

#define XML_HEADER \
          "<\\?xml version=\"1\\.0\"\\?>" _                  \
          "<crpcut" S                                        \
          "xmlns:xsi" _ "=" _                                \
          "\"http://www.w3.org/2001/XMLSchema-instance\"" S  \
          "xsi:noNamespaceSchemaLocation" _ "=" _            \
          "\"http://crpcut.sourceforge.net/crpcut-"          \
          CRPCUT_VERSION_STRING ".xsd\"" S                   \
          "starttime" _ "=" _ "\"[0-9T:-]+Z\"" S             \
          "host" _ "=" _ "\"[[:alnum:].]+\"" S               \
          "command" _ "=" _ "\"apa katt orm\"" S                      \
          "id" _ "=" _ "\"one\"" _ ">"

#define XML_TRAILER _ "</crpcut>"
#define XML_DATA_FIELD(name, data) "<" #name ">" #data "</" #name ">"

#define XML_STATISTICS(registered, selected, blocked, run, failed, failed_nc) \
  _ "<statistics>"                                                      \
  _ XML_DATA_FIELD(registered_test_cases, registered)                   \
  _ XML_DATA_FIELD(selected_test_cases, selected)                       \
  _ XML_DATA_FIELD(untested_test_cases, blocked)                        \
  _ XML_DATA_FIELD(run_test_cases, run)                                 \
  _ XML_DATA_FIELD(failed_test_cases, failed)                           \
  _ XML_DATA_FIELD(failed_non_critical_test_cases, failed_nc)           \
  _ "</statistics>"

#define ARG_COUNT(...) GET_ARG_COUNT(__VA_ARGS__, 10,9,8,7,6,5,4,3,2,1,0)
#define GET_ARG_COUNT(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,N,...) N

#define XML_FIELD_TAG(name, tag, value) _ "<" #name S #tag _ "=" _ "\"" #value "\"" _ "/>"

#define XML_FIELD_TAG_PAIR(name, t1, v1, t2, v2) _ "<"  #name \
                                                    S #t1 _ "=" _ "\"" #v1 "\""\
                                                    S #t2 _ "=" _ "\"" #v2 "\"" _ "/>"
#define XML_REPEAT_TAG_0(name, tag) "<" #name "/>"
#define XML_REPEAT_TAG_1(name, tag, _1) XML_FIELD_TAG(name, tag, _1)
#define XML_REPEAT_TAG_2(name, tag, _1, _2)                    \
  XML_FIELD_TAG(name, tag, _1)                                 \
  XML_FIELD_TAG(name, tag, _2)
#define XML_REPEAT_TAG_3(name, tag, _1, _2, _3)                  \
  XML_FIELD_TAG(name, tag, _1)                                   \
  XML_FIELD_TAG(name, tag, _2)                                   \
  XML_FIELD_TAG(name, tag, _3)
#define XML_REPEAT_TAG_4(name, tag, _1, _2, _3, _4)                \
  XML_FIELD_TAG(name, tag, _1)                                     \
  XML_FIELD_TAG(name, tag, _2)                                     \
  XML_FIELD_TAG(name, tag, _3)                                     \
  XML_FIELD_TAG(name, tag, _4)
#define XML_REPEAT_TAG_5(name, tag, _1, _2, _3, _4, _5)            \
  XML_FIELD_TAG(name, tag, _1)                                     \
  XML_FIELD_TAG(name, tag, _2)                                     \
  XML_FIELD_TAG(name, tag, _3)                                     \
  XML_FIELD_TAG(name, tag, _4)                                     \
  XML_FIELD_TAG(name, tag, _5)

#define XML_REPEAT_TAG_6(name, tag, _1, _2, _3, _4, _5, _6)       \
  XML_FIELD_TAG(name, tag, _1)                                    \
  XML_FIELD_TAG(name, tag, _2)                                    \
  XML_FIELD_TAG(name, tag, _3)                                    \
  XML_FIELD_TAG(name, tag, _4)                                    \
  XML_FIELD_TAG(name, tag, _5)                                    \
  XML_FIELD_TAG(name, tag, _6)

#define XML_REPEAT_TAG_7(name, tag, _1, _2, _3, _4, _5, _6, _7)      \
  XML_FIELD_TAG(name, tag, _1)                                       \
  XML_FIELD_TAG(name, tag, _2)                                       \
  XML_FIELD_TAG(name, tag, _3)                                       \
  XML_FIELD_TAG(name, tag, _4)                                       \
  XML_FIELD_TAG(name, tag, _5)                                       \
  XML_FIELD_TAG(name, tag, _6)                                       \
  XML_FIELD_TAG(name, tag, _7)

#define XML_REPEAT_TAG_8(name, tag, _1, _2, _3, _4, _5, _6, _7, _8)     \
  XML_FIELD_TAG(name, tag, _1)                                           \
  XML_FIELD_TAG(name, tag, _2)                                           \
  XML_FIELD_TAG(name, tag, _3)                                           \
  XML_FIELD_TAG(name, tag, _4)                                           \
  XML_FIELD_TAG(name, tag, _5)                                           \
  XML_FIELD_TAG(name, tag, _6)                                           \
  XML_FIELD_TAG(name, tag, _7)                                           \
  XML_FIELD_TAG(name, tag, _8)

#define XML_REPEAT_TAG_9(name, tag, _1, _2, _3, _4, _5, _6, _7, _8, _9) \
  XML_FIELD_TAG(name, tag, _1)                                          \
  XML_FIELD_TAG(name, tag, _2)                                          \
  XML_FIELD_TAG(name, tag, _3)                                          \
  XML_FIELD_TAG(name, tag, _4)                                          \
  XML_FIELD_TAG(name, tag, _5)                                          \
  XML_FIELD_TAG(name, tag, _6)                                          \
  XML_FIELD_TAG(name, tag, _7)                                          \
  XML_FIELD_TAG(name, tag, _8)                                          \
  XML_FIELD_TAG(name, tag, _9)

#define XML_REPEAT_TAG_PAIR_2(name, t1, t2, _1, _2) \
  XML_FIELD_TAG_PAIR(name, t1, _1, t2, _2)

#define XML_REPEAT_TAG_PAIR_4(name, t1, t2, _1, _2, _3, _4) \
  XML_FIELD_TAG_PAIR(name, t1, _1, t2, _2)                  \
  XML_FIELD_TAG_PAIR(name, t1, _3, t2, _4)

#define XML_REPEAT_TAG_PAIR_6(name, t1, t2, _1, _2, _3, _4, _5, _6) \
  XML_FIELD_TAG_PAIR(name, t1, _1, t2, _2)                          \
  XML_FIELD_TAG_PAIR(name, t1, _3, t2, _4)                          \
  XML_FIELD_TAG_PAIR(name, t1, _5, t2, _6)

#define XML_REPEAT_TAG_PAIR_8(name, t1, t2, _1, _2, _3, _4, _5, _6, _7, _8) \
  XML_FIELD_TAG_PAIR(name, t1, _1, t2, _2)                                  \
  XML_FIELD_TAG_PAIR(name, t1, _3, t2, _4)                                  \
  XML_FIELD_TAG_PAIR(name, t1, _5, t2, _6)                                  \
  XML_FIELD_TAG_PAIR(name, t1, _7, t2, _8)

#define CALL_CONCAT_(name, num, ...) name ## num (__VA_ARGS__)
#define CALL_CONCAT(name, num, ...) CALL_CONCAT_(name, num, __VA_ARGS__)

#define XML_REPEAT_TAG(name, tag, ...)           \
  CALL_CONCAT(XML_REPEAT_TAG_,                  \
              ARG_COUNT(__VA_ARGS__),           \
              name,                             \
              tag,                              \
              __VA_ARGS__)

#define XML_REPEAT_TAG_PAIR(name, t1, t2, ...) \
  CALL_CONCAT(XML_REPEAT_TAG_PAIR_,            \
              ARG_COUNT(__VA_ARGS__),          \
              name, t1, t2,                    \
              __VA_ARGS__)

#define XML_BLOCKED_LIST(...)                                \
  _ "<blocked_tests>"                                        \
  _ XML_REPEAT_TAG_PAIR(test, name, importance, __VA_ARGS__) \
  _ "</blocked_tests>"
TESTSUITE(output)
{
  TESTSUITE(xml_formatter)
  {
    static const char *vec[] = { "apa", "katt", "orm", 0 };
    using namespace testing;

    TEST(construction_and_immediate_destruction_yields_header_only,
        fix)
    {
      StrictMock<mock::stream_buffer> test_buffer;
      {
        crpcut::output::xml_formatter obj(test_buffer,
                                          "one",
                                          vec,
                                          tags,
                                          1,1);

      }
      static const char re[] =
        XML_HEADER
        ;

      ASSERT_PRED(crpcut::match<crpcut::regex>(re,
                                               crpcut::regex::e,
                                               crpcut::regex::m),
                  test_buffer.os.str());
    }

    TEST(construction_and_statistics_yields_minimal_complete_report,
         fix)
    {
      StrictMock<mock::stream_buffer> test_buffer;
      {
        crpcut::output::xml_formatter obj(test_buffer,
                                          "one",
                                          vec,
                                          tags,
                                          0,0);
        obj.statistics(0,0);
      }
      static const char re[] =
        XML_HEADER
        XML_STATISTICS(0,0,0,0,0,0)
        XML_TRAILER
        ;
      ASSERT_PRED(crpcut::match<crpcut::regex>(re,
                                               crpcut::regex::e,
                                               crpcut::regex::m),
                  test_buffer.os.str());
    }


    TEST(minimal_complete_report,
         fix)
    {
      StrictMock<mock::stream_buffer> test_buffer;
      crpcut::output::xml_formatter obj(test_buffer,
                                        "one",
                                        vec,
                                        tags,
                                        0,0);
      obj.statistics(0,0);
      static const char re[] =
        XML_HEADER
        XML_STATISTICS(0,0,0,0,0,0)
        XML_TRAILER
        ;
      ASSERT_PRED(crpcut::match<crpcut::regex>(re,
                                               crpcut::regex::e,
                                               crpcut::regex::m),
                  test_buffer.os.str());
    }

    TEST(report_with_blocked_tests_list, fix)
    {
      StrictMock<mock::stream_buffer> test_buffer;
      {
        crpcut::output::xml_formatter obj(test_buffer,
                                          "one",
                                          vec,
                                          tags,
                                          3,3);
        obj.blocked_test(crpcut::tag::critical, s(apa));
        obj.blocked_test(crpcut::tag::non_critical, s(katt));
        obj.blocked_test(crpcut::tag::disabled, s(ko));
        obj.statistics(0,0);
      }
      static const char re[] =
        XML_HEADER
        XML_BLOCKED_LIST(apa, critical, katt, non_critical, ko, disabled)
        XML_STATISTICS(3,3,3,0,0,0)
        XML_TRAILER
        ;

        INFO << re;
      ASSERT_PRED(crpcut::match<crpcut::regex>(re,
                                               crpcut::regex::e,
                                               crpcut::regex::m),
                  test_buffer.os.str());
    }

#define MAKE_TAG(n, obj) mock::test_tag n(#n, &obj)
#define XML_TAG_ENTRY(name, p, f, c) \
    _ "<tag" S "name" _ "=" _ "\"" #name "\""                           \
      S "passed" _ "=" _ "\"" #p "\""                                   \
      S "failed" _ "=" _ "\"" #f "\""                                   \
      S "critical" _ "=" _ "\"" #c  "\"" _ "/>"

    TEST(report_with_tags_list, fix)
    {
      EXPECT_CALL(tags, get_importance()).
        WillRepeatedly(Return(crpcut::tag::critical));

      MAKE_TAG(apa, tags);
      EXPECT_CALL(apa, num_passed()).
        WillRepeatedly(Return(1));
      EXPECT_CALL(apa, num_failed()).
        WillRepeatedly(Return(3));
      EXPECT_CALL(apa, get_importance()).
        WillRepeatedly(Return(crpcut::tag::critical));

      MAKE_TAG(katt, tags);
      EXPECT_CALL(katt, num_passed()).
        WillRepeatedly(Return(2));
      EXPECT_CALL(katt, num_failed()).
        WillRepeatedly(Return(3));
      EXPECT_CALL(katt, get_importance()).
        WillRepeatedly(Return(crpcut::tag::non_critical));

      StrictMock<mock::stream_buffer> test_buffer;
      {
        crpcut::output::xml_formatter obj(test_buffer,
                                          "one",
                                          vec,
                                          tags,
                                          9,9);
        obj.statistics(9,6);
      }
      static const char re[] =
        XML_HEADER
        _ "<tag_summary>"
        XML_TAG_ENTRY(apa,1,3,true)
        XML_TAG_ENTRY(katt,2,3,false)
        _ "</tag_summary>"
        XML_STATISTICS(9,9,0,9,6,3)
        XML_TRAILER
        ;

        INFO << re;
      ASSERT_PRED(crpcut::match<crpcut::regex>(re,
                                               crpcut::regex::e,
                                               crpcut::regex::m),
                  test_buffer.os.str());
    }

#define XML_TEST(name, critical, result) \
    _ "<" _ "test" S "name" _ "=" _ "\"" #name "\"" \
      S "critical" _ "=" _ "\"" #critical "\""      \
      S "result" _ "=" _ "\"" #result "\""
#define XML_CLOSED_TEST(name, critical, result) \
    XML_TEST(name, critical, result) _ "/>"

#define XML_OPEN_TEST(name, critical, result) \
    XML_TEST(name, critical, result) _ ">"

    TEST(report_with_blocked_tests_and_tag_summary, fix)
    {
      EXPECT_CALL(tags, get_importance()).
        WillRepeatedly(Return(crpcut::tag::critical));

      MAKE_TAG(apa, tags);
      EXPECT_CALL(apa, num_passed()).
        WillRepeatedly(Return(1));
      EXPECT_CALL(apa, num_failed()).
        WillRepeatedly(Return(3));
      EXPECT_CALL(apa, get_importance()).
        WillRepeatedly(Return(crpcut::tag::critical));

      MAKE_TAG(katt, tags);
      EXPECT_CALL(katt, num_passed()).
        WillRepeatedly(Return(2));
      EXPECT_CALL(katt, num_failed()).
        WillRepeatedly(Return(3));
      EXPECT_CALL(katt, get_importance()).
        WillRepeatedly(Return(crpcut::tag::non_critical));

      StrictMock<mock::stream_buffer> test_buffer;
      {
        using crpcut::datatypes::fixed_string;
        crpcut::output::xml_formatter obj(test_buffer,
                                          "one",
                                          vec,
                                          tags,
                                          12,12);

        obj.begin_case(s(tupp), true, true);
        obj.end_case();
        obj.begin_case(s(lemur), false, true);
        obj.print(s(stderr), s(ehepp));
        obj.terminate(crpcut::running,
                      fixed_string::make("Died on signal 6\n"
                                         "Expected normal exit"),
                      s());
        obj.end_case();
        obj.blocked_test(crpcut::tag::critical, s(apa));
        obj.blocked_test(crpcut::tag::non_critical, s(katt));
        obj.blocked_test(crpcut::tag::disabled, s(ko));
        obj.statistics(9,6);
      }
      static const char re[] =
        XML_HEADER
        XML_CLOSED_TEST(tupp, true, PASSED)
        XML_OPEN_TEST(lemur, true, FAILED)
        _ "<log>"
        _ XML_DATA_FIELD(stderr, ehepp)
        _ "<violation" S "phase" _ "=" _ "\"running\"" _ ">Died on signal 6\n"
        "Expected normal exit</violation>"
        _ "</log>"
        _ "</test>"

        XML_BLOCKED_LIST(apa, critical, katt, non_critical, ko, disabled)
        _ "<tag_summary>"
        XML_TAG_ENTRY(apa,1,3,true)
        XML_TAG_ENTRY(katt,2,3,false)
        _ "</tag_summary>"
        XML_STATISTICS(12,12,3,9,6,3)
        XML_TRAILER
        ;

        INFO << re;
      ASSERT_PRED(crpcut::match<crpcut::regex>(re,
                                               crpcut::regex::e,
                                               crpcut::regex::m),
                  test_buffer.os.str());
    }

    TEST(only_lt_gt_amp_apos_quot_are_escaped, fix)
    {
      StrictMock<mock::stream_buffer> test_buffer;
      {
        crpcut::output::xml_formatter obj(test_buffer,
                                          "one",
                                          vec,
                                          tags,
                                          0,0);
        static const char msg[] = "\t\n\r!\"#$%&'()*+,-./0123456789"
                                  ":;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                  "[\\]^_` abcdefghijklmnopqrstuvwxyz"
                                  "{|}~";
        obj.begin_case(s(tupp), true, true);
        obj.print(s(info),
                  crpcut::datatypes::fixed_string::make(msg, sizeof(msg) - 1));
        obj.end_case();
        obj.statistics(0,0);
      }
      static const char re[] =
        XML_HEADER
        XML_OPEN_TEST(tupp, true, PASSED)
        _ "<log>"
        _ "<info>"
        "\t\n\r!&quot;#\\$%&amp;&apos;\\(\\)\\*\\+,-\\./0123456789"
        ":;&lt;=&gt;\\?@ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "\\[\\\\]\\^_` abcdefghijklmnopqrstuvwxyz"
        "\\{\\|\\}~"
        "</info>"
        _ "</log>"
        _ "</test>"
        XML_STATISTICS(0,0,0,0,0,0)
        XML_TRAILER
        ;
      INFO << re;
      ASSERT_PRED(crpcut::match<crpcut::regex>(re,
                                               crpcut::regex::e,
                                               crpcut::regex::m),
                  test_buffer.os.str());

    }

    TEST(illegal_characters_are_replaced, fix)
    {
      StrictMock<mock::stream_buffer> test_buffer;
      {
        crpcut::output::xml_formatter obj(test_buffer,
                                          "one",
                                          vec,
                                          tags,
                                          0,0);
        char msg[256];
        for (size_t i = 0; i < sizeof(msg); ++i)
          {
            msg[i] = char(i);
          }
        obj.begin_case(s(tupp), true, true);
        obj.print(s(info),
                  crpcut::datatypes::fixed_string::make(msg));
        obj.end_case();
        obj.statistics(0,0);
      }

#define IL "&#xfffd;"

      static const char re[] =
        XML_HEADER
        XML_OPEN_TEST(tupp, true, PASSED)
        _ "<log>"
        _ "<info>"
        IL IL IL IL IL IL IL IL IL "\t\n" IL IL "\r" IL IL IL IL IL IL IL IL IL
        IL IL IL IL IL IL IL IL IL
        " !&quot;#\\$%&amp;&apos;\\(\\)\\*\\+,-\\./0123456789"
        ":;&lt;=&gt;\\?@ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "\\[\\\\]\\^_`abcdefghijklmnopqrstuvwxyz"
        "\\{\\|\\}~" "&#x7f;"
        IL IL IL IL IL IL IL IL IL IL IL IL IL IL IL IL
        IL IL IL IL IL IL IL IL IL IL IL IL IL IL IL IL
        IL IL IL IL IL IL IL IL IL IL IL IL IL IL IL IL
        IL IL IL IL IL IL IL IL IL IL IL IL IL IL IL IL
        IL IL IL IL IL IL IL IL IL IL IL IL IL IL IL IL
        IL IL IL IL IL IL IL IL IL IL IL IL IL IL IL IL
        IL IL IL IL IL IL IL IL IL IL IL IL IL IL IL IL
        IL IL IL IL IL IL IL IL IL IL IL IL IL IL IL
        "</info>"
        _ "</log>"
        _ "</test>"
        XML_STATISTICS(0,0,0,0,0,0)
        XML_TRAILER
        ;
      INFO << re;
      ASSERT_PRED(crpcut::match<crpcut::regex>(re,
                                               crpcut::regex::e,
                                               crpcut::regex::m),
                  test_buffer.os.str());


    }

    TEST(non_empty_directory_is_displayed, fix)
    {
      StrictMock<mock::stream_buffer> test_buffer;
      {
        crpcut::output::xml_formatter obj(test_buffer,
                                          "one",
                                          vec,
                                          tags,
                                          0,0);
        obj.begin_case(s(tupp), false, true);
        obj.terminate(crpcut::post_mortem,
                      s(),
                      s(/tmp/crpcut02342/tests::tupp));
        obj.end_case();
        obj.nonempty_dir("/tmp/crpcut02342");
        obj.statistics(0,0);
      }

#define IL "&#xfffd;"

      static const char re[] =
        XML_HEADER
        XML_OPEN_TEST(tupp, true, FAILED)
        _ "<log>"
        _ "<violation" S "phase" _ "=" _ "\"post_mortem\""
        S "nonempty_dir" _ "=" _ "\"/tmp/crpcut02342/tests::tupp\"" _ "/>"
        _ "</log>"
        _ "</test>"
        _ "<remaining_files" S "nonempty_dir" _ "=" _ "\"/tmp/crpcut02342\"" _ "/>"
        XML_STATISTICS(0,0,0,0,0,0)
        XML_TRAILER
        ;
      INFO << re;
      ASSERT_PRED(crpcut::match<crpcut::regex>(re,
                                               crpcut::regex::e,
                                               crpcut::regex::m),
                  test_buffer.os.str());

    }
  }
}
