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
    struct text_formatter::tag_result
    {
      tag_result(fixed_string n, std::size_t p, std::size_t f, bool c)
        : name(n), passed(p), failed(f), critical(c) {}
      fixed_string name;
      std::size_t passed;
      std::size_t failed;
      bool        critical;
    };
    text_formatter
    ::text_formatter(buffer              &buff,
                     const char          *,
                     int,
                     const char         **,
                     const text_modifier &m)
      : writer(buff,
               output_charset(test_case_factory::get_output_charset()),
               illegal_replacement(test_case_factory::get_illegal_rep())),
        conversion_type_(test_case_factory::get_output_charset()
                         ? translated
                         : verbatim),
        modifier_(m)
    {
    }

    text_formatter
    ::~text_formatter()
    {
    }

    void
    text_formatter
    ::begin_case(datatypes::fixed_string name,
                 bool                    result,
                 bool                    critical)
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
                datatypes::fixed_string dirname)
    {
      if (did_output_)
        {
          write(delim, conversion_type_);
        }
      did_output_ = true;
      if (dirname)
        {
          write(dirname, conversion_type_);
          write(" is not empty!!\n", conversion_type_);
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
          write(msg, conversion_type_);
          write("\n", conversion_type_);
          write(delim, conversion_type_);
        }
    }

    void
    text_formatter
    ::print(datatypes::fixed_string label,
            datatypes::fixed_string data)
    {
      if (did_output_)
        {
          write(delim, conversion_type_);
        }
      did_output_ = true;
      write(label, conversion_type_);
      write(data, conversion_type_);
      write("\n", conversion_type_);
    }

    void
    text_formatter
    ::display_tag_list_header()
    {
      std::ostringstream os;
      os << " "
         << std::setw(tag_list::longest_name_len())
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
    ::statistics(unsigned /* num_registered */,
                 unsigned num_selected,
                 unsigned num_run,
                 unsigned num_failed)
    {
      write(num_selected);
      write(" test cases selected\n", conversion_type_);
      std::size_t sum_passed[] = { 0, 0 };
      std::size_t sum_failed[] = { 0, 0 };
      if (tag_results.size() > 0)
        {
          bool header_displayed = false;
          const std::size_t tag_space = size_t(tag_list::longest_name_len());
          const std::size_t dec_space = modifier_.longest_decorator_len()*2U;
          const std::size_t ws_len = 1 + 8 + 8 + 8 + 1 + 2;
          const std::size_t buff_len = tag_space + dec_space + ws_len;
          char *buffer = static_cast<char*>(alloca(buff_len));
          while (!tag_results.empty())
            {
              tag_result &t = tag_results.back();
              if (t.name)
                {
                  if (!header_displayed)
                    {
                      display_tag_list_header();
                      header_displayed = true;
                    }
                  stream::oastream os(buffer, buff_len);
                  const bool result = t.failed == 0;
                  modifier_.write_to(os,
                                     violation_mods[result][t.critical]);
                  os << (t.critical ? '!' : '?')
                     << std::string(t.name.str, t.name.len)
                     << std::setw(int(tag_space - t.name.len + 1))
                     << ' '
                     << std::setw(7) << t.passed + t.failed
                     << std::setw(8) << t.passed
                     << std::setw(8) << t.failed;
                  modifier_.write_to(os, text_modifier::NORMAL);
                  os << '\n';
                  write(os, conversion_type_);
                }
              sum_passed[t.critical] += t.passed;
              sum_failed[t.critical] += t.failed;
              tag_results.pop_back();
            }

        }
      write("\nTotal    :     Sum   Critical   Non-critical");
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
      if (num_selected != num_run)
        {
          std::ostringstream os;
          os << "\n";
          modifier_.write_to(os, text_modifier::BLOCKED_SUM);
          os << "UNTESTED :" << std::setw(8) << num_selected - num_run;
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
    ::blocked_test(datatypes::fixed_string name)
    {
      if (!blocked_tests_)
        {
          write("The following tests were blocked from running:\n",
                conversion_type_);
          blocked_tests_ = true;
        }
      write("  ");
      modifier_.write_to(*this, text_modifier::BLOCKED);
      write(name, conversion_type_);
      modifier_.write_to(*this, text_modifier::NORMAL);
      write("\n");
    }

    void
    text_formatter
    ::tag_summary(fixed_string tag_name,
                  std::size_t  num_passed,
                  std::size_t  num_failed,
                  bool         critical)
    {
      tag_results.push_back(tag_result(tag_name,
                                       num_passed,
                                       num_failed,
                                       critical));
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
