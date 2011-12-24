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

#ifndef OUTPUT_BUFFER_HPP
#define OUTPUT_BUFFER_HPP

#include <utility>
extern "C" {
#  include <sys/types.h>
}
namespace crpcut {
  namespace output {
    class buffer
    {
    public:
      static std::pair<const char*, std::size_t> get_buffer();
      static void advance();
      static ssize_t write(const char *buff, std::size_t len);
      static bool is_empty();
    private:
      buffer();
      ~buffer();

      static buffer& obj();
      std::pair<const char*, std::size_t> do_get_buffer() const;
      void do_advance();
      ssize_t do_write(const char *buff, std::size_t len);
      bool do_is_empty() const;
      struct block
      {
        block() :next(0), len(0) {}
        static const std::size_t size = 128;

        block       *next;
        char         mem[size];
        std::size_t  len;
      };


      block  *head;
      block **current;
    };

    inline std::pair<const char*, std::size_t> buffer::get_buffer()
    {
      return obj().do_get_buffer();
    }

    inline void buffer::advance()
    {
      obj().do_advance();
    }

    inline ssize_t buffer::write(const char *buff, std::size_t len)
    {
      return obj().do_write(buff, len);
    }

    inline bool buffer::is_empty()
    {
      return obj().do_is_empty();
    }

    inline buffer::buffer()
      : head(0),
        current(&head)
    {
    }

    inline buffer& buffer::obj()
    {
      static buffer object;
      return object;
    }

    inline bool buffer::do_is_empty() const
    {
      return !head;
    }

  }
}

#endif // OUTPUT_BUFFER_HPP
