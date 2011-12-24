/*
 * Copyright 2011 Bjorn Fahller <bjorn@fahller.se>
 * All rights reserved

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
  collate_result
  ::collate_result(const char *refstr, std::string comp, const std::locale& l)
    : r(refstr),
      intl(comp),
      locale(l),
      side(right)
  {
  }

  collate_result
  ::collate_result(const collate_result& o)
    : r(o.r),
      intl(o.intl),
      locale(o.locale),
      side(o.side)
  {
  }

  collate_result
  ::operator const collate_result::comparator*() const
  {
    return operator()();
  }

  const collate_result::comparator*
  collate_result
  ::operator()() const
  {
    return reinterpret_cast<const comparator*>(r ? 0 : this);
  }

  collate_result&
  collate_result
  ::set_lh()
  {
    side = left;
    return *this;
  }

  std::ostream &operator<<(std::ostream& os, const collate_result &obj)
  {
    static const char rs[] = "\"\n"
      "  and right hand value = \"";
    os << "Failed in locale \"" << obj.locale.name() << "\"\n"
      "  with left hand value = \"";
    if (obj.side == collate_result::right)
      {
        os << obj.intl << rs << obj.r << "\"";
      }
    else
      {
        os << obj.r << rs << obj.intl << "\"";
      }
    return os;
  }
}
