/*
 * Copyright 2012-2013 Bjorn Fahller <bjorn@fahller.se>
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


  std::pair<unsigned, unsigned>
  registrator_list
  ::filter_out_unused(const char *const *names,
                      std::ostream &err_os)
  {
    unsigned num_registered_tests = 0U;
    typedef crpcut_test_case_registrator reg;

    {
      reg *n;
      for (reg *i = next(); !is_this(i); i = n)
        {
          n = i->next();
          ++num_registered_tests;
          tag &t = i->crpcut_tag();
          if (t.get_importance() == tag::ignored)
            {
              i->unlink();
              i->crpcut_uninhibit_dependants();
              continue;
            }
          if (i->get_importance() == tag::disabled)
            {
              i->crpcut_uninhibit_dependants();
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
        for (reg *i = next(); !is_this(i); i = n)
          {
            n = i->next();
            if (i->match_name(*name))
              {
                ++matches;
                ++num_selected_tests;
                i->unlink();
                i->link_before(result);
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

    // Ensure that tests not selected for running, do not prevent
    // selected tests from running
    for (reg *i = next(); !is_this(i); i = i->next())
      {
         if (i->get_importance() != tag::disabled)
           {
             i->crpcut_register_success(true);
           }
      }

    unlink();
    link_after(result);
    result.unlink();
    return std::make_pair(num_selected_tests, num_registered_tests);

  }

  void
  registrator_list
  ::list_tests_to(std::ostream &os, size_t tag_margin) const
  {
    int wmargin = int(tag_margin);
    if (tag_margin)
      {
        os << ' ' << std::setw(wmargin) << "tag"
           << " : test-name\n="
           << std::setfill('=')
           << std::setw(wmargin)
           << "==="
           << "============\n"
           << std::setfill(' ');
      }
    for (const crpcut_test_case_registrator *i = next();
         !is_this(i); i = i->next())
      {
        tag::importance importance = i->get_importance();

        if (importance == tag::ignored) continue;

        os << importance;
        if (tag_margin)
          {
            os << std::setw(wmargin)
               << i->crpcut_tag().get_name().str
               << " : ";
          }
        os << *i << '\n';
      }
  }

}
