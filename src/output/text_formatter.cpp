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

#include "text_formatter.hpp"
#include "text_modifier.hpp"

namespace {
  static const char barrier[] =
    "===============================================================================\n";

#define MK_FIXSTR(s) {  #s , sizeof(#s) - 1 } /* no need for '\0' in len */

  static const crpcut::datatypes::fixed_string rlabel[] =
    {
      MK_FIXSTR(FAILED),
      MK_FIXSTR(PASSED)
    };
  static const char delim[]=
    "-------------------------------------------------------------------------------\n";

  const char *output_charset(const char *p)
  {
    return p ? p : "UTF-8";
  }

  const char *illegal_replacement(const char *p)
  {
    return p ? p : ".";
  }

  typedef crpcut::output::text_modifier tm;
  static const tm::decorator violation_mods[2][2] =
    {
      { tm::NCFAILED, tm::FAILED },
      { tm::NCPASSED, tm::PASSED }
    };
}

namespace crpcut {
  namespace output {
    using datatypes::fixed_string;
    text_formatter
    ::text_formatter(buffer              &buff,
                     const char          *,
                     const char   *const *,
                     const tag_list_root &tags,
                     std::size_t       /*num_registered*/,
                     std::size_t         num_selected,
                     const text_modifier &mod,
                     const char         *os)
      : writer(buff,
               output_charset(os),
               illegal_replacement(get_illegal_char_representation())),
        did_output_(false),
        blocked_tests_(false),
        conversion_type_(os ? translated : verbatim),
        tags_(tags),
        modifier_(mod),
        num_selected_(num_selected)
    {
    }

    text_formatter
    ::~text_formatter()
    {
    }

    void
    text_formatter
    ::begin_case(std::string     name,
                 bool            result,
                 bool            critical,
                 unsigned long /*duration_us*/)
    {
      modifier_.write_to(*this, violation_mods[result][critical]);
      did_output_ = false;
      write(rlabel[result].str, rlabel[result].len, conversion_type_);
      write(critical ? "!" : "?");
      write(": ");
      write(name, conversion_type_);
      write("\n", conversion_type_);
    }

    void
    text_formatter
    ::end_case()
    {
      modifier_.write_to(*this, text_modifier::NORMAL);
      write(barrier, conversion_type_);
    }

    void
    text_formatter
    ::terminate(test_phase              phase,
                datatypes::fixed_string msg,
                datatypes::fixed_string location,
                std::string             dirname)
    {
      if (did_output_)
        {
          write(delim, conversion_type_);
        }
      did_output_ = true;
      if (dirname.length())
        {
          write(dirname, conversion_type_);
          write(" is not empty!\n", conversion_type_);
        }
      if (msg)
        {
          write("phase=", conversion_type_);
          const fixed_string &ps = phase_str(phase);
          write(ps.str, ps.len, conversion_type_);
          write("  ", conversion_type_);
          write(delim + 8 + ps.len,
                sizeof(delim) - 8 - ps.len - 1,
                conversion_type_);
          write(location, conversion_type_);
          write("\n", conversion_type_);
          write(msg, conversion_type_);
          write("\n", conversion_type_);
          write(delim, conversion_type_);
        }
    }

    void
    text_formatter
    ::print(datatypes::fixed_string label,
            datatypes::fixed_string data,
            datatypes::fixed_string location)
    {
      if (did_output_)
        {
          write(delim, conversion_type_);
        }
      did_output_ = true;
      std::size_t len = write(label, conversion_type_);
      if (len < sizeof(delim))
        {
          write(delim + len, sizeof(delim) - len - 1, conversion_type_);
        }
      if (location)
        {
          write(location, conversion_type_);
          write("\n", conversion_type_);
        }
      write(data, conversion_type_);
      write("\n", conversion_type_);
    }

    void
    text_formatter
    ::display_tag_list_header()
    {
      std::ostringstream os;
      os << " "
         << std::setw(int(tags_.longest_tag_name()))
         << std::setiosflags(std::ios::left) << "tag"
         << std::resetiosflags(std::ios::left)
         << std::setw(8) << "run"
         << std::setw(8) << "passed"
         << std::setw(8) << "failed"
         << '\n';
      write(os, conversion_type_);
    }

    void
    text_formatter
    ::statistics(unsigned num_run,
                 unsigned num_failed)
    {
      write(num_selected_);
      write(" test cases selected\n", conversion_type_);
      std::size_t sum_passed[] = { 0, 0 };
      std::size_t sum_failed[] = { 0, 0 };

      bool header_displayed = false;
      const std::size_t tag_space = size_t(tags_.longest_tag_name());
      const std::size_t dec_space = modifier_.longest_decorator_len()*2U;
      const std::size_t ws_len = 1 + 8 + 8 + 8 + 1 + 2;
      const std::size_t buff_len = tag_space + dec_space + ws_len;
      char *buffer = static_cast<char*>(alloca(buff_len));
      tag_list_root::const_iterator i = tags_.begin();
      tag_list_root::const_iterator const end = tags_.end();
      do {
        const tag &t = *i;
        const bool critical = t.get_importance() == tag::critical;

        sum_passed[critical] += t.num_passed();
        sum_failed[critical] += t.num_failed();

        datatypes::fixed_string name = t.get_name();
        if (!name) continue;
        if (t.num_passed() + t.num_failed() == 0) continue;
        if (!header_displayed)
          {
            display_tag_list_header();
            header_displayed = true;
          }
        stream::oastream os(buffer, buff_len);
        const bool result = t.num_failed() == 0;
        modifier_.write_to(os,
                           violation_mods[result][critical]);
        os << (critical ? '!' : '?')
           << std::string(name.str, name.len)
           << std::setw(int(tag_space - name.len + 8))
            << t.num_passed() + t.num_failed()
           << std::setw(8) << t.num_passed()
           << std::setw(8) << t.num_failed();
        modifier_.write_to(os, text_modifier::NORMAL);
        os << '\n';
        write(os, conversion_type_);
      } while (i++ != end);

      write("\n               Sum   Critical   Non-critical");
      if (num_run != num_failed)
        {
          std::ostringstream os;
          os << "\n";
          modifier_.write_to(os, sum_passed[1]
                             ? text_modifier::PASSED_SUM
                             : text_modifier::NCPASSED_SUM);
          os << "PASSED   :" << std::setw(8) << num_run - num_failed
             << std::setw(11) << (num_run - num_failed - sum_passed[0]);
          os << std::setw(15) << sum_passed[0];
          modifier_.write_to(os, text_modifier::NORMAL);
          write(os);
      }
      if (num_failed)
        {
          std::ostringstream os;
          os << "\n";
          modifier_.write_to(os,
                             sum_failed[0] == num_failed
                             ? text_modifier::NCFAILED_SUM
                             : text_modifier::FAILED_SUM);
          os << "FAILED   :" << std::setw(8) << num_failed
             << std::setw(11) << num_failed - sum_failed[0];
          os << std::setw(15) << sum_failed[0];
          modifier_.write_to(os, text_modifier::NORMAL);
          write(os);
        }
      if (num_selected_ != num_run)
        {
          std::ostringstream os;
          os << "\n";
          modifier_.write_to(os, text_modifier::BLOCKED_SUM);
          os << "UNTESTED :" << std::setw(8) << num_selected_ - num_run;
          modifier_.write_to(os, text_modifier::NORMAL);
          write(os);
        }
      write("\n", conversion_type_);
    }

    void
    text_formatter
    ::nonempty_dir(const char *s)
    {
      write("Files remain under ", conversion_type_);
      write(s, conversion_type_);
      write("\n", conversion_type_);
    }

    void
    text_formatter
    ::blocked_test(tag::importance i, std::string name)
    {
      if (!blocked_tests_)
        {
          write("The following tests were blocked from running:\n",
                conversion_type_);
          blocked_tests_ = true;
        }
      {
        stream::toastream<3> os;
        os << "  " << i;
        write(os, conversion_type_);
      }
      modifier_.write_to(*this, text_modifier::BLOCKED);
      write(name, conversion_type_);
      modifier_.write_to(*this, text_modifier::NORMAL);
      write("\n", conversion_type_);
    }

    const text_modifier&
    text_formatter
    ::default_text_modifier()
    {
      static const text_modifier obj;
      return obj;
    }
  }
}
