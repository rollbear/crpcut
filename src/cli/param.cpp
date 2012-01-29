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


#include "param.hpp"
#include "../wrapped/posix_encapsulation.hpp"
#include <cassert>
#include <sstream>
namespace {
  const char *match_or_end(const char *p, char c)
  {
    assert(p);
    while (*p && *p != c) ++p;
    return p;
  }
}

namespace crpcut {
  namespace cli {
    const char *const *
    param
    ::match(const char *const *p)
    {
      assert(p);
      if (*p == 0) return 0;
      const char *param = *p++;
      if (*param != '-') return 0;
      if (param[1] == short_form_ && param[2] == 0)
        {
          return p + match_value(*p, true);
        }
      if (param[1] != '-') return 0;
      param+= 2;
      const char *value = match_or_end(param, '=');
      size_t len = size_t(value - param);
      if (len != long_form_len_ || wrapped::strncmp(long_form_, param, len) != 0)
        {
          return 0;
        }
      match_value(*value ? value + 1 : 0, false);
      return p;
    }

    param
    ::~param()
    {
    }

    std::ostream&
    param
    ::syntax(std::ostream &os) const
    {
      os << '-' << short_form_;
      if (value_description_)
        {
          os << " " << value_description_;
        }
      os << " / --" << long_form_;
      if (value_description_)
        {
          os << '=' << value_description_;
        }
      return os;
    }

    bool
    param
    ::match_value(const char *p, bool is_short)
    {
      if (!p) return false;
      if (is_short) return false;
      if (*p == '-') return false;
      std::ostringstream os;
      syntax(os) << " does not accept any value";
      throw exception(os.str());
    }

    std::ostream &
    param
    ::print_to(std::ostream &os) const
    {
      os << "   ";
      syntax(os);
      const char *begin = param_description_;
      while (*begin)
        {
          const char *p;
          for (p = begin; *p && *p != '\n'; ++p)
            ;
          os << "\n        ";
          os.write(begin, p - begin);
          begin = p + (*p == '\n');
        }
      return os;
    }
  }
}






