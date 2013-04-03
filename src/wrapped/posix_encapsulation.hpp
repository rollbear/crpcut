/*
 * Copyright 2009-2013 Bjorn Fahller <bjorn@fahller.se>
 * All rights reserved

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


#ifndef POSIX_ENCAPSULATION_HPP
#define POSIX_ENCAPSULATION_HPP

#include <crpcut.hpp>
extern "C" {
#  include <dirent.h>
#  include <signal.h>
#  include <sys/resource.h>
}

#include "../posix_write.hpp"

namespace crpcut {
  extern unsigned use_local_heap;

  namespace wrapped {
    CRPCUT_NORETURN void _Exit(int c);
    CRPCUT_NORETURN void abort();
    int                  chdir(const char *n);
    int                  close(int);
    int                  closedir(DIR* p);
    int                  dup2(int o, int n);
    CRPCUT_NORETURN void exit(int c);
    int                  fork(void);
    void                 free(const void*);
    char *               getcwd(char *buf, size_t size);
    char *               getenv(const char*);
    int                  gethostname(char *n, size_t l);
    pid_t                getpgid(pid_t);
    pid_t                getpid();
    int                  getrusage(int, struct rusage *);
    struct tm *          gmtime(const time_t *t);
    int                  killpg(int p, int s);
    void *               malloc(size_t);
    void *               memcpy(void *d, const void *s, size_t n);
    int                  mkdir(const char *n, mode_t m);
    char *               mkdtemp(char *t);
    DIR*                 opendir(const char *n);
    int                  open(const char *, int, mode_t);
    int                  pipe(int p[2]);
    int                  readdir_r(DIR* p, struct dirent* e, struct dirent** r);
    int                  rename(const char *o, const char *n);
    int                  remove(const char *n);
    int                  rmdir(const char *n);
    int                  select(int, fd_set*, fd_set*, fd_set*, timeval *);
    int                  setpgid(pid_t pid, pid_t pgid);
    int                  setrlimit(int, const struct rlimit*);
    sighandler_t         signal(int, sighandler_t);
    int                  strncmp(const char *s1, const char *s2, size_t n);
    char *               strchr(const char *, int);
    char *               strstr(const char *, const char *);
    time_t               time(time_t *t);
    int                  waitid(idtype_t t, id_t i, siginfo_t *si, int o);
  }

  class libc_write : public posix_write
  {
    virtual ssize_t do_write(int fd, const void *p, std::size_t n);
  };
}
#endif // POSIX_ENCAPSULATION_HPP
