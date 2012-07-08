/*
 * Copyright 2009 Bjorn Fahller <bjorn@fahller.se>
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

#ifndef CLOCKS_HPP
#define CLOCKS_HPP
namespace crpcut {
  namespace clocks {
    class monotonic
    {
    public:
      static const monotonic &obj();
      virtual ~monotonic();
      typedef unsigned long timestamp;
      virtual timestamp now() const;
      static timestamp timestamp_absolute() { return func(); }
      static const char *get_name() { return name; }
      typedef timestamp (*timestamp_func)();
    private:
      static timestamp_func func;
      static const char    *name;

      static timestamp_func try_mach_high_res_timer();
      static timestamp_func try_clock_gettime_monotonic();
      static timestamp_func try_getitimer_real();
      static timestamp_func try_gettimeofday();
      class initializer
      {
      public:
        initializer();
      };

      static initializer bootstrap;
    };

    class cputime
    {
    public:
      typedef unsigned long timestamp;
      static timestamp timestamp_absolute() { return func(); }
      static const char *get_name() { return name; }
      typedef timestamp (*timestamp_func)();
    private:
      static timestamp_func func;
      static const char    *name;

      static timestamp_func try_clock_gettime_cputime();
      static timestamp_func try_getitimer_virtual();
      static timestamp_func try_getitimer_prof();

      class initializer
      {
      public:
        initializer();
      };

      static initializer bootstrap;
    };
  }
}
#endif // CLOCKS_HPP
