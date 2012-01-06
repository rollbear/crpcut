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
#include "output_buffer.hpp"

namespace {
  const char data[] = "abcdefghijklmnopqrstuvwxyz12345";

  void fill_buffer(crpcut::output::buffer &b)
  {
    std::size_t target_objects = crpcut::heap::allocated_objects() + 10;
    while (crpcut::heap::allocated_objects() < target_objects)
      {
        size_t bytes_written = 0;
        while (bytes_written < sizeof(data) - 1)
          {
            ssize_t rv = b.write(data + bytes_written,
                                 sizeof(data) - 1 - bytes_written);
            ASSERT_TRUE(rv >= 0);
            bytes_written+= size_t(rv);
          }
      }
  }
}

TESTSUITE(output_buffer)
{

  using crpcut::output::buffer;

  TEST(default_constructed_buffer_is_empty)
  {
    buffer b;
    ASSERT_TRUE(b.is_empty());
    std::pair<const char*, std::size_t> r = b.get_buffer();
    static const char *const zerostr = 0;
    ASSERT_TRUE(r.first == zerostr);
    ASSERT_TRUE(r.second == 0U);
    b.advance();
  }

  TEST(block_handling)
  {
    ASSERT_SCOPE_HEAP_LEAK_FREE
    {
      buffer b;
      fill_buffer(b);
      const char *p = data;
      while (!b.is_empty())
        {
          std::pair<const char*, std::size_t> m = b.get_buffer();
          for (std::size_t n = 0U; n < m.second; ++n)
            {
              ASSERT_TRUE(m.first[n] == *p);
              if (++p == data + sizeof(data) - 1) p = data;
            }
          b.advance();
        }
    }
  }

  TEST(destructor_cleans_memory)
  {
    ASSERT_SCOPE_HEAP_LEAK_FREE
    {
      buffer b;
      fill_buffer(b);
    }
  }
}
