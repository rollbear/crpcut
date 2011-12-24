/*
 * Copyright 2011 Bjorn Fahller <bjorn@fahller.se>
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

namespace {
  bool match_name(const char *name,
                  const char *begin, const char *end)
  {
    const char *i = begin;
    while (i != end)
      {
        int idx = 0;
        while (i + idx != end && i[idx] != ',' && i[idx] == name[idx]) ++idx;
        if (i + idx == end || i[idx] == ',') return true;
        while (i != end && *i++ != ',')
          ;
      }
    return false;
  }
}

namespace crpcut {
  tag_filter
  ::tag_filter(const char * p)
    : begin_select(0),
      end_select(0),
      begin_noncritical(0),
      end_noncritical(0)
  {
    if (p == 0) return;

    subtract_select = *p == '-';
    if (subtract_select) ++p;
    begin_select = p;
    while (*p && *p != '/') ++p;
    end_select = p;
    if (*p++ == 0)
      {
        begin_noncritical = 0;
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
  ::lookup(const char *name) const
  {
    if (begin_select != end_select) {
      if (match_name(name, begin_select, end_select) == subtract_select)
        {
          return tag::ignored;
        }
    }
    if (*name && begin_noncritical != end_noncritical)
      {
        bool match = match_name(name, begin_noncritical, end_noncritical);
        if (match != subtract_noncritical)
          {
            return tag::non_critical;
          }
      }
    return tag::critical;
  }

}
