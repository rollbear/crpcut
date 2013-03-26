/*
 * Copyright 2010-2013 Bjorn Fahller <bjorn@fahller.se>
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
#include <cerrno>
#include <cstring>
#include <cstdlib>
extern "C"
{
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <dirent.h>
}
#include "posix_encapsulation.hpp"

namespace crpcut {
  unsigned use_local_heap = 0;
}
namespace {
  template <typename T, std::size_t N>
  inline T* begin(T (&array)[N])
  {
    return array;
  }

  template <typename T, std::size_t N>
  inline T* end(T (&array)[N])
  {
    return array + N;
  }

  class global_heap_disabler
  {
  public:
    global_heap_disabler()
    {
      assert(!crpcut::use_local_heap);
      ++crpcut::use_local_heap;
    }
    ~global_heap_disabler()
    {
      assert(crpcut::use_local_heap);
      --crpcut::use_local_heap;
    }
  private:
    global_heap_disabler(const global_heap_disabler&);
    global_heap_disabler& operator=(const global_heap_disabler);
  };
}
namespace crpcut {
  namespace libs {
    const char * const * libc()
    {
      static const char * const name[] = {
        "libc.so",
        "libc.so.6",
        0
      };
      return name;
    }
    const char * const * librt()
    {
      static const char * const name[] = {
        "librt.so",
        "librt.so.1",
        0
      };
      return name;
    }
    const char *const * rtld_next()
    {
      return 0;
    }
  }

  namespace libwrapper
  {
    void *dlloader::load(const char *const *lib)
    {
      static const int flags[] = {
        RTLD_NOW | RTLD_NOLOAD,
        RTLD_NOW
      };
      void *libp = 0;
      global_heap_disabler disabler_obj;
      for (const int *pf = begin(flags); pf != end(flags); ++pf)
        {
          for (const char * const *name = lib; *name; ++name)
            {
              libp = ::dlopen(*name, *pf);
              if (libp) return libp;
            }
        }
      return 0;
    }

    void *dlloader::symbol(void *libp, const char *name)
    {
      global_heap_disabler disabler_obj;
      return libp ? ::dlsym(libp, name) : 0;
    }

    void dlloader::unload(void *libp)
    {
      global_heap_disabler disabler_obj;
      (void)::dlclose(libp); // nothing much to do in case of error.
    }

    void dlloader::assert_lib_is_loaded(void *libp)
    {
      if (!libp) *(int*)libp = 0; // can't rely on abort() here
    }
  }
}


#if defined(HAVE_CLOCK_GETTIME)

extern "C" {
#include <time.h>
}
namespace crpcut {
  namespace wrapped {
    CRPCUT_WRAP_FUNC(HAVE_CLOCK_GETTIME,
                     clock_gettime,
                     int,
                     (clockid_t id, struct timespec *s),
                     (id, s))
  }
}
#endif

#if defined(HAVE_EPOLL)
extern "C" {
 #include <sys/epoll.h>
}
namespace crpcut {
  namespace wrapped {
    CRPCUT_WRAP_FUNC(libc, epoll_create, int, (int n), (n))
    CRPCUT_WRAP_FUNC(libc, epoll_ctl,
                     int,
                     (int epfd, int op, int fd, struct epoll_event *ev),
                     (epfd, op, fd, ev))

    CRPCUT_WRAP_FUNC(libc, epoll_wait,
                     int,
                     (int epfd, struct epoll_event *ev, int m, int t),
                     (epfd, ev, m, t))
  }
}
#endif


namespace crpcut {
  namespace wrapped {
    CRPCUT_WRAP_FUNC(rtld_next, malloc, void*, (size_t s), (s))
    CRPCUT_WRAP_V_FUNC(rtld_next, free, void, (const void *p), (p))
    CRPCUT_WRAP_FUNC(rtld_next, calloc, void *, (size_t n, size_t s), (n, s))
    CRPCUT_WRAP_FUNC(rtld_next, realloc, void *, (void *m, size_t n), (m, n))

    CRPCUT_WRAP_FUNC(libc, chdir, int, (const char *n), (n))
    CRPCUT_WRAP_FUNC(libc, close, int, (int fd), (fd))
    CRPCUT_WRAP_FUNC(libc, closedir, int, (DIR *d), (d))
    CRPCUT_WRAP_FUNC(libc, dup2, int, (int f1, int f2), (f1, f2))
    CRPCUT_WRAP_FUNC(libc, fork, int, (void), ())
    CRPCUT_WRAP_FUNC(libc, getcwd, char*, (char *buf, size_t size), (buf, size))
    CRPCUT_WRAP_FUNC(libc, getenv, char*, (const char*n), (n))
    CRPCUT_WRAP_FUNC(libc, gethostname, int, (char *n, size_t s), (n, s))
    CRPCUT_WRAP_FUNC(libc, getitimer, int, (int i, struct itimerval *v), (i, v))
    CRPCUT_WRAP_FUNC(libc, getpgid, pid_t, (pid_t p), (p))
    CRPCUT_WRAP_FUNC(libc, getpid, int, (void), ())
    CRPCUT_WRAP_FUNC(libc, getrusage, int, (int w, struct rusage *u), (w, u))
    CRPCUT_WRAP_FUNC(libc, gettimeofday,
                     int,
                     (struct timeval *tv, struct timezone *tz),
                     (tv, tz))
    CRPCUT_WRAP_FUNC(libc, gmtime, struct tm*, (const time_t *t), (t))
    CRPCUT_WRAP_FUNC(libc, killpg, int, (int p, int s), (p, s))
    CRPCUT_WRAP_FUNC(libc, memcpy, void*,
                     (void *d, const void *s, size_t n), (d, s, n))
    CRPCUT_WRAP_FUNC(libc, mkdir, int, (const char *n, mode_t m), (n, m))
    CRPCUT_WRAP_FUNC(libc, mkdtemp, char*, (char *n), (n))
    CRPCUT_WRAP_FUNC(libc, open, int,
                     (const char *n, int m, mode_t t),
                     (n, m, t))
    CRPCUT_WRAP_FUNC(libc, opendir, DIR*, (const char *n), (n))
    CRPCUT_WRAP_FUNC(libc, pipe, int, (int p[2]), (p))
    CRPCUT_WRAP_FUNC(libc, read, ssize_t,
                     (int fd, void* p, size_t s),
                     (fd, p, s))
    CRPCUT_WRAP_FUNC(libc, readdir_r,
                     int,
                     (DIR *d, struct dirent *e, struct dirent **r),
                     (d, e, r))
    CRPCUT_WRAP_FUNC(libc, regcomp, int, (regex_t* r, const char *e, int f), (r, e, f))
    CRPCUT_WRAP_FUNC(libc, regexec,
                     int,
                     (const regex_t *r, const char *m, size_t nm, regmatch_t p[], int f),
                     (r, m, nm, p, f))
    CRPCUT_WRAP_FUNC(libc, regerror,
                     size_t,
                     (int e, const regex_t *r, char *b, size_t s),
                     (e, r, b, s))
    CRPCUT_WRAP_V_FUNC(libc, regfree,
                       void,
                       (regex_t *r),
                       (r))
    CRPCUT_WRAP_FUNC(libc, remove, int, (const char *n), (n))
    CRPCUT_WRAP_FUNC(libc, rename, int, (const char *o, const char *n), (o, n))
    CRPCUT_WRAP_FUNC(libc, rmdir, int, (const char *n), (n))
    CRPCUT_WRAP_FUNC(libc, select,
                     int,
                     (int n, fd_set *r, fd_set *w, fd_set *e, struct timeval *t),
                     (n, r, w, e, t))
    CRPCUT_WRAP_FUNC(libc, setitimer,
                     int,
                     (int n, const struct itimerval *i, struct itimerval *o),
                     (n, i, o))
    CRPCUT_WRAP_FUNC(libc, setpgid, int, (pid_t pid, pid_t pgid), (pid, pgid))
    CRPCUT_WRAP_FUNC(libc, setrlimit,
                     int,
                     (int n, const struct rlimit *r),
                     (n, r))
    CRPCUT_WRAP_FUNC(libc, signal, sighandler_t, (int s, sighandler_t h), (s, h))
    CRPCUT_WRAP_FUNC(libc, strcmp, int, (const char *l, const char *r), (l, r))
    CRPCUT_WRAP_FUNC(libc, strerror, char *, (int n), (n))
    CRPCUT_WRAP_FUNC(libc, strlen, size_t, (const char *p), (p))
    CRPCUT_WRAP_FUNC(libc, strncmp,
                     int,
                     (const char *s1, const char *s2, size_t n),
                     (s1, s2, n))
    CRPCUT_WRAP_FUNC(libc, strchr, char *, (const char *s, int c), (s, c))
    CRPCUT_WRAP_FUNC(libc, strstr, char *, (const char *h, const char *n),
                     (h, n))
    CRPCUT_WRAP_FUNC(libc, time, time_t, (time_t *p), (p))
    CRPCUT_WRAP_FUNC(libc, waitid,
                     int,
                     (idtype_t t, id_t i, siginfo_t *s, int o),
                     (t, i, s, o))
    CRPCUT_WRAP_FUNC(libc, write,
                     ssize_t,
                     (int f, const void *p, size_t s),
                     (f, p, s))
    CRPCUT_WRAP_V_FUNC(libc, _Exit, CRPCUT_NORETURN void, (int n), (n))
    CRPCUT_WRAP_V_FUNC(libc, abort, CRPCUT_NORETURN void, (void),  ())
    CRPCUT_WRAP_V_FUNC(libc, exit,  CRPCUT_NORETURN void, (int n), (n))
  }

  ssize_t
  libc_write
  ::do_write(int fd, const void *p, std::size_t n)
  {
    return wrapped::write(fd, p, n);
  }
}
