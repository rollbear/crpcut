/*
 * Copyright 2011-2012 Bjorn Fahller <bjorn@fahller.se>
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

#include "output_formatter.hpp"
#include "output_writer.hpp"

namespace crpcut {
  namespace output {
    class buffer;

    class xml_formatter : public formatter,
                          private writer
    {
    public:
      xml_formatter(output::buffer &buffer,
                    const char     *id,
                    int             argc,
                    const char     *argv[]);
      virtual ~xml_formatter();
      virtual void begin_case(const char *name,
                              std::size_t name_len,
                              bool        result,
                              bool        critical);
      virtual void end_case();
      virtual void terminate(test_phase phase,
                             const char *msg,
                             std::size_t msg_len,
                             const char *dirname = 0,
                             std::size_t dn_len = 0);
      virtual void print(const char *tag,
                         std::size_t tlen,
                         const char *data,
                         std::size_t dlen);
      virtual void statistics(unsigned num_registered,
                              unsigned num_selected,
                              unsigned num_run,
                              unsigned num_failed);
      virtual void nonempty_dir(const char *s);
      virtual void blocked_test(const crpcut_test_case_registrator*);
      virtual void tag_summary(const char *tag_name,
                               std::size_t num_passed,
                               std::size_t num_failed,
                               bool        critical);
    private:
      virtual fixed_string escape(char c) const;
      void make_closed();

      std::size_t non_critical_fail_sum;
      bool        last_closed_;
      bool        blocked_tests_;
      bool        tag_summary_;
    };
  }
}

#endif // XML_FORMATTER_HPP
