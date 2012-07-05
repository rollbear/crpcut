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

#include <crpcut.hpp>
#include "poll.hpp"
namespace crpcut {


  bool
  fdreader::
  read_data()
  {
    return do_read_data();
  }

  crpcut_test_monitor *
  fdreader
  ::get_monitor() const
  {
    return mon_;
  }

  void
  fdreader
  ::close()
  {
    unregister();
  }

  fdreader
  ::fdreader(crpcut_test_monitor *r, int fd)
    : rfile_descriptor(fd),
      mon_(r),
      poller_(0)
  {
  }

  void fdreader::set_fd(int fd, poll<fdreader> *poller)
  {
    assert(mon_ != 0);
    assert(!poller_);
    assert(poller);
    rfile_descriptor(fd).swap(*this);
    poller_ = poller;
    poller_->add_fd(this);
    mon_->activate_reader();
  }

  void fdreader::unregister()
  {
    assert(poller_);
    assert(mon_ != 0);
    mon_->deactivate_reader();
    poller_->del_fd(this);
    rfile_descriptor().swap(*this);;
    poller_ = 0;
  }
}
