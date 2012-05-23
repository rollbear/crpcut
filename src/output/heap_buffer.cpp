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

#include "heap_buffer.hpp"
#include "../wrapped/posix_encapsulation.hpp"

namespace crpcut {
  namespace output {

    struct heap_buffer::block
    {
      block() :next_(0), len_(0) {}
      static const std::size_t size = 128;

      block       *next_;
      char         mem_[size];
      std::size_t  len_;
    };

    heap_buffer
    ::heap_buffer()
      : head_(0),
        current_(&head_)
    {
    }

    ssize_t
    heap_buffer
    ::write(const char *buff, std::size_t len)
    {
      if (!*current_)
        {
          *current_ = new block;
        }
      std::size_t size = std::min(len, block::size - (*current_)->len_);
      wrapped::memcpy((*current_)->mem_ + (*current_)->len_, buff, size);
      (*current_)->len_ += size;
      if ((*current_)->len_ == block::size)
        {
          current_ = &(*current_)->next_;
        }
      return ssize_t(size);
    }

    heap_buffer
    ::~heap_buffer()
    {
      while (head_)
        {
          block *tmp = head_;
          head_ = head_->next_;
          delete tmp;
        }
    }

    std::pair<const char *, std::size_t>
    heap_buffer
    ::get_buffer() const
    {
      static const char *null = 0;
      static const std::size_t zero = 0;

      if (!head_) return std::make_pair(null, zero);

      return std::make_pair(head_->mem_, head_->len_);
    }

    void
    heap_buffer
    ::advance()
    {
      if (head_)
        {
          block *tmp = head_;
          head_ = tmp->next_;
          if (current_ == &tmp->next_) current_ = &head_;
          delete tmp;
        }
      if (!head_) current_ = &head_;
    }

    inline
    bool
    heap_buffer
    ::is_empty() const
    {
      return !head_;
    }
  }
}
