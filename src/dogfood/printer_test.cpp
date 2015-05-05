/*
 * Copyright 2011-2013 Bjorn Fahller <bjorn@fahller.se>
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

#include <trompeloeil.hpp>
#include <crpcut.hpp>
#include "../printer.hpp"
#include "../output/formatter.hpp"

TESTSUITE(printer)
{
  class test_formatter : public crpcut::output::formatter
  {
  public:
    MAKE_MOCK4(begin_case, void(std::string, bool, bool, unsigned long));
    MAKE_MOCK0(end_case, void());
    MAKE_MOCK4(terminate,
               void(crpcut::test_phase,
                    crpcut::datatypes::fixed_string,
                    crpcut::datatypes::fixed_string,
                    std::string));
    MAKE_MOCK3(print, void(crpcut::datatypes::fixed_string,
                           crpcut::datatypes::fixed_string,
                           crpcut::datatypes::fixed_string));
    MAKE_MOCK2(statistics, void(unsigned, unsigned));
    MAKE_MOCK1(nonempty_dir, void(const char*));
    MAKE_MOCK2(blocked_test,
               void(crpcut::tag::importance, std::string));
  };

  TEST(create_and_destroy_calls_formatter_begin_and_end)
  {
    test_formatter fmt;

    crpcut::printer *p = nullptr;
    {
      REQUIRE_CALL(fmt, begin_case("apa", false, true, 100U));
      p = new crpcut::printer(fmt, "apa", false, true, 100);
    }
    REQUIRE_CALL(fmt, end_case());
    delete p;
  }
}
