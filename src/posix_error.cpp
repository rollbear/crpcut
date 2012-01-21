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

#include "posix_error.hpp"
#include "wrapped/posix_encapsulation.hpp"

namespace crpcut {
  posix_error
  ::posix_error(int e, const char *msg)
  : msg_(0),
    e_(e)
  {
    if (e_ == 0) return;
    const size_t mlen = wrapped::strlen(msg);
    const char *errstr = wrapped::strerror(e);
    const size_t elen = wrapped::strlen(errstr);

    char *str = static_cast<char*>(wrapped::malloc(elen + mlen + 6 + 1));
    if (!str)
      {
        // better to bite the dust here, due to memory error, than to
        // terminate in the what() member function
        static std::bad_alloc exc;
        throw exc;
      }
    lib::strcpy(lib::strcpy(lib::strcpy(str, errstr),
                            " from "),
                msg);
    msg_ = str;
  }

  posix_error
  ::posix_error(const posix_error &e) :
    std::exception(*this),
    msg_(e.msg_),
    e_(e.e_)
  {
    e.msg_ = 0; // move
  }

  const char *
  posix_error
  ::what() const throw ()
  {
    return msg_;
  }

  posix_error
  ::~posix_error() throw ()
  {
    wrapped::free(msg_);
  }

}
