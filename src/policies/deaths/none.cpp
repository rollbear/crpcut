/*
 * Copyright 2012 Bjorn Fahller <bjorn@fahller.se>
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

namespace crpcut {
  namespace policies {
    namespace deaths {

      unsigned long
      crpcut_none
      ::crpcut_calc_deadline(unsigned long ts_us) const
      {
        return ts_us + 1000000UL;
      }

      void
      crpcut_none
      ::crpcut_on_ok_action(const char *) const
      {
      }

      bool
      crpcut_none
      ::crpcut_is_expected_exit(int) const
      {
        return false;
      }

      bool
      crpcut_none
      ::crpcut_is_expected_signal(int) const
      {
        return false;
      }

      void
      crpcut_none
      ::crpcut_expected_death(std::ostream &os) const
      {
        os << "normal exit";
      }

    }
  }
}
