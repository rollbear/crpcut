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
#include "../posix_error.hpp"

namespace crpcut {
  namespace comm {

    file_descriptor::file_descriptor(int fd)
    : fd_(fd)
    {
    }

    file_descriptor::~file_descriptor()
    {
      close();
    }

    void
    file_descriptor::close()
    {
      if (fd_ == -1) return;
      (void)wrapped::close(fd_); // not much to do on error
      fd_ = -1;
    }

    rfile_descriptor
    ::rfile_descriptor(int fd)
      : file_descriptor(fd)
    {
    }

    rfile_descriptor
    ::rfile_descriptor()
      : file_descriptor()
    {
    }

    ssize_t
    rfile_descriptor
    ::read(void *buff, size_t len) const
    {
      return wrapped::read(fd_, buff, len);
    }


    wfile_descriptor
    ::wfile_descriptor(int fd)
    : file_descriptor(fd)
    {
    }

    wfile_descriptor
    ::wfile_descriptor()
    : file_descriptor()
    {
    }

    ssize_t
    wfile_descriptor
    ::write(const void *buff, size_t len) const
    {
      errno = 0;
      return wrapped::write(fd_, buff, len);
    }

  }
}
