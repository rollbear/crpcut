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

#ifndef OUTPUT_WRITER_HPP
#define OUTPUT_WRITER_HPP

#include <iosfwd>
extern "C"
{
#  include <sys/types.h>
#  include <iconv.h>
}
#include <crpcut.hpp>
namespace crpcut {
  namespace output {
    template <bool b>
    struct enable_if;
    template <>
    struct enable_if<true>
    {
      typedef void type;
    };

    class buffer;

    class writer
    {
    public:
      typedef enum { translated, verbatim } type;
      writer(buffer& buff, const char *to_charset, const char *subst);
      virtual ~writer();
      std::size_t write(std::string s, type t = verbatim) const
      {
        return write(s.c_str(), s.length(), t);
      }
      std::size_t write(datatypes::fixed_string s, type t = verbatim) const
      {
        return write(s.str, s.len, t);
      }
      std::size_t write(const char *s, type t = verbatim) const
      {
        return write(s, wrapped::strlen(s), t);
      }
      template <std::size_t N>
      std::size_t write(const char (&str)[N], type t = verbatim) const
      {
        return write(&str[0], N - 1, t);
      }
      std::size_t write(const stream::oastream &o, type t = verbatim) const
      {
        return write(o.begin(), o.size(), t);
      }
      std::size_t write(const std::ostringstream &os, type t = verbatim) const;
      std::size_t write(const char *str, std::size_t len, type t = verbatim) const;
      template <typename T>
      std::size_t write(T val,
                        typename enable_if<std::numeric_limits<T>::is_integer>::type * = 0)
      {
        stream::toastream<std::numeric_limits<T>::digits10 + 2> o;
        o << val;
        return write(o);
      }
    private:
      virtual datatypes::fixed_string escape(char c) const;
      std::size_t do_write(const char *p, std::size_t len) const;
      std::size_t do_write_converted(const char *buff, std::size_t len) const;

      buffer     &buffer_;
      iconv_t     iconv_handle_;
      const char *illegal_substitute_;
      std::size_t illegal_substitute_len_;
    };
  }
}

#endif // OUTPUT_WRITER_HPP
