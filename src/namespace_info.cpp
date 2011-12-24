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

#include <crpcut.hpp>

namespace crpcut {

  namespace_info
  ::namespace_info(const char *n, namespace_info *p)
    : name(n),
      parent(p)
  {
  }

  const char *
  namespace_info
  ::match_name(const char *n) const
  {
    if (!parent) return n;

    const char *match = parent->match_name(n);
    if (!match) return match;
    if (!*match) return match; // actually a perfect match
    if (match != n && *match++ != ':') return 0;
    if (match != n && *match++ != ':') return 0;

    const char *p = name;
    while (*p && *match && *p == *match)
      {
        ++p;
        ++match;
      }
    return *p ? 0 : match;
  }

  std::size_t
  namespace_info
  ::full_name_len() const
  {
    return (name ? wrapped::strlen(name) : 0)
      + (parent ? 2 + parent->full_name_len() : 0);
  }

  std::ostream &
  operator<<(std::ostream &os, const namespace_info &ns)
  {
    if (!ns.parent) return os;
    os << *ns.parent;
    os << ns.name;
    os << "::";
    return os;
  }

}

crpcut::namespace_info crpcut_current_namespace(0,0);
