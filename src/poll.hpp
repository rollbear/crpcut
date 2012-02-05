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

#ifndef POLL_HPP
#define POLL_HPP
#include <cstddef>

namespace crpcut {
  template <typename T>
  class poll
  {
  public:
    struct polltype
    {
      typedef enum { r = 1, w = 2, rw = 3 } type;
    };
    class descriptor;
    poll();
    virtual ~poll();
    template <typename U>
    void add_fd(comm::file_descriptor &fd, U *data, int flags = polltype::r)
    {
      int fd_n = fd.get_fd();
      add_fd(fd_n, data, flags);
    }
    void add_fd(int fd, T* data, int flags = polltype::r)
    {
      do_add_fd(fd, data, flags);
    }
    template <typename U>
    void add_fd(U* data, int flags = polltype::r)
    {
      comm::file_descriptor *fd = data;
      int fd_n = fd->get_fd();
      add_fd(fd_n, data, flags);
    }
    void del_fd(comm::file_descriptor *fd)
    {
      int fd_n = fd->get_fd();
      del_fd(fd_n);
    }
    void del_fd(int fd)
    {
      do_del_fd(fd);
    }
    descriptor wait(int timeout_ms = -1) { return do_wait(timeout_ms); }
    std::size_t num_fds() const { return do_num_fds(); }
  private:
    virtual void do_add_fd(int fd, T* data, int flags = polltype::r) = 0;
    virtual void do_del_fd(int fd) = 0;
    virtual descriptor do_wait(int timeout_ms) = 0;
    virtual std::size_t do_num_fds() const = 0;
  };


  template <typename T>
  class poll<T>::descriptor
  {
  public:
    static const int readbit  = 1;
    static const int writebit = 2;
    static const int hupbit   = 4;
    descriptor(T* t, int m) : data_(t), mode_(m) {}
    T* operator->() const { return data_; }
    T* get() const { return data_; }
    bool read() const;
    bool write() const;
    bool hup() const;
    bool timeout() const { return mode_ == 0; }
  private:

    T* data_;
    int mode_;
  };

  template <typename T>
  poll<T>
  ::poll()
  {
  }

  template <typename T>
  poll<T>
  ::~poll()
  {
  }

  template <typename T>
  inline
  bool
  poll<T>
  ::descriptor::read() const
  {
    return mode_ & readbit;
  }

  template <typename T>
  inline
  bool
  poll<T>
  ::descriptor::write() const
  {
    return mode_ & writebit;
  }

  template <typename T>
  inline
  bool
  poll<T>
  ::descriptor::hup() const
  {
    return mode_ & hupbit;
  }

}

#endif // POLL_HPP
