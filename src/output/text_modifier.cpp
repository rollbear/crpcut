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

#include "text_modifier.hpp"
#include "writer.hpp"
#include <stdexcept>

#define MK_F_STR(x) { #x, sizeof(#x) - 1 }
namespace {
  template <typename T, std::size_t N>
  T* begin(T (&array)[N])
  {
    return array;
  }

  template <typename T, std::size_t N>
  T *end(T (&array)[N])
  {
    return array + N;
  }

  const crpcut::datatypes::fixed_string decorator_names[] =
    { DECORATORS(MK_F_STR) };

  void zero_decorators(crpcut::datatypes::fixed_string *b,
                       crpcut::datatypes::fixed_string *e)
  {
    while (b != e)
      {
        b->str = 0;
        b->len = 0;
        ++b;
      }
  }

  template <typename err>
  crpcut::datatypes::fixed_string get_substr(const char *&p, char separator,
                                             const std::string &msg)
  {
    const char *e = crpcut::wrapped::strchr(p, separator);
    if (!e)
      {
        throw err(msg);
      }
    crpcut::datatypes::fixed_string rv = { p, std::size_t(e - p) };
    p = e + 1;
    return rv;
  }

  template <typename err, size_t N>
  int get_decorator(crpcut::datatypes::fixed_string s,
                    const crpcut::datatypes::fixed_string (&array)[N])
  {
    for (const crpcut::datatypes::fixed_string *i = begin(array);
         i != end(array);
         ++i)
      {
        if (*i == s) return int(i - begin(array));
      }
    throw err(std::string(s.str, s.len) + " is not a decorator");
  }
}

namespace crpcut {
  namespace output {

    text_modifier
    ::text_modifier(const char *rules)
      : longest_decorator_len_(0)
    {
      zero_decorators(begin(decorators), end(decorators));
      if (!rules) return;
      const char separator = *rules++;

      typedef illegal_decoration_format err;
      if (*rules)
        {
          decorators[NORMAL]
            = get_substr<err>(rules, separator,
                              "Missing separator after default decorator");
          longest_decorator_len_ = decorators[NORMAL].len;

        }
      while (*rules)
        {
          datatypes::fixed_string name =
            get_substr<err>(rules, '=', "Missing = after name");
          std::string msg = "Missing separator after value for "
            + std::string(name.str, name.len);
          datatypes::fixed_string value = get_substr<err>(rules, separator,
                                                          msg);
          int idx = get_decorator<err>(name, decorator_names);
          decorators[idx] = value;
          if (value.len > longest_decorator_len_)
            {
              longest_decorator_len_ = value.len;
            }
        }
      init_decorator(NCFAILED,     FAILED);
      init_decorator(NCPASSED,     PASSED);
      init_decorator(PASSED_SUM,   PASSED);
      init_decorator(FAILED_SUM,   FAILED);
      init_decorator(NCPASSED_SUM, NCPASSED);
      init_decorator(NCFAILED_SUM, NCFAILED);
      init_decorator(BLOCKED_SUM,  BLOCKED);
    }

    void
    text_modifier
    ::write_to(writer &output, decorator m) const
    {
      assert(m < END_OF_LIST);
      if (!decorators[m]) return;

      output.write(decorators[m].str, decorators[m].len, writer::verbatim);
    }

    void
    text_modifier
    ::write_to(std::ostream &os, decorator m) const
    {
      assert(m < END_OF_LIST);
      if (!decorators[m]) return;

      os.write(decorators[m].str, std::streamsize(decorators[m].len));
    }

    void
    text_modifier
    ::init_decorator(decorator dest, decorator src)
    {
      if (!decorators[dest])
        {
          decorators[dest] = decorators[src];
        }
    }
  }
}
