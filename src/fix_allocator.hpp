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

#ifndef FIX_ALLOCATOR_HPP
#define FIX_ALLOCATOR_HPP

#include <crpcut.hpp>

namespace crpcut {
  template <typename T, size_t N>
  class fix_allocator
  {
    union elem {
      char ballast[sizeof(T)];
      elem *next;
    };
    static datatypes::array_v<elem, N> array;
    static elem *first_free;
  public:
    static void *alloc();
    static void release(void *p);
  };

  template <typename T, size_t N>
  typename fix_allocator<T, N>::elem *fix_allocator<T, N>::first_free;

  template <typename T, size_t N>
  datatypes::array_v<typename fix_allocator<T, N>::elem, N>
  fix_allocator<T, N>::array;

  template <typename T, size_t N>
  void *
  fix_allocator<T, N>::alloc()
  {
    if (first_free)
      {
        elem *p = first_free;
        first_free = p->next;
        return p;
      }
    if (array.size() < N)
      {
        array.push_back(elem());
        return &array.back();
      }
    return wrapped::malloc(sizeof(T));
  }

  template <typename T, size_t N>
  void
  fix_allocator<T, N>
  ::release(void *p)
  {
    if (p >= array.begin() && p < array.end())
      {
        elem *e = static_cast<elem*>(p);
        e->next = first_free;
        first_free = e;
      }
    else
      {
        wrapped::free(p);
      }
  }

}

#endif // FIX_ALLOCATOR_HPP
