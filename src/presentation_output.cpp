/*
 * Copyright 2011-2012 Bjorn Fahller <bjorn@fahller.se>
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

#include "presentation_output.hpp"
#include "output/buffer.hpp"
#include "wrapped/posix_encapsulation.hpp"
#include "poll.hpp"

namespace crpcut {

  presentation_output
  ::presentation_output(output::buffer &buffer,
                        poll<io>       &poller,
                        int             fd,
                        posix_write    &w)
    : buffer_(buffer),
      poller_(poller),
      fd_(fd),
      pos_(0),
      is_enabled_(false),
      write_(w)
  {
  }

  presentation_output
  ::~presentation_output()
  {
  }

  void
  presentation_output
  ::enable(bool val)
  {
    if (fd_ != -1 && val != is_enabled_)
      {
        if (val)
          {
            poller_.add_fd(fd_, this, poll<io>::polltype::w);
          }
        else
          {
            poller_.del_fd(fd_);
          }
        is_enabled_ = val;
      }
  }

  bool
  presentation_output
  ::enabled() const
  {
    return is_enabled_;
  }

  bool
  presentation_output
  ::read()
  {
    assert("Shan't ever read output" == 0);
    return true;
  }

  bool
  presentation_output
  ::write()
  {
    assert(enabled());
    assert(!buffer_.is_empty());

    std::pair<const char *, size_t> data = buffer_.get_buffer();
    const char *buff = data.first;
    size_t      len  = data.second;

    assert(buff);
    assert(len);

    ssize_t n = write_(fd_, buff + pos_, len - pos_);

    assert(n >= 0);
    pos_+= size_t(n);
    if (pos_ == len)
      {
        pos_ = 0;
        buffer_.advance();
      }
    return false;
  }

  posix_write&
  presentation_output
  ::default_write()
  {
    static libc_write obj;
    return obj;
  }
}
