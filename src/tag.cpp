/*
 * Copyright 2011-2013 Bjorn Fahller <bjorn@fahller.se>
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
  tag::tag()
    : failed_(0),
      passed_(0),
      importance_(critical)
  {
  }

  tag::tag(size_t len, tag_list_root *list)
    : failed_(0),
      passed_(0),
      importance_(critical)
  {
    link_before(*list);
    list->store_name_length(len);
  }

  tag::~tag()
  {
  }

  void tag::fail()
  {
    ++failed_;
  }

  void tag::pass()
  {
    ++passed_;
  }

  size_t tag::num_failed() const
  {
    return failed_;
  }

  size_t tag::num_passed() const
  {
    return passed_;
  }


  void tag::set_importance(tag::importance i)
  {
    importance_ = i;
  }

  tag::importance tag::get_importance() const
  {
    return importance_;
  }


#define VERBATIM_SECOND(p1, p2) p2

  std::ostream &operator<<(std::ostream &os, tag::importance i)
  {
    static const char icon[] = { CRPCUT_TEST_IMPORTANCE(VERBATIM_SECOND) };
    return os << icon[i];
  }
}
