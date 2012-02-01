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

#include "boolean_flip.hpp"
#include "../wrapped/posix_encapsulation.hpp"
#include <sstream>
#include <typeinfo>
namespace {
  const char *truth_values[][2] = {
                                  { "N",     "Y"    },
                                  { "n",     "y"    },
                                  { "No",    "Yes"  },
                                  { "no",    "yes"  },
                                  { "NO",    "YES"  },
                                  { "FALSE", "TRUE" },
                                  { "false", "true" },
                                  { "False", "True" },
                                  { "off",   "on"   },
                                  { "OFF",   "ON"   },
                                  { "Off",   "On"   }
  };

  template <typename T, size_t N>
  T* begin(T (&array)[N])
  {
    return array;
  }

  template <typename T, size_t N>
  T *end(T (&array)[N])
  {
    return array + N;
  }
}
namespace crpcut {
  namespace cli {
    bool
    boolean_flip::get_value(bool b) const
    {
      if (value_ == set_true) return true;
      if (value_ == set_false) return false;
      return b == (value_ == uninitialized);
    }

    boolean_flip
    ::operator boolean_flip::bool_type() const
    {
      return seen_ ? &boolean_flip::match_value : 0;
    }
    bool
    boolean_flip::match_value(const char *p, bool short_form)
    {
      if (value_ == set_true || value_ == set_false)
        {
          std::ostringstream os;
          syntax(os) << " is already set";
          throw exception(os.str());
        }
      if (p && *p != 0)
        {
          typedef char const * (*array)[2];
          for (array t = begin(truth_values); t != end(truth_values); ++t)
            {
              for (int i = 0; i < 2; ++i)
                {
                  if (wrapped::strcmp((*t)[i], p) == 0)
                    {
                      value_ = i ? set_true : set_false;
                      seen_ = true;
                      return true;
                    }
                }
            }
          if (!short_form)
            {
              std::ostringstream os;
              syntax(os) << " - can't interpret \"" << p << "\" as boolean value";
              throw exception(os.str());
            }
        }
      value_ = (value_ == uninitialized ? flipped : uninitialized);
      seen_ = true;
      return false;
    }
  }
}







