/*
 * Copyright 2013 Bjorn Fahller <bjorn@fahller.se>
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



#ifndef POLL_BUFFER_VECTOR_HPP_
#define POLL_BUFFER_VECTOR_HPP_


#include "poll.hpp"
#include "buffer_vector.hpp"

extern "C"
{
#  include <errno.h>
#  include <sys/select.h>
}
namespace crpcut {
  namespace wrapped {
    int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
  }

  struct polldata
  {
    polldata(void *p, size_t capacity);
    struct fdinfo
    {
      struct has_fd
      {
        has_fd(int num) : fd_(num) {}
        bool operator()(const fdinfo& i) const { return fd_ == i.fd; }
      private:
        int fd_;
      };
      fdinfo(int fd_ = 0, int mode_ = 0, void *ptr_ = 0)
        : fd(fd_), mode(mode_), ptr(ptr_)
      {
      }
      int   fd;
      int   mode;
      void *ptr;
    };
    buffer_vector<fdinfo> access;
    size_t pending_fds;
    fd_set rset;
    fd_set wset;
    fd_set xset;
  };

  template <typename T>
  class poll_buffer_vector : public poll<T>,
                             private polldata
  {
  public:
    typedef typename poll<T>::descriptor descriptor;
    typedef typename poll<T>::polltype   polltype;
    static std::size_t space_for(std::size_t capacity)
    {
      return buffer_vector<fdinfo>::space_for(capacity);
    }
    poll_buffer_vector(void *p, std::size_t capacity);
    virtual ~poll_buffer_vector();
  private:
    virtual void do_add_fd(int fd, T* data, int flags = polltype::r);
    virtual void do_del_fd(int fd);
    virtual descriptor do_wait(int timeout_ms);
    virtual size_t do_num_fds() const;
  };

  template <typename T>
  inline poll_buffer_vector<T>::poll_buffer_vector(void *p, std::size_t capacity)
  :  polldata(p, capacity)
  {
  }

  template <typename T>
  inline poll_buffer_vector<T>::~poll_buffer_vector()
  {
  }

  template <typename T>
  inline
  void
  poll_buffer_vector<T>
  ::do_add_fd(int fd, T* data, int flags)
  {
    access.push_back(fdinfo(fd, flags, data));
  }

  template <typename T>
  inline
  void
  poll_buffer_vector<T>
  ::do_del_fd(int fd)
  {
    fdinfo *i = 0;
    for (size_t idx = 0; idx < access.size(); ++idx)
      {
        fdinfo &info = access.at(idx);
        if (info.fd == fd) { i = &info; break; }
      }
    assert(i != 0 && "fd not found");
    *i = access.back();
    access.pop_back();
    if (   FD_ISSET(fd, &this->xset)
           || FD_ISSET(fd, &this->rset)
           || FD_ISSET(fd, &this->wset))
      {
        FD_CLR(fd, &this->rset);
        FD_CLR(fd, &this->wset);
        FD_CLR(fd, &this->xset);
        --this->pending_fds;
      }
  }

  template <typename T>
  inline
  typename poll_buffer_vector<T>::descriptor
  poll_buffer_vector<T>
  ::do_wait(int timeout_ms)
  {
    if (this->pending_fds == 0)
      {
        int maxfd = 0;
        for (size_t i = 0; i < access.size(); ++i)
          {
            int fd = this->access.at(i).fd;
            if (fd > maxfd) maxfd = fd;
            if (access.at(i).mode & polltype::r) FD_SET(fd, &rset);
            if (access.at(i).mode & polltype::w) FD_SET(fd, &wset);
            FD_SET(fd, &xset);
          }
        struct timeval tv = { timeout_ms / 1000, (timeout_ms % 1000) * 1000 };
        for (;;)
          {
            int rv = wrapped::select(maxfd + 1,
                                     &rset,
                                     &wset,
                                     &xset,
                                     timeout_ms == -1 ? 0 : &tv);
            if (rv == -1 && errno == EINTR) continue;
            if (rv < 0) throw posix_error(errno, "select");
            if (rv == 0) return descriptor(0,0); // timeout
            this->pending_fds = size_t(rv);
            break;
          }
      }
    for (size_t j = 0; j < access.size(); ++j)
      {
        int fd = access.at(j).fd;
        int mode = 0;
        if (FD_ISSET(fd, &rset))
          {
            mode|= descriptor::readbit;
            FD_CLR(fd, &rset);
          }
        if (FD_ISSET(fd, &wset))
          {
            mode|= descriptor::writebit;
            FD_CLR(fd, &wset);
          }
        if (FD_ISSET(fd, &xset))
          {
            mode|= descriptor::hupbit;
            FD_CLR(fd, &xset);
          }
        if (mode)
          {
            --this->pending_fds;
            return descriptor(static_cast<T*>(access.at(j).ptr), mode);
          }
      }
    assert("no matching fd" == 0);
    return descriptor(0, 0);
  }

  template <typename T>
  inline
  size_t
  poll_buffer_vector<T>
  ::do_num_fds() const
  {
    return access.size();
  }

  inline polldata::polldata(void *p, std::size_t capacity)
    : access(p, capacity),
      pending_fds(0U)
  {
    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_ZERO(&xset);
  }

}


#endif // POLL_BUFFER_VECTOR_HPP_
