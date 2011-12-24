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
  tag::tag()
    : next_(this),
      prev_(this),
      failed_(0),
      passed_(0),
      importance_(critical)
  {
  }

  tag::tag(int len, tag *n)
    : next_(n),
      prev_(n->prev_),
      failed_(0),
      passed_(0),
      importance_(critical)
  {
    n->prev_ = this;
    prev_->next_ = this;
    if (len > longest_name_len_) longest_name_len_ = len;
  }

  tag::~tag()
  {
    tag *p = prev_;
    next_->prev_ = prev_;
    p->next_ = next_;
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

  tag *tag::get_next() const
  {
    return next_;
  }

  tag *tag::get_prev() const
  {
    return prev_;
  }

  void tag::set_importance(tag::importance i)
  {
    importance_ = i;
  }

  tag::importance tag::get_importance() const
  {
    return importance_;
  }

  int tag::longest_name_len_;
}
