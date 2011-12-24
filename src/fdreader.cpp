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

#include <crpcut.hpp>
#include "poll_singleton.hpp"
namespace crpcut {


  bool
  fdreader::
  read(bool do_reply)
  {
    return do_read(fd_, do_reply);
  }

  crpcut_test_case_registrator *
  fdreader
  ::get_registrator() const
  {
    return reg;
  }

  void
  fdreader
  ::close()
  {
    if (fd_)
      {
        wrapped::close(fd_);
      }
    unregister();
  }

  fdreader
  ::fdreader(crpcut_test_case_registrator *r, int fd)
    : reg(r),
      fd_(fd)
  {
  }

  void fdreader::set_fd(int fd)
  {
    assert(fd_ == 0);
    assert(reg != 0);
    fd_ = fd;
    poller.add_fd(fd_, this);
    reg->crpcut_activate_reader();
  }

  void fdreader::unregister()
  {
    assert(fd_ != 0);
    assert(reg != 0);
    reg->crpcut_deactivate_reader();
    poller.del_fd(fd_);
    fd_ = 0;
  }
}
