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

namespace crpcut {
  namespace policies {


    void
    report_unexpected_exception(comm::type               action,
                                datatypes::fixed_string  location,
                                const char              *check_name,
                                const char              *check_type,
                                const char              *params)
    {
      std::string exc = policies::crpcut_exception_translator::try_all();
      std::ostringstream os;

      os << check_name << "_" << check_type << '(' << params << ")\n"
         << "  caught " << exc;
      comm::report(action, os, location);

    }

    crpcut_exception_translator
    ::crpcut_exception_translator(int)
    {
    }

    crpcut_exception_translator
    ::crpcut_exception_translator(crpcut_exception_translator &r)
      : datatypes::list_elem<crpcut_exception_translator>()
    {
      link_before(r);
    }

    crpcut_exception_translator
    ::~crpcut_exception_translator()
    {
    }

    std::string
    crpcut_exception_translator
    ::do_try_all()
    {
      for (crpcut_exception_translator *p = next();
           p != this;
           p = p->next())
        {
          try {
            return p->crpcut_translate();
          }
          catch (...)
            {
            }
        }
      return "...";
    }

    std::string
    crpcut_exception_translator
    ::try_all()
    {
      return root_object().do_try_all();
    }

    crpcut_exception_translator&
    crpcut_exception_translator
    ::root_object()
    {
      static crpcut_exception_translator obj(0);
      return obj;
    }

    std::string crpcut_exception_translator::crpcut_translate() const
    {
      throw;
    }
  }
}
