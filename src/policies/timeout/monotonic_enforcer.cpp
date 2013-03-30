/*
 * Copyright 2012-2013 Bjorn Fahller <bjorn@fahller.se>
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
#include "../../clocks/clocks.hpp"

namespace crpcut {
  namespace policies {
    namespace timeout {

      monotonic_enforcer
      ::monotonic_enforcer(unsigned long timeout_ms,
                           const crpcut_test_monitor *current_test)
        : duration_us_(timeout_ms * timeout_multiplier()),
          start_timestamp_us_(clocks::monotonic::timestamp_absolute()),
          current_test_(current_test)
      {

        if (timeouts_are_enabled())
          {
            clocks::monotonic::timestamp deadline = duration_us_;
            comm::report(comm::set_timeout, deadline);
          }
      }

      monotonic_enforcer
      ::~monotonic_enforcer()
      {
        if (!timeouts_are_enabled()) return;
        typedef clocks::monotonic mono;
        mono::timestamp now  = mono::timestamp_absolute();
        unsigned long diff = now - start_timestamp_us_;
        if (diff <= duration_us_)
          {
            comm::report(comm::cancel_timeout, "", comm::reporter::no_location(), current_test_);
            return;
          }
        std::ostringstream os;
        os << "Realtime timeout " << duration_us_ / 1000
           << "ms exceeded.\n  Actual time to completion was " << diff / 1000
           << "ms";
        comm::report(comm::exit_fail, os, current_test_->get_location());
      }
    }
  }
}
