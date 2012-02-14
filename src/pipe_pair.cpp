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

#include "wrapped/posix_encapsulation.hpp"
#include "posix_error.hpp"
#include "pipe_pair.hpp"

namespace {
  inline void close_and_invalidate(int& fd)
  {
    if (fd <= 0) return;
    int rv = crpcut::wrapped::close(fd);
    assert(rv == 0);
    fd = -1;
  }
}

namespace crpcut {
  pipe_pair
  ::pipe_pair(const char *purpose_msg)
  {
    int rv = wrapped::pipe(fds);
    if (rv < 0) throw posix_error(errno, purpose_msg);
  }

  pipe_pair
  ::~pipe_pair()
  {
    close();
  }

  void
  pipe_pair
  ::close()
  {
    close_and_invalidate(fds[0]);
    close_and_invalidate(fds[1]);
  }

  int
  pipe_pair
  ::for_reading(purpose p)
  {
    assert(fds[1] >= 0);
    close_and_invalidate(fds[1]);
    int n = fds[0];
    if (p == release_ownership) fds[0] = -1;
    return n;
  }

  int
  pipe_pair
  ::for_writing(purpose p)
  {
    assert(fds[0] >= 0);
    close_and_invalidate(fds[0]);
    int n = fds[1];
    if (p == release_ownership) fds[1] = -1;
    return n;
  }
}
