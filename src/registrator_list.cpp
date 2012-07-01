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

#include "registrator_list.hpp"

namespace crpcut {

  void
  registrator_list
  ::run_test_case()
  {
    wrapped::abort();
  }

  tag&
  registrator_list
  ::crpcut_tag() const
  {
    return wrapped::abort(), crpcut_tag_info<crpcut::crpcut_none>::obj();
  }

  tag::importance
  registrator_list
  ::get_importance() const
  {
    return wrapped::abort(), tag::disabled;
  }

  void
  registrator_list
  ::setup(poll<fdreader> &, pid_t, int, int, int, int)
  {
    wrapped::abort();
  }

  std::pair<unsigned, unsigned>
  registrator_list
  ::filter_out_unused(const char *const *names,
                      std::ostream &err_os)
  {
    unsigned num_registered_tests = 0U;
    typedef crpcut_test_case_registrator reg;

    {
      reg *n;
      for (reg *i = next(); i != this; i = n)
        {
          n = i->next();
          ++num_registered_tests;
          switch (i->get_importance())
            {
            case tag::ignored:
              i->unlink();
              // no break
            case tag::disabled:
              i->crpcut_uninhibit_dependants();
              break;
            default:
              break; // stupid eclipse!
            }
        }
      }
    if (*names == 0)
      {
        return std::make_pair(num_registered_tests, num_registered_tests);
      }
    registrator_list result;
    unsigned num_selected_tests = 0U;
    unsigned mismatches = 0U;
    for (const char *const*name = names; *name; ++name)
      {
        unsigned matches = 0;
        reg *n;
        for (reg *i = next(); i != this; i = n)
          {
            n = i->next();
            if (i->match_name(*name))
              {
                ++matches;
                ++num_selected_tests;
                i->unlink();
                i->link_before(result);
                i->crpcut_uninhibit_dependants();
             }
          }
        if (matches == 0)
          {
            if (mismatches++)
              {
                err_os << ", ";
              }
            err_os << *name;
          }
      }
    if (mismatches)
      {
        err_os << (mismatches == 1U ? " does" : " do")
               << " not match any test names\n";
        return std::make_pair(1U,0U);
      }
    unlink();
    link_after(result);
    result.unlink();
    return std::make_pair(num_selected_tests, num_registered_tests);

  }
}
