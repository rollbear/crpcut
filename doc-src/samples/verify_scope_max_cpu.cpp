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


#include <crpcut.hpp>
extern "C"
{
#  include <unistd.h>
#  include <sys/times.h>
}
TEST(long_real_time_short_cpu_time)
{
  VERIFY_SCOPE_MAX_CPUTIME_MS(3)
  {
    for (int i = 0; i < 50; ++i)
      {
        usleep(1000); // would fail if implemented as busy wait
      }
  }
}

TEST(short_real_time_long_cpu_time)
{
  const clock_t clocks_per_tick = sysconf(_SC_CLK_TCK);
  tms t;
  times(&t);
  clock_t deadline = t.tms_utime + t.tms_stime + clocks_per_tick/10;
  VERIFY_SCOPE_MAX_CPUTIME_MS(100)
  {
    for (;;)
      {
        for (volatile int n = 0; n < 100000; ++n)
          ;
        times(&t);
        if (t.tms_utime + t.tms_stime > deadline) break;
      }
  }
  INFO << "after violation";
}

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
