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

#include "../working_dir_allocator.hpp"
#include <crpcut.hpp>
#include <set>
TESTSUITE(working_dir_allocator)
{
  TEST(allocate_all_in_sequence_gives_unique_numbers)
  {
    unsigned vector[8 + 2];
    vector[0] = vector[9] = 0x12345678U;
    crpcut::working_dir_allocator dirs(vector + 1, 8);
    std::set<unsigned> numbers;
    for (int i = 0; i < 8; ++i)
      {
        ASSERT_TRUE(numbers.insert(dirs.allocate()).second);
      }
    ASSERT_TRUE(vector[0] == 0x12345678U);
    ASSERT_TRUE(vector[9] == 0x12345678U);
  }

#ifndef NDEBUG
  TEST(allocate_one_too_many_dies_on_sigabrt,
       DEPENDS_ON(allocate_all_in_sequence_gives_unique_numbers),
       EXPECT_SIGNAL_DEATH(SIGABRT),
       NO_CORE_FILE)
  {
    unsigned vector[9];
    crpcut::working_dir_allocator dirs(vector + 1, 8);
    for (int i = 0; i < 9; ++i)
      {
        dirs.allocate();
      }
  }
#endif

  TEST(allocate_gives_released_in_reverse_order)
  {
    unsigned vector[8];
    crpcut::working_dir_allocator dirs(vector, 8);
    unsigned values[8];
    for (int i = 0; i < 8; ++i)
      {
        values[i] = dirs.allocate();
      }
    dirs.free(values[3]);
    dirs.free(values[0]);
    dirs.free(values[5]);
    dirs.free(values[2]);
    dirs.free(values[7]);
    dirs.free(values[4]);
    dirs.free(values[6]);
    dirs.free(values[1]);
    ASSERT_TRUE(dirs.allocate() == values[1]);
    ASSERT_TRUE(dirs.allocate() == values[6]);
    ASSERT_TRUE(dirs.allocate() == values[4]);
    ASSERT_TRUE(dirs.allocate() == values[7]);
    ASSERT_TRUE(dirs.allocate() == values[2]);
    ASSERT_TRUE(dirs.allocate() == values[5]);
    ASSERT_TRUE(dirs.allocate() == values[0]);
    ASSERT_TRUE(dirs.allocate() == values[3]);
  }
}







