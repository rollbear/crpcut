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


#include "named_param.hpp"
#include "../wrapped/posix_encapsulation.hpp"
#include <cassert>
namespace crpcut {
  namespace cli {
    bool
    named_param
    ::match_value(const char *p, bool)
    {
      if (!p || *p == 0)
        {
          std::ostringstream os;
          syntax(os) << " missing a parameter name and a value";
          throw exception(os.str());
        }
      const char *name_end = match_or_end(p, '=');
      if (name_end[0] == 0 || name_end[1] == 0)
        {
          std::ostringstream os;
          syntax(os) << " missing a value after \"";
          os.write(p, name_end - p);
          os << "\"";
          throw exception(os.str());
        }
      return true;
    }

    const char *
    named_param
    ::match_param_name(const char *param, const char *name)
    {
       const size_t name_len = wrapped::strlen(name);
       const char *end = match_or_end(param, '=');
       size_t param_len = size_t(end - param);
       if (param_len == name_len
           && wrapped::strncmp(name, param, name_len) == 0)
         {
           return end + 1;
         }
       return 0;
    }

    const char *
    named_param
    ::value_for(const char *const *arg_list, const char *name)
    {
      const char *const *p = arg_list;
      while (*p)
        {
          const char *const *n = match(p);
          if (n)
            {
              const char *str = *p + 1;
              const char *param_name = str[0] == '-'
                                     ? match_or_end(str + 1, '=') + 1
                                     : *(++p);
              assert(*param_name);
              const char *rv = match_param_name(param_name, name);
              if (rv) return rv;
            }
          ++p;
        }
      return 0;
    }
  }
}






