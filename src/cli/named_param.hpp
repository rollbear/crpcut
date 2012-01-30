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



#ifndef NAMED_PARAM_HPP_
#define NAMED_PARAM_HPP_

#include "param.hpp"

namespace crpcut {
  namespace cli {

    class named_param : public param
    {
    public:
      template <size_t N>
      named_param(char short_form, const char (&long_form)[N],
                  const char *param_description,
                  param_list &root);
      const char *value_for(const char*const *arg_list, const char *name);
    protected:
      // here this is only a sanity check to see if the form is correct
      virtual bool match_value(const char *, bool is_short);
    private:
      static const char *match_param_name(const char *param, const char *name);
    };


    template <size_t N>
    named_param::named_param(char short_form, const char (&long_form)[N],
                             const char *param_description,
                             param_list &root)
    : param(short_form, long_form,
            "name=value", param::mandatory,
            param_description, root)
    {
    }
  }
}




#endif // NAMED_PARAM_HPP_
