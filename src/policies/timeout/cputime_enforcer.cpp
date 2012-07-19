/*
 * Copyright 2009-2012 Bjorn Fahller <bjorn@fahller.se>
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
#include "../../wrapped/posix_encapsulation.hpp"
#include "../../clocks/clocks.hpp"

namespace crpcut {
  namespace policies {
    namespace timeout {

      cputime_enforcer
      ::cputime_enforcer(unsigned long timeout_us)
        : duration_us(timeout_us * timeout_multiplier()),
          start_timestamp_us(clocks::cputime::timestamp_absolute())
      {
        if (timeout_us && timeouts_are_enabled())
          {
            rlimit r = { (duration_us + 1500000) / 1000000,
                         (duration_us + 2500000) / 1000000 };
            wrapped::setrlimit(RLIMIT_CPU, &r);
          }
      }

      cputime_enforcer
      ::~cputime_enforcer()
      {
        if (!timeouts_are_enabled() || duration_us == 0) return;

        clocks::cputime::timestamp now
          = clocks::cputime::timestamp_absolute();
        unsigned long diff = now - start_timestamp_us;
        if  (diff > duration_us)
          {
            std::ostringstream os;
            os << comm::report.get_location()
               << "\nCPU-time timeout " << duration_us / 1000
               << "ms exceeded.\n  Actual time to completion was "
               << diff / 1000 << "ms";
            comm::report(comm::exit_fail, os);
          }
      }
    }
  }
}
