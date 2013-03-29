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

#ifndef XML_FORMATTER_HPP
#define XML_FORMATTER_HPP

#include "formatter.hpp"
#include "writer.hpp"

namespace crpcut {
  namespace output {
    class buffer;

    class xml_formatter : public formatter,
                          private writer
    {
    public:
      xml_formatter(output::buffer      &buffer,
                    const char          *id,
                    const char * const   argv[],
                    const tag_list_root &tags,
                    std::size_t          num_registered,
                    std::size_t          num_selected);
      virtual ~xml_formatter();
      virtual void begin_case(std::string   name,
                              bool          result,
                              bool          critical,
                              unsigned long duration_us);
      virtual void end_case();
      virtual void terminate(test_phase              phase,
                             datatypes::fixed_string msg,
                             datatypes::fixed_string location,
                             std::string             dirname);
      virtual void print(datatypes::fixed_string label,
                         datatypes::fixed_string data,
                         datatypes::fixed_string location);
      virtual void statistics(unsigned num_run,
                              unsigned num_failed);
      virtual void nonempty_dir(const char *s);
      virtual void blocked_test(tag::importance i, std::string name);
    private:
      void tag_summary(const tag& t);
      virtual datatypes::fixed_string escape(char c) const;
      void make_closed();

      std::size_t          non_critical_fail_sum_;
      bool                 last_closed_;
      bool                 blocked_tests_;
      bool                 tag_summary_;
      const tag_list_root &tags_;
      std::size_t          num_registered_;
      std::size_t          num_selected_;
    };
  }
}

#endif // XML_FORMATTER_HPP
