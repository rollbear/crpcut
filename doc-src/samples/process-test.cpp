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

#include "process-example.hpp"
#include <crpcut.hpp>
#include <cerrno>
#include <list>
#include <map>

namespace original
{
  CRPCUT_WRAP_FUNC(libc, fork, pid_t, (void), ())
  CRPCUT_WRAP_FUNC(libc, close, int, (int fd), (fd))
  CRPCUT_WRAP_FUNC(libc, pipe, int, (int fd[2]), (fd))
  CRPCUT_WRAP_FUNC(libc, wait, pid_t, (void *status), (status))
  CRPCUT_WRAP_FUNC(libc, read, ssize_t, (int fd, void *p, size_t n), (fd, p, n))
  CRPCUT_WRAP_FUNC(libc, write, ssize_t, (int fd, const void *p, size_t n), (fd, p, n))
}

//
// data structures used to control how the wrappers behave
//

struct pipe_data
{
  pipe_data(int retval_, int *rfd_, int *wfd_, int err_)
    : retval(retval_), rfd(rfd_), wfd(wfd_), err(err_) {}
  int  retval;
  int *rfd;
  int *wfd;
  int  err;
};

std::list<pipe_data> pipe_actions;

struct fork_data
{
  fork_data(pid_t retval_, int err_)
    : retval(retval_), err(err_) {}
  pid_t retval;
  int   err;
};

std::list<fork_data> fork_actions;

struct read_data
{
  read_data(ssize_t retval_, int *efd, const void *data_, int err_)
    : retval(retval_), expected_fd(efd), data(data_), err(err_) {}
  ssize_t     retval;
  int        *expected_fd;
  const void *data;
  int         err;
};

std::list<read_data> read_actions;

struct write_data
{
  write_data(ssize_t retval_, int *efd, const void *data, size_t len, int err_)
    : retval(retval_), expected_fd(efd), expected_data(data),
      expected_len(len), err(err_) {}
  ssize_t     retval;
  int        *expected_fd;
  const void *expected_data;
  size_t      expected_len;
  int         err;
};

std::list<write_data> write_actions;


struct wait_data
{
  wait_data(int retval_, pid_t pid_, int err_)
    : retval(retval_), pid(pid_), err(err_) {}
  int        retval;
  pid_t      pid;
  int        err;
};

std::list<wait_data> wait_actions;

enum fdstatus { unknown, is_pipe, is_closed }; 
std::map<int, fdstatus> file_descriptors;


//
// wrapper implementations
//

extern "C" pid_t fork() throw ()
{
  if (fork_actions.empty()) return original::fork();
  const fork_data &a = fork_actions.front();
  errno = a.err;
  int rv = a.retval;
  fork_actions.pop_front();
  return rv;
}

extern "C" ssize_t read(int fd, void *p, size_t n)
{
  if (read_actions.empty()) return original::read(fd, p, n);
  const read_data &a = read_actions.front();
  if (a.expected_fd)
    {
      ASSERT_EQ(fd, *a.expected_fd);
    }
  ssize_t rv = a.retval;
  if (rv != -1)
    {
      size_t srv = size_t(rv);
      ASSERT_GE(n, srv);
      memcpy(p, a.data, srv);
    }
  errno = a.err;
  read_actions.pop_front();
  return rv;
}

extern "C" ssize_t write(int fd, const void *p, size_t n)
{
  if (write_actions.empty()) return original::write(fd, p, n);
  const write_data &a = write_actions.front();
  if (a.expected_fd)
    {
      ASSERT_EQ(fd, *a.expected_fd);
    }
  ssize_t rv = a.retval;
  if (rv >= 0)
    {
      ASSERT_EQ(n, size_t(rv));
      if (::memcmp(a.expected_data, p, n) != 0)
        {
          FAIL << "write() data differs from expected";
        }
    }
  errno = a.err;
  write_actions.pop_front();
  return rv;
}

extern "C" int pipe(int fd[2]) throw ()
{
  int rv;
  if (pipe_actions.empty() || pipe_actions.front().retval == 0)
    {
      rv = original::pipe(fd);
      if (rv == 0)
        {
          file_descriptors[fd[0]] = is_pipe;
          file_descriptors[fd[1]] = is_pipe;
          if (!pipe_actions.empty())
            {
              *pipe_actions.front().rfd = fd[0];
              *pipe_actions.front().wfd = fd[1];
            }
        }
    }
  else
    {
      rv = pipe_actions.front().retval;
      errno = pipe_actions.front().err;
    }
  if (!pipe_actions.empty()) pipe_actions.pop_front();
  return rv;
}

extern "C" int close(int fd)
{
  int rv = original::close(fd);
  ASSERT_NE(file_descriptors[fd], is_closed);
  file_descriptors[fd] = is_closed;
  return rv;
}

extern "C" pid_t wait(void *p)
{
  if (wait_actions.empty()) return original::wait(p);
  const wait_data &a = wait_actions.front();
  int rv = a.retval;
  errno = a.err;
  *static_cast<int*>(p) = a.pid;
  wait_actions.pop_front();
  return rv;
}


TEST(fork_fails, EXPECT_EXCEPTION(work::fork_exception))
{
  fork_actions.push_back(fork_data(-1, ENOMEM));
  work obj;
}

TEST(pipe_fails, EXPECT_EXCEPTION(work::pipe_exception))
{
  pipe_actions.push_back(pipe_data(-1, 0, 0, EMFILE));
  work obj;
}

static const char parent_str[] = "hello parent!";

struct read_fixture
{
  read_fixture()
    :  len(sizeof(parent_str) - 1),
       zero(0)
  {
    pipe_actions.push_back(pipe_data(0, &rfd, &wfd, 0));
    write_actions.push_back(write_data(sizeof(len),  &wfd, &len,
                                       sizeof(len), 0));
    fork_actions.push_back(fork_data(0x3ffff3, 0));
    wait_actions.push_back(wait_data(0, 0x3ffff3, 0));

    read_actions.push_back(read_data(sizeof(len),  &rfd, &len, 0));
    read_actions.push_back(read_data(len,          &rfd,  parent_str,  0));
    read_actions.push_back(read_data(sizeof(zero), &rfd, &zero,        0));
  }
  const ssize_t len;
  const ssize_t zero;
  int rfd;
  int wfd;
};

TEST(read_one_string, read_fixture)
{
  {
    work obj;
    ASSERT_EQ(file_descriptors[wfd], is_closed);
    std::string s = obj.get_data();
    ASSERT_EQ(s, parent_str);
    s = obj.get_data();
    ASSERT_EQ(s, "");
    obj.wait();
  }
  ASSERT_EQ(file_descriptors[rfd], is_closed);
}

TEST(signal_on_string_read, read_fixture)
{
  read_actions.insert(++read_actions.begin(),
                      read_data(-1, &rfd, 0, EINTR));

  {
    work obj;
    ASSERT_EQ(file_descriptors[wfd], is_closed);
    std::string s = obj.get_data();
    ASSERT_EQ(s, parent_str);
    s = obj.get_data();
    ASSERT_EQ(s, "");
    obj.wait();
  }
  ASSERT_EQ(file_descriptors[rfd], is_closed);
}

TEST(normal_work)
{
  work obj;
  std::string s = obj.get_data();
  INFO << "s=\"" << s << "\"";
  ASSERT_EQ(s, "");
  obj.wait();
}

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
