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

#ifndef TEXT_MODIFIER_HPP
#define TEXT_MODIFIER_HPP

#include "../wrapped/posix_encapsulation.hpp"
#include <iosfwd>
#define DECORATORS(x)                           \
  x(NORMAL),                                    \
    x(PASSED),                                  \
    x(FAILED),                                  \
    x(NCFAILED),                                \
    x(NCPASSED),                                \
    x(BLOCKED),                                 \
    x(PASSED_SUM),                              \
    x(FAILED_SUM),                              \
    x(NCPASSED_SUM),                            \
    x(NCFAILED_SUM),                            \
    x(BLOCKED_SUM)


#define VERBATIM(x) x
#define EMPTY(x) { 0, 0 }
namespace crpcut {
  namespace output {
    class writer;

    class text_modifier {
    public:
      class illegal_decoration_format ;
      typedef enum { DECORATORS(VERBATIM), END_OF_LIST } decorator;
      text_modifier(const char *rules =
                    wrapped::getenv("CRPCUT_TEXT_DECORATION"));
      void write_to(writer& output, decorator m) const;
      void write_to(std::ostream& output, decorator m) const;
      std::size_t longest_decorator_len() const
      {
        return longest_decorator_len_;
      }
    private:
      void init_decorator(decorator dest, decorator src);
      datatypes::fixed_string decorators[END_OF_LIST];
      std::size_t             longest_decorator_len_;
    };

    class text_modifier::illegal_decoration_format
      : public std::runtime_error
    {
    public:
      illegal_decoration_format(const std::string &s)
        : std::runtime_error(s)
      {
      }
    };

  }
}

#endif // TEXT_MODIFIER_HPP
