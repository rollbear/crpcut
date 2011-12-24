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

#ifndef LIST_ELEM_HPP
#define LIST_ELEM_HPP

namespace crpcut {
  template <typename T>
  class list_elem
  {
  public:
    list_elem();
    list_elem(T *p);
    virtual ~list_elem();
    void link_after(list_elem& r);
    void link_before(list_elem &r);
    T *next() { return next_; }
    T *prev() { return prev_; }
    bool is_empty() const;
  private:
    void unlink();
    list_elem(const list_elem&);
    list_elem& operator=(const list_elem&);
    T *next_;
    T *prev_;
  };

  template <typename T>
  inline list_elem<T>::list_elem()
    : next_(static_cast<T*>(this)),
      prev_(static_cast<T*>(this))
  {
  }

  template <typename T>
  inline list_elem<T>::list_elem(T *p)
    : next_(p),
      prev_(p)
  {
  }

  template <typename T>
  inline list_elem<T>::~list_elem()
  {
    unlink();
  }

  template <typename T>
  inline void list_elem<T>::link_after(list_elem& r)
  {
    next_ = r.next_;
    prev_ = static_cast<T*>(&r);
    next_->prev_ = static_cast<T*>(this);
    r.next_ = static_cast<T*>(this);
  }

  template <typename T>
  inline void list_elem<T>::link_before(list_elem &r)
  {
    prev_ = r.prev_;
    next_ = static_cast<T*>(&r);
    prev_->next_ = static_cast<T*>(this);
    r.prev_ = static_cast<T*>(this);
  }

  template <typename T>
  inline bool list_elem<T>::is_empty() const
  {
    return next_ == static_cast<const T*>(this);
  }

  template <typename T>
  inline void list_elem<T>::unlink()
  {
    T *n = next_;
    T *p = prev_;
    n->prev_ = p;
    p->next_ = n;
    prev_ = static_cast<T*>(this);
    next_ = static_cast<T*>(this);
  }
}

#endif // LIST_ELEM_HPP
