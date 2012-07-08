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
#include "../../output/nil_formatter.hpp"
#include "stream_buffer_mock.hpp"
#include "tag_mocks.hpp"
namespace {

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
}

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
        static const char* vec[] = { "apa", "katt", "orm", 0 };
        StrictMock<mock::tag_list> tags;
        StrictMock<mock::stream_buffer> buffer;
        {
          crpcut::output::nil_formatter f(buffer,
                                          "test id",
                                          vec,
                                          tags,
                                          100,
                                          10);
          f.begin_case("tupp::lemur", true, true, 100);
          f.print(s("info"), s("apa"));
          f.terminate(crpcut::running, s("failed"), std::string());
          f.end_case();
          f.begin_case("ko::orm", true, true, 100);
          f.terminate(crpcut::destroying, s("ouch"), "apa");
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
