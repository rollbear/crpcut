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

#ifndef NIL_FORMATTER_HPP
#define NIL_FORMATTER_HPP

#include "formatter.hpp"

namespace crpcut {
  namespace output {
    class buffer;
    class nil_formatter : public formatter
    {
    public:
      nil_formatter(output::buffer &,
                    const char *,
                    const char*const*,
                    const tag_list_root &,
                    std::size_t,
                    std::size_t);

      ~nil_formatter() override;
      void begin_case(std::string   name,
                      bool          result,
                      bool          critical,
                      unsigned long duration_us) override;
      void end_case() override;
      void terminate(test_phase              phase,
                     datatypes::fixed_string msg,
                     datatypes::fixed_string location,
                     std::string             dirname) override;
      void print(datatypes::fixed_string label,
                 datatypes::fixed_string data,
                 datatypes::fixed_string location) override;
      void statistics(unsigned num_run,
                      unsigned num_failed) override;
      void nonempty_dir(const char *s) override;
      void blocked_test(tag::importance i, std::string name) override;
    };
  }
}
#endif // NIL_FORMATTER_HPP
