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
#include "text_modifier.hpp"
#include <crpcut.hpp>
#include <sstream>
#include "output_writer.hpp"
#include "output_buffer.hpp"


TESTSUITE(text_modifier)
{
  class test_buffer : public crpcut::output::buffer
  {
  public:
    typedef std::pair<const char*, std::size_t> buff;
    MOCK_CONST_METHOD0(get_buffer, buff());
    MOCK_METHOD0(advance, void());
    MOCK_METHOD2(write, ssize_t(const char*, std::size_t));
    MOCK_CONST_METHOD0(is_empty, bool());
  };
  using crpcut::output::text_modifier;
  TEST(nullstring_does_nothing)
  {
    text_modifier obj(0);
    std::ostringstream os;
    obj.write_to(os, text_modifier::NORMAL);
    ASSERT_TRUE(os.str() == "");
    obj.write_to(os, text_modifier::PASSED);
    ASSERT_TRUE(os.str() == "");
    obj.write_to(os, text_modifier::FAILED);
    ASSERT_TRUE(os.str() == "");
    obj.write_to(os, text_modifier::NCFAILED);
    ASSERT_TRUE(os.str() == "");
    obj.write_to(os, text_modifier::NCPASSED);
    ASSERT_TRUE(os.str() == "");
    obj.write_to(os, text_modifier::BLOCKED);
    ASSERT_TRUE(os.str() == "");
    obj.write_to(os, text_modifier::PASSED_SUM);
    ASSERT_TRUE(os.str() == "");
    obj.write_to(os, text_modifier::FAILED_SUM);
    ASSERT_TRUE(os.str() == "");
    obj.write_to(os, text_modifier::NCPASSED_SUM);
    ASSERT_TRUE(os.str() == "");
    obj.write_to(os, text_modifier::NCFAILED_SUM);
    ASSERT_TRUE(os.str() == "");
    obj.write_to(os, text_modifier::BLOCKED_SUM);
    ASSERT_TRUE(os.str() == "");

    test_buffer buff;
    using testing::_;
    EXPECT_CALL(buff, write(_,_))
      .Times(0);
    crpcut::output::writer w(buff, "UTF-8", "--illegal--");
    obj.write_to(w, text_modifier::NORMAL);
    obj.write_to(w, text_modifier::PASSED);
    obj.write_to(w, text_modifier::FAILED);
    obj.write_to(w, text_modifier::NCFAILED);
    obj.write_to(w, text_modifier::NCPASSED);
    obj.write_to(w, text_modifier::BLOCKED);
    obj.write_to(w, text_modifier::PASSED_SUM);
    obj.write_to(w, text_modifier::FAILED_SUM);
    obj.write_to(w, text_modifier::NCPASSED_SUM);
    obj.write_to(w, text_modifier::NCFAILED_SUM);
    obj.write_to(w, text_modifier::BLOCKED_SUM);
  }

  using namespace testing;

  class fix
  {
  protected:
    fix() : writer(buff, "UTF-8", "--illegal--") {}
    StrictMock<test_buffer> buff;
    crpcut::output::writer writer;
    Sequence seq;
  };

  TEST(set_values_are_honoured, fix)
  {
    static const char config[] =
      " 0"
      " PASSED=1"
      " FAILED=2"
      " NCFAILED=3"
      " NCPASSED=4"
      " BLOCKED=5"
      " PASSED_SUM=6"
      " FAILED_SUM=7"
      " NCPASSED_SUM=8"
      " NCFAILED_SUM=9"
      " BLOCKED_SUM=10"
      " ";
    text_modifier modifier(config);
    ON_CALL(buff, write(_,1)).WillByDefault(Return(1));
    ON_CALL(buff, write(_,2)).WillByDefault(Return(2));
    EXPECT_CALL(buff, write(StartsWith("0"), 1)).InSequence(seq);
    EXPECT_CALL(buff, write(StartsWith("1"), 1)).InSequence(seq);
    EXPECT_CALL(buff, write(StartsWith("2"), 1)).InSequence(seq);
    EXPECT_CALL(buff, write(StartsWith("3"), 1)).InSequence(seq);
    EXPECT_CALL(buff, write(StartsWith("4"), 1)).InSequence(seq);
    EXPECT_CALL(buff, write(StartsWith("5"), 1)).InSequence(seq);
    EXPECT_CALL(buff, write(StartsWith("6"), 1)).InSequence(seq);
    EXPECT_CALL(buff, write(StartsWith("7"), 1)).InSequence(seq);
    EXPECT_CALL(buff, write(StartsWith("8"), 1)).InSequence(seq);
    EXPECT_CALL(buff, write(StartsWith("9"), 1)).InSequence(seq);
    EXPECT_CALL(buff, write(StartsWith("10"), 2)).InSequence(seq);

    modifier.write_to(writer, text_modifier::NORMAL);
    modifier.write_to(writer, text_modifier::PASSED);
    modifier.write_to(writer, text_modifier::FAILED);
    modifier.write_to(writer, text_modifier::NCFAILED);
    modifier.write_to(writer, text_modifier::NCPASSED);
    modifier.write_to(writer, text_modifier::BLOCKED);
    modifier.write_to(writer, text_modifier::PASSED_SUM);
    modifier.write_to(writer, text_modifier::FAILED_SUM);
    modifier.write_to(writer, text_modifier::NCPASSED_SUM);
    modifier.write_to(writer, text_modifier::NCFAILED_SUM);
    modifier.write_to(writer, text_modifier::BLOCKED_SUM);
  }

  TEST(failed_propagates, fix)
  {
    static const char config[] =
      " 0"
      " FAILED=F"
      " ";

    text_modifier modifier(config);
    ON_CALL(buff, write(_, 1)).WillByDefault(Return(1));
    EXPECT_CALL(buff, write(StartsWith("F"), 1)).Times(4);
    modifier.write_to(writer, text_modifier::FAILED);
    modifier.write_to(writer, text_modifier::NCFAILED);
    modifier.write_to(writer, text_modifier::FAILED_SUM);
    modifier.write_to(writer, text_modifier::NCFAILED_SUM);
    Mock::VerifyAndClearExpectations(&buff);

    EXPECT_CALL(buff, write(StartsWith("0"), 1)).Times(1);
    modifier.write_to(writer, text_modifier::NORMAL);
    Mock::VerifyAndClearExpectations(&buff);

    modifier.write_to(writer, text_modifier::PASSED);
    modifier.write_to(writer, text_modifier::NCPASSED);
    modifier.write_to(writer, text_modifier::BLOCKED);
    modifier.write_to(writer, text_modifier::PASSED_SUM);
    modifier.write_to(writer, text_modifier::NCPASSED_SUM);
    modifier.write_to(writer, text_modifier::BLOCKED_SUM);
  }

  TEST(ncfailed_propagates, fix)
  {
    static const char config[] =
      " 0"
      " NCFAILED=F"
      " ";

    text_modifier modifier(config);
    ON_CALL(buff, write(_, 1)).WillByDefault(Return(1));
    EXPECT_CALL(buff, write(StartsWith("F"), 1)).Times(2);
    modifier.write_to(writer, text_modifier::NCFAILED);
    modifier.write_to(writer, text_modifier::NCFAILED_SUM);
    Mock::VerifyAndClearExpectations(&buff);

    EXPECT_CALL(buff, write(StartsWith("0"), 1)).Times(1);
    modifier.write_to(writer, text_modifier::NORMAL);
    Mock::VerifyAndClearExpectations(&buff);

    modifier.write_to(writer, text_modifier::FAILED_SUM);
    modifier.write_to(writer, text_modifier::FAILED);
    modifier.write_to(writer, text_modifier::PASSED);
    modifier.write_to(writer, text_modifier::NCPASSED);
    modifier.write_to(writer, text_modifier::BLOCKED);
    modifier.write_to(writer, text_modifier::PASSED_SUM);
    modifier.write_to(writer, text_modifier::NCPASSED_SUM);
    modifier.write_to(writer, text_modifier::BLOCKED_SUM);
  }

  TEST(ncpassed_propagates, fix)
  {
    static const char config[] =
      " 0"
      " NCPASSED=P"
      " ";

    text_modifier modifier(config);
    ON_CALL(buff, write(_, 1)).WillByDefault(Return(1));
    EXPECT_CALL(buff, write(StartsWith("P"), 1)).Times(2);
    modifier.write_to(writer, text_modifier::NCPASSED);
    modifier.write_to(writer, text_modifier::NCPASSED_SUM);

    EXPECT_CALL(buff, write(StartsWith("0"), 1)).Times(1);
    modifier.write_to(writer, text_modifier::NORMAL);
    EXPECT_CALL(buff, write(_,_)).Times(0);
    modifier.write_to(writer, text_modifier::NCFAILED);
    modifier.write_to(writer, text_modifier::NCFAILED_SUM);
    modifier.write_to(writer, text_modifier::FAILED_SUM);
    modifier.write_to(writer, text_modifier::FAILED);
    modifier.write_to(writer, text_modifier::PASSED);
    modifier.write_to(writer, text_modifier::BLOCKED);
    modifier.write_to(writer, text_modifier::PASSED_SUM);
    modifier.write_to(writer, text_modifier::BLOCKED_SUM);
  }

  TEST(blocked_propagates, fix)
  {
    static const char config[] =
      " 0"
      " BLOCKED=B"
      " ";

    text_modifier modifier(config);
    ON_CALL(buff, write(_, 1)).WillByDefault(Return(1));
    EXPECT_CALL(buff, write(StartsWith("B"), 1)).Times(2);
    modifier.write_to(writer, text_modifier::BLOCKED);
    modifier.write_to(writer, text_modifier::BLOCKED_SUM);

    EXPECT_CALL(buff, write(StartsWith("0"), 1)).Times(1);
    modifier.write_to(writer, text_modifier::NORMAL);
    EXPECT_CALL(buff, write(_,_)).Times(0);
    modifier.write_to(writer, text_modifier::NCPASSED);
    modifier.write_to(writer, text_modifier::NCPASSED_SUM);
    modifier.write_to(writer, text_modifier::NCFAILED);
    modifier.write_to(writer, text_modifier::NCFAILED_SUM);
    modifier.write_to(writer, text_modifier::FAILED_SUM);
    modifier.write_to(writer, text_modifier::FAILED);
    modifier.write_to(writer, text_modifier::PASSED);
    modifier.write_to(writer, text_modifier::PASSED_SUM);
  }

  TEST(terminals_do_not_propagate, fix)
  {
    static const char config[] =
      " 0"
      " PASSED_SUM=PS"
      " FAILED_SUM=FS"
      " NCFAILED_SUM=NFS"
      " NCPASSED_SUM=NPS"
      " BLOCKED_SUM=BS"
      " ";

    text_modifier modifier(config);
    ON_CALL(buff, write(_, 1)).WillByDefault(Return(1));
    ON_CALL(buff, write(_, 2)).WillByDefault(Return(2));
    ON_CALL(buff, write(_, 3)).WillByDefault(Return(3));
    EXPECT_CALL(buff, write(StartsWith("PS"), 2)).Times(1);
    modifier.write_to(writer, text_modifier::PASSED_SUM);

    EXPECT_CALL(buff, write(StartsWith("FS"), 2)).Times(1);
    modifier.write_to(writer, text_modifier::FAILED_SUM);

    EXPECT_CALL(buff, write(StartsWith("BS"), 2)).Times(1);
    modifier.write_to(writer, text_modifier::BLOCKED_SUM);

    EXPECT_CALL(buff, write(StartsWith("NFS"), 3)).Times(1);
    modifier.write_to(writer, text_modifier::NCFAILED_SUM);

    EXPECT_CALL(buff, write(StartsWith("NPS"), 3)).Times(1);
    modifier.write_to(writer, text_modifier::NCPASSED_SUM);

    EXPECT_CALL(buff, write(StartsWith("0"), 1)).Times(1);
    modifier.write_to(writer, text_modifier::NORMAL);

    EXPECT_CALL(buff, write(_,_)).Times(0);
    modifier.write_to(writer, text_modifier::PASSED);
    modifier.write_to(writer, text_modifier::FAILED);
    modifier.write_to(writer, text_modifier::NCFAILED);
    modifier.write_to(writer, text_modifier::NCPASSED);
    modifier.write_to(writer, text_modifier::BLOCKED);
  }

  TEST(variable_subset_length_throws,
       EXPECT_EXCEPTION(text_modifier::illegal_decoration_format))
  {
    text_modifier modifier(" 0 BLOCKED_SU=apa ");
  }

  TEST(variable_superset_length_throws,
       EXPECT_EXCEPTION(text_modifier::illegal_decoration_format))
  {
    text_modifier modifier(" 0 BLOCKED_SUMM=apa ");
  }

  TEST(lacking_assign_throws,
       EXPECT_EXCEPTION(text_modifier::illegal_decoration_format))
  {
    text_modifier modifier(" 0 BLOCKED_SUM ");
  }

  TEST(lacking_terminator_throws,
       EXPECT_EXCEPTION(text_modifier::illegal_decoration_format))
  {
    text_modifier modifier(" 0 BLOCKED_SUM=apa");
  }
}
