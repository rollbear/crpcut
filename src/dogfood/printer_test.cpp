/*
 * Copyright 2011 Bjorn Fahller <bjorn@fahller.se>
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
#include "../printer.hpp"
#include "../output/formatter.hpp"

TESTSUITE(printer)
{
  class test_formatter : public crpcut::output::formatter
  {
  public:
    MOCK_METHOD4(begin_case, void(const char*, std::size_t, bool, bool));
    MOCK_METHOD0(end_case, void());
    MOCK_METHOD5(terminate,
                 void(crpcut::test_phase,
                      const char*, std::size_t,
                      const char*, std::size_t));
    MOCK_METHOD4(print, void(const char*, std::size_t,
                             const char *, std::size_t));
    MOCK_METHOD4(statistics, void(unsigned, unsigned, unsigned, unsigned));
    MOCK_METHOD1(nonempty_dir, void(const char*));
    MOCK_METHOD1(blocked_test,
                 void(const crpcut::crpcut_test_case_registrator *));
    MOCK_METHOD4(tag_summary,
                 void(const char*, std::size_t, std::size_t, bool));
  };
  TEST(create_and_destroy_calls_formatter_begin_and_end)
  {
    using namespace testing;
    StrictMock<test_formatter> fmt;

    EXPECT_CALL(fmt, begin_case("apa", 3, false, true)).Times(1);
    crpcut::printer p(fmt, "apa", 3, false, true);
    Mock::VerifyAndClearExpectations(&fmt);
    EXPECT_CALL(fmt, end_case()).Times(1);
  }
}
