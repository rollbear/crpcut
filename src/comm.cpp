/*
 * Copyright 2009 Bjorn Fahller <bjorn@fahller.se>
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

namespace crpcut {

  namespace comm {

    void reporter::operator()(type t, size_t len, const char *msg) const
    {
      if (!test_case_factory::tests_as_child_procs())
        {
          // this is strange. If I use std::cout, output is lost, despite
          // that it's set to unbuffered, and even if it's explicitly
          // flushed.
          ::write(1, msg, len);
          if (t == exit_fail)
            {
              ::abort();
            }
          return;
        }
      std::cout << std::flush;
      write(t);
      write(len);
      const char *p = msg;
      size_t bytes_written = 0;
      while (bytes_written < len)
        {
          int rv = ::write(write_fd,
                           p + bytes_written,
                           len - bytes_written);
          if (rv == -1 && errno == EINTR) continue;
          if (rv <= 0) throw "report failed";
          bytes_written += rv;
        }
      read(bytes_written);
      assert(len == bytes_written);
      bool terminal = (t == comm::exit_ok) || (t == comm::exit_fail);
      if (!terminal) return;
      std::_Exit(0);
    }

    reporter report;
  }

}