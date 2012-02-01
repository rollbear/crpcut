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



#ifndef BOOLEAN_FLIP_HPP_
#define BOOLEAN_FLIP_HPP_

#include "param.hpp"

namespace crpcut {
  namespace cli {
    class boolean_flip : public param
    {
      typedef bool (boolean_flip::*bool_type)(const char *, bool);
    public:
      template <size_t N>
      boolean_flip(char short_form, const char (&long_form)[N],
                   const char *param_description,
                   param_list &root);
      bool get_value(bool b) const;
      operator bool_type() const;
    protected:
      virtual bool match_value(const char *, bool);
    private:
      enum { uninitialized, flipped, set_false, set_true  } value_;
      bool seen_;
    };

    template <size_t N>
    boolean_flip::boolean_flip(char short_form, const char (&long_form)[N],
                               const char *param_description,
                               param_list &root)
    : param(short_form, long_form,
            "boolean value", param::optional,
            param_description,
            root),
      value_(uninitialized),
      seen_(false)
    {
    }

  }
}





#endif // BOOLEAN_FLIP_HPP_
