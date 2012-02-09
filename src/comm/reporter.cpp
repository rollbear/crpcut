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
#include "../test_environment.hpp"
#include "../wrapped/posix_encapsulation.hpp"

namespace crpcut {

  namespace comm {
    reporter report;

    void reporter::set_test_environment(test_environment *env)
    {
      assert(current_test_ == 0);
      current_test_ = env;
    }

    void reporter::send_message(type t, const char *msg, size_t len) const
    {
      const size_t header_size = sizeof(t) + sizeof(len);

      void *report_addr = alloca(len + header_size);

      *static_cast<type*>(report_addr) = t;
      char *p = static_cast<char *>(report_addr);
      p+= sizeof(type);

      *static_cast<size_t*>(static_cast<void*>(p)) = len;
      p+= sizeof(len);

      wrapped::memcpy(p, msg, len);
      write_fd_->write_loop(report_addr, len + header_size);

      size_t bytes_written;
      read(bytes_written);
      assert(len == bytes_written);
    }

    void reporter::operator()(type t, const char *msg, size_t len) const
    {
      if (!current_test_)
        {
          if (len)
            {
              default_out_ << "\n" << std::string(msg, len) << std::flush;
            }
          if (t == exit_fail)
            {
              wrapped::abort();
            }
          return;
        }

      try {

        if (current_test_->is_naughty_child())
          {
            t = static_cast<type>(t | kill_me);
          }

        send_message(t, msg, len);

        if (t == comm::exit_fail)
          {
            wrapped::_Exit(0);
          }
      }
      catch (...)
        {
          current_test_->freeze();
        }
    }

    reporter::reporter(std::ostream &default_out)
      : write_fd_(0),
        read_fd_(0),
        current_test_(0),
        default_out_(default_out)
    {
    }

    void
    reporter::set_read_fd(rfile_descriptor *r)
    {
      assert(read_fd_ == 0);
      read_fd_ = r;
    }

    void
    reporter::set_write_fd(wfile_descriptor *w)
    {
      assert(write_fd_ == 0);
      write_fd_ = w;
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
