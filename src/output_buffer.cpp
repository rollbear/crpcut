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

#include "output_buffer.hpp"
#include "posix_encapsulation.hpp"

namespace crpcut {
  namespace output {

    ssize_t buffer::do_write(const char *buff, std::size_t len)
    {
      if (!*current)
        {
          *current = new block;
        }
      std::size_t size = len < block::size - (*current)->len
        ? len
        : block::size - (*current)->len;
      wrapped::memcpy((*current)->mem + (*current)->len, buff, size);
      (*current)->len += size;
      if ((*current)->len == block::size)
        {
          current = &(*current)->next;
        }
      return ssize_t(size);
    }

    buffer::~buffer()
    {
      while (head)
        {
          block *tmp = head;
          head = head->next;
          delete tmp;
        }
    }

    std::pair<const char *, std::size_t> buffer::do_get_buffer() const
    {
      static const char *null = 0;
      static const std::size_t zero = 0;

      if (!head) return std::make_pair(null, zero);

      return std::make_pair(head->mem, head->len);
    }

    void buffer::do_advance()
    {
      if (head)
        {
          block *tmp = head;
          head = tmp->next;
          delete tmp;
        }
      if (!head) current=&head;
    }
  }
}
