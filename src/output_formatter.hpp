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

#ifndef OUTPUT_FORMATTER_HPP
#define OUTPUT_FORMATTER_HPP

extern "C"
{
#  include <sys/types.h>
#  include <iconv.h>
}

#include "fixed_string.hpp"
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

    class formatter
    {

    public:
      typedef enum { translated, verbatim } type;
      virtual void begin_case(const char  *name,
                              std::size_t  name_len,
                              bool         result,
                              bool         critical) = 0;
      virtual void end_case()  = 0;
      virtual void terminate(test_phase   phase,
                             const char  *msg,
                             std::size_t  msg_len,
                             const char  *dirname = 0,
                             std::size_t  dn_len = 0) = 0;
      virtual void print(const char  *tag,
                         std::size_t  tlen,
                         const char  *data,
                         std::size_t  dlen) = 0;
      virtual void statistics(unsigned num_registered,
                              unsigned num_selected,
                              unsigned num_run,
                              unsigned num_failed) = 0;
      virtual void nonempty_dir(const  char*)  = 0;
      virtual void blocked_test(const crpcut_test_case_registrator *)  = 0;
      virtual void tag_summary(const char *tag_name,
                               std::size_t num_passed,
                               std::size_t num_failed,
                               bool        critical) = 0;
      virtual ~formatter();
    protected:
      static const fixed_string &phase_str(test_phase);
      formatter(const char *to_charset, const char *subst);
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
      std::size_t write(const char *str, std::size_t len, type t = verbatim) const;
      template <typename T>
      std::size_t write(T val,
                        typename enable_if<std::numeric_limits<T>::is_integer>::type * = 0)
      {
        stream::toastream<std::numeric_limits<T>::digits10> o;
        o << val;
        return write(o.begin(), o.size());
      }
    private:
      virtual fixed_string escape(char c) const;
      std::size_t do_write(const char *p, std::size_t len) const;
      std::size_t do_write_converted(const char *buff, std::size_t len) const;

      iconv_t     iconv_handle;
      const char *illegal_substitute;
      std::size_t illegal_substitute_len;
    };

  }
}

#endif // OUTPUT_FORMATTER_HPP
