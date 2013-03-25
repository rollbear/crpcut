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

TESTSUITE(comm)
{
  TESTSUITE(reporter) {}
  TESTSUITE(direct_reporter, DEPENDS_ON(ALL_TESTS(reporter)))
  {
    struct fix
    {
      fix() : actual_reporter(os) { }
      crpcut::stream::toastream<32> os;
      crpcut::comm::reporter actual_reporter;
    };

    TEST(trivial_output_is_copied_in_verbatim, fix)
    {
      {
        crpcut::comm::direct_reporter<crpcut::comm::fail> d(actual_reporter,0);
        d << "apa" << ' ' << 32;
        ASSERT_TRUE(os.size() == 0U);
      }
      ASSERT_TRUE(std::string(os.begin(), os.end()) == "\napa 32\n");
    }

    TEST(output_stream_manipulators_are_forwarded, fix)
    {
      {
        crpcut::comm::direct_reporter<crpcut::comm::fail> d(actual_reporter,0);
        d << std::hex << std::noshowbase << 32 << std::endl
          << std::setw(5) << std::left << 1;
      }
      ASSERT_TRUE(std::string(os.begin(), os.end()) == "\n20\n1    \n");
    }

    TEST(output_works_in_limited_heap, fix)
    {
      {
        crpcut::heap::set_limit(crpcut::heap::allocated_bytes());
        crpcut::comm::direct_reporter<crpcut::comm::fail> d(actual_reporter,0);
        d << "apa\nkatt";
      }
      crpcut::heap::set_limit(crpcut::heap::system);
      ASSERT_TRUE(std::string(os.begin(), os.end()) == "\napa\nkatt\n");
    }

    TEST(variables_are_forwarded, fix)
    {
      {
        crpcut::comm::direct_reporter<crpcut::comm::fail> d(actual_reporter,0);
        int c = 3;
        const unsigned u = 8;
        d << c << " " << u;
      }
      ASSERT_TRUE(std::string(os.begin(), os.end()) == "\n3 8\n");
    }

  }
}

