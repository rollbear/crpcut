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

#include <crpcut.hpp>

namespace crpcut {
  crpcut_tag_info<crpcut::crpcut_none>
  ::crpcut_tag_info()
    : tag()
  {
  }

  const char*
  crpcut_tag_info<crpcut_none>
  ::get_name() const
  {
    return "";
  }

  crpcut_tag_info<crpcut_none>&
  crpcut_tag_info<crpcut_none>::obj()
  {
    static crpcut_tag_info<crpcut_none> t;
    return t;
  }

  crpcut_tag_info<crpcut_none>::iterator
  ::iterator(crpcut::tag *p_)
    : p(p_)
  {
  }

  crpcut_tag_info<crpcut_none>::iterator&
  crpcut_tag_info<crpcut_none>::iterator
  ::operator++()
  {
    p = p->get_next();
    return *this;
  }

  crpcut_tag_info<crpcut_none>::iterator
  crpcut_tag_info<crpcut_none>::iterator
  ::operator++(int)
  {
    iterator rv(*this);
    ++(*this);
    return rv;
  }

  crpcut::tag *
  crpcut_tag_info<crpcut_none>::iterator
  ::operator->()
  {
    return p;
  }

  crpcut::tag&
  crpcut_tag_info<crpcut_none>::iterator
  ::operator&()
  {
    return *p;
  }

  bool
  crpcut_tag_info<crpcut_none>::iterator
  ::operator==(const iterator &i) const
  {
    return p == i.p;
  }

  bool
  crpcut_tag_info<crpcut_none>::iterator
  ::operator!=(const iterator &i) const
  {
    return p != i.p;
  }

  crpcut_tag_info<crpcut_none>::iterator
  crpcut_tag_info<crpcut_none>
  ::begin()
  {
    return iterator(obj().get_next());
  }

  crpcut_tag_info<crpcut_none>::iterator
  crpcut_tag_info<crpcut_none>
  ::end()
  {
    return iterator(&obj());
  }

  int
  crpcut_tag_info<crpcut_none>::
  longest_name_len()
  {
    return tag::longest_name_len_;
  }

}
