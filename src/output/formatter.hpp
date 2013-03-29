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

#ifndef OUTPUT_FORMATTER_HPP
#define OUTPUT_FORMATTER_HPP


#include <crpcut.hpp>
#include <iosfwd>
namespace crpcut {
  namespace output {
    class buffer;

    class formatter
    {
    public:
      virtual void begin_case(std::string   name,
                              bool          result,
                              bool          critical,
                              unsigned long duration_us) = 0;
      virtual void end_case()  = 0;
      virtual void terminate(test_phase              phase,
                             datatypes::fixed_string msg,
                             datatypes::fixed_string location,
                             std::string             dirname = "") = 0;
      virtual void print(datatypes::fixed_string label,
                         datatypes::fixed_string data,
                         datatypes::fixed_string location) = 0;
      virtual void statistics(unsigned num_run,
                              unsigned num_failed) = 0;
      virtual void nonempty_dir(const  char*)  = 0;
      virtual void blocked_test(tag::importance i, std::string name)  = 0;
      virtual ~formatter();
    protected:
      static const datatypes::fixed_string &phase_str(test_phase);
    };

  }
}

#endif // OUTPUT_FORMATTER_HPP
