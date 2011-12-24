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

#ifndef PRESENTATION_READER_HPP
#define PRESENTATION_READER_HPP

#include "list_elem.hpp"
#include "test_case_result.hpp"
#include "io.hpp"
namespace crpcut {
  namespace output {
    class formatter;
  }

  template <typename T, size_t> class poll;

  class presentation_reader : public io
  {
  public:
    presentation_reader(poll<io, 2>       &poller_,
                        int                fd_,
                        output::formatter &fmt_,
                        bool               verbose_);
    virtual ~presentation_reader();
    virtual bool read();
    virtual bool write();
    virtual void exception();
  private:
    list_elem<test_case_result>  messages;
    poll<io, 2>                 &poller;
    int                          fd;
    output::formatter           &fmt;
    bool                         verbose;

    presentation_reader();
    presentation_reader(const presentation_reader&);
    presentation_reader& operator=(const presentation_reader&);
  };
}

#endif // PRESENTATION_READER_HPP
