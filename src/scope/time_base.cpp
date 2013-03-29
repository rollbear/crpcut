/*
 * Copyright 2011-2013 Bjorn Fahller <bjorn@fahller.se>
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
#include "../clocks/clocks.hpp"

namespace crpcut {
  namespace scope {
    time_base::time_base(unsigned long  deadline,
                         datatypes::fixed_string location)
        : deadline_(deadline),
          location_(location)
    {
    }

    const char *time_base::min::name()
    {
      return "MIN";
    }

    bool time_base::min::busted(unsigned long now, unsigned long deadline)
    {
      return now < deadline;
    }

    const char *time_base::max::name()
    {
      return "MAX";
    }

    bool time_base::max::busted(unsigned long now, unsigned long deadline)
    {
      return now > deadline;
    }

    const char *time_base::realtime::name()
    {
      return "REALTIME";
    }

    unsigned long time_base::realtime::now()
    {
      return clocks::monotonic::timestamp_absolute();
    }

    const char *time_base::cputime::name()
    {
      return "CPUTIME";
    }
    unsigned long time_base::cputime::now()
    {
      return clocks::cputime::timestamp_absolute();
    }
  }
}
