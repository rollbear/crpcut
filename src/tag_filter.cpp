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

#include "tag_filter.hpp"
#include "algorithm/split_range.hpp"
#include <algorithm>

namespace {

  void
  assert_name_list(const char *begin, const char*end, crpcut::tag_list_root &l)
  {
    for (auto [b,e] : crpcut::split_range(begin,end,','))
    {
      std::string_view needle{b, std::size_t(e-b)};
      auto matches_name = [needle](auto& tag){
        return needle == tag.get_name();
      };
      if (!std::any_of(l.begin(), l.end(), matches_name))
      {
        auto msg = std::string(needle) + " is not a tag";
        throw crpcut::tag_filter::spec_error(msg);
      }
    }
  }

  bool match_name(std::string_view name,
                  const char *begin, const char *end)
  {
    for (auto [b,e] : crpcut::split_range(begin,end,','))
    {
      if (std::string_view(b, std::size_t(e - b)) == name) return true;
    }
    return false;
  }
}

namespace crpcut {

  tag_filter
  ::tag_filter(const char * p)
  {
    if (p == nullptr) return;

    subtract_select = *p == '-';
    if (subtract_select) ++p;
    begin_select = p;
    while (*p && *p != '/') ++p;
    end_select = p;
    if (*p++ == 0)
      {
        begin_noncritical = nullptr;
        return;
      }
    subtract_noncritical = *p == '-';
    if (subtract_noncritical) ++p;
    begin_noncritical = p;
    while (*p) ++p;
    end_noncritical = p;
  }

  tag::importance
  tag_filter
  ::lookup(datatypes::fixed_string name) const
  {
    if (begin_select != end_select) {
      if (match_name(name, begin_select, end_select) == subtract_select)
        {
          return tag::ignored;
        }
    }
    if (name && begin_noncritical != end_noncritical)
      {
        bool match = match_name(name, begin_noncritical, end_noncritical);
        if (match != subtract_noncritical)
          {
            return tag::non_critical;
          }
      }
    return tag::critical;
  }

  void
  tag_filter
  ::assert_names(tag_list_root &l) const
  {
    assert_name_list(begin_select,      end_select,      l);
    assert_name_list(begin_noncritical, end_noncritical, l);
  }
}
