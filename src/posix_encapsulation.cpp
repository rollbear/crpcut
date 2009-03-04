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
#include <dirent.h>
#include <stdarg.h>
}

namespace crpcut {
  namespace libwrapper {
    template <>
    const char *traits<libs::libc>::name = "libc.so.6";

    template <>
    const char *traits<libs::librt>::name = "librt.so";
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
    CRPCUT_WRAP_FUNC(libc, chdir, int, (const char *n), (n))
    CRPCUT_WRAP_FUNC(libc, close, int, (int fd), (fd))
    CRPCUT_WRAP_FUNC(libc, closedir, int, (DIR *d), (d))
    CRPCUT_WRAP_FUNC(libc, dup2, int, (int f1, int f2), (f1, f2))
    CRPCUT_WRAP_FUNC(libc, fork, int, (void), ())
    CRPCUT_WRAP_FUNC(libc, getcwd, char*, (char *buf, size_t size), (buf, size))
    CRPCUT_WRAP_FUNC(libc, gethostname, int, (char *n, size_t s), (n, s))
    CRPCUT_WRAP_FUNC(libc, getitimer, int, (int i, struct itimerval *v), (i, v))
    CRPCUT_WRAP_FUNC(libc, getpid, int, (void), ())
    CRPCUT_WRAP_FUNC(libc, gettimeofday,
                     int,
                     (struct timeval *tv, struct timezone *tz),
                     (tv, tz))
    CRPCUT_WRAP_FUNC(libc, gmtime, struct tm*, (const time_t *t), (t))
    CRPCUT_WRAP_FUNC(libc, kill, int, (pid_t p, int s), (p, s))
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
    CRPCUT_WRAP_FUNC(libc, setrlimit,
                     int,
                     (int n, const struct rlimit *r),
                     (n, r))

    CRPCUT_WRAP_FUNC(libc, vsnprintf,
                     int,
                     (char *d, size_t s, const char *f, va_list ap),
                     (d, s, f, ap))

    int snprintf(char *s, size_t si, const char *f, ...)
    {
      va_list ap;
      va_start(ap, f);
      int r = vsnprintf(s, si, f, ap);
      va_end(ap);
      return r;
    }


    CRPCUT_WRAP_FUNC(libc, strcmp, int, (const char *l, const char *r), (l, r))
    CRPCUT_WRAP_FUNC(libc, strcpy, char *, (char *l, const char *r), (l, r))
    CRPCUT_WRAP_FUNC(libc, strerror, char *, (int n), (n))
    CRPCUT_WRAP_FUNC(libc, strlen, size_t, (const char *p), (p))
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
}