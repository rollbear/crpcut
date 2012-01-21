/*
 * Copyright 2009-2012 Bjorn Fahller <bjorn@fahller.se>
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
#include "../wrapped/posix_encapsulation.hpp"

namespace crpcut {

  namespace comm {
    reporter report;

    void reporter::operator()(type t, const char *msg, size_t len) const
    {
      if (!test_case_factory::tests_as_child_procs())
        {
          if (len)
            {
              std::cout << "\n" << std::string(msg, len) << std::flush;
            }
          if (t == exit_fail)
            {
              wrapped::abort();
            }
          return;
        }
      int mask = 0;
      if (test_case_factory::is_naughty_child())
        {
          mask = kill_me;
        }
      t = static_cast<type>(mask | t);

      static const size_t header_size = sizeof(t) + sizeof(len);
      void *report_addr = alloca(len + header_size);
      *static_cast<type*>(report_addr) = t;
      char *p = static_cast<char *>(report_addr);
      p+= sizeof(type);
      *static_cast<size_t*>(static_cast<void*>(p)) = len;
      p+= sizeof(len);
      wrapped::memcpy(p, msg, len);
      len+= header_size;
      write_fd.write_loop(report_addr, len);
      while (mask) // infinite
        {
          wrapped::select(0, 0, 0, 0, 0);
        }
      size_t bytes_written;
      read(bytes_written);
      assert(len - header_size == bytes_written);
      if (t == comm::exit_fail)
        {
          wrapped::_Exit(0);
        }
    }

    reporter::reporter()
      : write_fd(),
        read_fd()
    {
    }

    void
    reporter::set_fds(int rfd, int wfd)
    {
      wfile_descriptor(wfd).swap(write_fd);
      rfile_descriptor(rfd).swap(read_fd);;
    }

    void
    reporter::operator()(type t, const stream::oastream &os) const
    {
      operator()(t, os.begin(), os.size());
    }

    void
    reporter::operator()(type t, const std::ostringstream &os) const
    {
      const std::string &s = os.str();
      operator()(t, s.c_str(), s.length());
    }

    void
    reporter::operator()(type t, const char *msg) const
    {
      operator()(t, msg, wrapped::strlen(msg));
    }
  }

}
