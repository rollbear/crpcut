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

#include "presentation_output.hpp"
#include "output_buffer.hpp"
#include "posix_encapsulation.hpp"
namespace crpcut {

  presentation_output
  ::presentation_output(poll<io, 2> &poller_, int fd_)
    : poller(poller_),
      fd(fd_),
      pos(0),
      is_enabled(false)
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
    if (val != is_enabled)
      {
        if (val)
          {
            poller.add_fd(fd, this, poll<io, 2>::polltype::w);
          }
        else
          {
            poller.del_fd(fd);
          }
        is_enabled = val;
      }
  }

  bool
  presentation_output
  ::enabled() const
  {
    return is_enabled;
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
    assert(!output::buffer::is_empty());
    while (!output::buffer::is_empty())
      {
        std::pair<const char *, size_t> data = output::buffer::get_buffer();
        const char *buff = data.first;
        size_t      len  = data.second;

        assert(buff);
        assert(len);

        ssize_t n = wrapped::write(fd, buff + pos, len - pos);
        assert(n >= 0);
        pos+= size_t(n);
        if (pos == len)
          {
            pos = 0;
            output::buffer::advance();
          }
      }
    return false;
  }

}
