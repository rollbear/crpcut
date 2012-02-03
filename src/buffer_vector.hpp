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



#ifndef BUFFER_VECTOR_HPP_
#define BUFFER_VECTOR_HPP_

#include <cstddef>
#include <cassert>
namespace crpcut {
  template <typename T>
  class buffer_vector
  {
  public:
    static size_t space_for(size_t num_elements);
    buffer_vector(void *storage, std::size_t capacity);
    ~buffer_vector();
    T& at(std::size_t idx);
    const T& at(std::size_t idx) const;
    T& push_back(const T&);
    void pop_back();
    T& front();
    const T& front() const;
    T& back();
    const T& back() const;
    size_t size() const;

    T* begin();
    const T* begin() const;
    T* end();
    const T* end() const;
  private:
    buffer_vector();
    buffer_vector(const buffer_vector &);
    buffer_vector& operator=(const buffer_vector&);
    void *address(size_t idx) const;
    void *storage_;
    std::size_t capacity_;
    std::size_t size_;
  };

  template <typename T>
  size_t buffer_vector<T>::space_for(std::size_t num_elements)
  {
    return num_elements * sizeof(T);
  }

  template <typename T>
  buffer_vector<T>::buffer_vector(void *storage, std::size_t capacity)
  : storage_(storage),
    capacity_(capacity),
    size_(0)
  {
    assert(storage);
    assert(capacity);
  }

  template <typename T>
  buffer_vector<T>::~buffer_vector()
  {
    while (size_ > 0U)
      {
        pop_back();
      }
  }
  template <typename T>
  T& buffer_vector<T>::at(std::size_t idx)
  {
    assert(idx < size_);
    return *static_cast<T*>(address(idx));
  }

  template <typename T>
  const T& buffer_vector<T>::at(std::size_t idx) const
  {
    assert(idx < size_);
    return *static_cast<const T*>(address(idx));
  }

  template <typename T>
  T& buffer_vector<T>::push_back(const T& t)
  {
    assert(size_ < capacity_);
    T* rv = new (address(size_++)) T(t);
    return *rv;
  }

  template <typename T>
  void buffer_vector<T>::pop_back()
  {
    assert(size_ > 0U);
    static_cast<T*>(address(--size_))->~T();
  }

  template <typename T>
  size_t buffer_vector<T>::size() const
  {
    return size_;
  }

  template <typename T>
  T& buffer_vector<T>::front()
  {
    assert(size_ > 0U);
    return at(0U);
  }

  template <typename T>
  const T& buffer_vector<T>::front() const
  {
    assert(size_ > 0U);
    return at(0U);
  }
  template <typename T>
  T& buffer_vector<T>::back()
  {
    assert(size_ > 0U);
    return at(size_ - 1U);
  }

  template <typename T>
  const T& buffer_vector<T>::back() const
  {
    assert(size_ > 0U);
    return at(size_ - 1U);
  }


  template <typename T>
  T* buffer_vector<T>::begin()
  {
    assert(size_ > 0U);
    return &at(0U);
  }

  template <typename T>
  const T* buffer_vector<T>::begin() const
  {
    assert(size_ > 0U);
    return &at(0U);
  }

  template <typename T>
  T* buffer_vector<T>::end()
  {
    return static_cast<T*>(address(size_));
  }

  template <typename T>
  const T* buffer_vector<T>::end() const
  {
    return static_cast<const T*>(address(size_));
  }

  template <typename T>
  void *buffer_vector<T>::address(size_t idx) const
  {
    return static_cast<T*>(storage_) + idx;
  }
}


#endif // BUFFER_VECTOR_HPP_
