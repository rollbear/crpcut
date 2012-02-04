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

#include "writer.hpp"
#include "buffer.hpp"
#include "../wrapped/posix_encapsulation.hpp"
extern "C" {
#  include <iconv.h>
}

namespace crpcut {
  namespace wrapped {
    CRPCUT_WRAP_FUNC(libc, iconv_open, iconv_t,
                     (const char *to, const char *from),
                     (to, from))
    CRPCUT_WRAP_FUNC(libc, iconv_close, int, (iconv_t handle), (handle))
    CRPCUT_WRAP_FUNC(libc, iconv, size_t,
                     (iconv_t h, char **i, size_t *iz, char **o, size_t *oz),
                     (h, i, iz, o, oz))
  }
  namespace output {
    writer
    ::writer(buffer &buff, const char *to_charset, const char *subst)
      : buffer_(buff),
        iconv_handle_(wrapped::iconv_open(to_charset,
                                          get_program_charset())),
        illegal_substitute_(subst),
        illegal_substitute_len_(wrapped::strlen(subst))
    {
      if (iconv_handle_ == iconv_t(-1))
        {
          std::ostringstream oss;
          oss << "Can't convert from \"" << get_program_charset()
              << "\" to \"" << to_charset << "\"";
          throw std::runtime_error(oss.str());
        }
    }

    writer
    ::~writer()
    {
      wrapped::iconv_close(iconv_handle_);
    }

    std::size_t
    writer
    ::write(const std::ostringstream &os, type t) const
    {
      const std::string &s = os.str();
      return write(s.c_str(), s.length(), t);
    }

    size_t
    writer
    ::write(const char *str, std::size_t len, type t) const
    {
      if (t == verbatim)
        {
          return do_write(str, len);
        }

      std::size_t prev_n = 0;
      for (std::size_t n = 0; n < len; ++n)
        {
          const datatypes::fixed_string esc = escape(str[n]);
          if (!esc) continue;
          do_write_converted(str + prev_n, n - prev_n);
          prev_n = n + 1;
          do_write(esc.str, esc.len);
        }
      do_write_converted(str + prev_n, len - prev_n);
      return len;
    }

    datatypes::fixed_string
    writer
    ::escape(char) const
    {
      static datatypes::fixed_string no_escape = { 0, 0 };
      return no_escape;
    }


    std::size_t
    writer
    ::do_write(const char *p, std::size_t len) const
    {
      std::size_t bytes_written = 0;
      while (bytes_written < len)
        {
          ssize_t rv = buffer_.write(p + bytes_written, len - bytes_written);
          assert(rv >= 0);
          bytes_written += size_t(rv);
        }
      return bytes_written;
    }

    std::size_t
    writer
    ::do_write_converted(const char *in, std::size_t len) const
    {
      while (len > 0)
        {
          const std::size_t conversion_buffer_size = 2048;
          static char conversion_buffer[conversion_buffer_size];
          std::size_t out_remaining = conversion_buffer_size;

          char *output_ptr = conversion_buffer;
          const char *const out_begin  = output_ptr;
          const std::size_t rv = wrapped::iconv(iconv_handle_,
                                                const_cast<char**>(&in), &len,
                                                &output_ptr, &out_remaining);
          const std::size_t out_length(size_t(output_ptr - out_begin));
          do_write(out_begin, out_length);
          if (rv != size_t(-1))
            {
              continue;
            }
          switch (errno)
            {
            case E2BIG:
              break;
            case EINVAL:
            case EILSEQ:
              if (len) { ++in; --len; }
              do_write(illegal_substitute_, illegal_substitute_len_);
              break;
            default:
              wrapped::abort();
              /* no break */
            }
        }
      return len;
    }

  }
}
