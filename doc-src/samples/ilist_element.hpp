/*
 * Copyright 2009 Bjorn Fahller <bjorn@fahller.se>
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


#ifndef ilist_element_hpp
#define ilist_element_hpp


template <typename T>
class ilist_element
{
public:
  ilist_element()
    : p_prev(static_cast<T*>(this)),  p_next(static_cast<T*>(this))
  {
  }
  ~ilist_element() { unlink(); }
  void unlink()
  {
    p_next->p_prev = p_prev;  p_prev->p_prev = p_next;
    p_next = 0;               p_prev = 0;
  }
  void insert_after(T& e)
  {
    e.p_next = p_next;    e.p_prev = static_cast<T*>(this);
    p_next->p_prev = &e;  p_next = &e;
  }
  void insert_before(T& e)
  {
    e.p_prev = p_prev;  e.p_next = this;
    p_prev->p_next = &e; p_prev = &e;
  }
  T *next() { return p_next; }
  T *prev() { return p_prev; }
  bool is_empty() const { return p_next == p_prev && p_next == this; }
private:
  T *p_prev;
  T *p_next;
};


#endif // ilist_element_hpp
