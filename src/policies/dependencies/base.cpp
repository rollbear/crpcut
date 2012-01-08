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
    namespace dependencies {

      crpcut_base
      ::~crpcut_base()
      {
      }

      void
      crpcut_base
      ::crpcut_uninhibit_dependants()
      {
        for (basic_enforcer *p = crpcut_dependants; p; p = p->next)
          {
            if (--p->crpcut_num == 0)
              {
                p->crpcut_dec_action();
              };
          }
      }

      void
      crpcut_base
      ::crpcut_register_success(bool value)
      {
        if (value)
          {
            if (crpcut_state != crpcut_not_run) return;
            crpcut_state = crpcut_success;
            crpcut_uninhibit_dependants();
          }
        else
          {
            crpcut_state = crpcut_fail;
          }
      }

      void
      crpcut_base
      ::crpcut_add_action(dependencies::basic_enforcer *other)
      {
        other->crpcut_inc();
      }

      crpcut_base
      ::crpcut_base()
        : crpcut_state(crpcut_not_run),
          crpcut_num(0),
          crpcut_dependants(0)
      {
      }

      void
      crpcut_base
      ::crpcut_add(basic_enforcer *other)
      {
        other->next = crpcut_dependants;
        crpcut_dependants = other;
        crpcut_add_action(other);
      }

      void
      crpcut_base
      ::crpcut_inc()
      {
        ++crpcut_num;
      }

      bool
      crpcut_base
      ::crpcut_can_run() const
      {
        return crpcut_num == 0;
      }

      bool
      crpcut_base
      ::crpcut_failed() const
      {
        return crpcut_state == crpcut_fail;
      }

      bool
      crpcut_base
      ::crpcut_succeeded() const
      {
        return crpcut_state == crpcut_success;
      }

      basic_enforcer
      ::basic_enforcer()
        : next(0)
      {
      }
    }
  }
}
