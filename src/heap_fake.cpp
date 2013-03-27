/*
 * Copyright 2013 Bjorn Fahller <bjorn@fahller.se>
 * All rights reserved

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
#include "heap.hpp"
namespace {
  const char libname[] = "-lcrpcut_heap";
void fail(const char *func)
{
  FAIL << "To use crpcut::heap::" << func << "() you must link with " << libname;
}
}
namespace crpcut {
  namespace heap {
    size_t allocated_bytes()
    {
      fail("allocated_bytes");
      return 0U;
    }
    size_t allocated_objects()
    {
      fail("allocated_objects");
      return 0U;
    }
    size_t set_limit(size_t n)
    {
      if (n != system) fail("set_limit");
      return system;
    }
    void control::enable()
    {
      enabled = true;
    }
    bool control::enabled = false;
    void enable_backtrace()
    {
      std::cerr << "Backtrace can only be used when linked with " << libname << '\n';
      exit(1);
    }
  }
}
