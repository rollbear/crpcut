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

#include "xml_formatter.hpp"
#include "../wrapped/posix_encapsulation.hpp"

namespace {
  inline const char *xml_replacement(const char *p)
  {
    return p ? p : "&#xfffd;";
  }

}

namespace crpcut {
  namespace wrapped
  {
#ifdef isprint
#define ISPRINT(x) isprint(x)
#else
    CRPCUT_WRAP_FUNC(libc, isprint, int, (int c), (c))
#define ISPRINT(x) wrapped::isprint(x)
#endif
  }

  namespace output {

    xml_formatter
    ::xml_formatter(output::buffer &buffer,
                    const char     *id,
                    int             argc,
                    const char *argv[],
                    const tag_list_root &tags)
      : writer(buffer,
               "UTF-8",
               xml_replacement(test_case_factory::get_illegal_rep())),
        non_critical_fail_sum_(0),
        last_closed_(false),
        blocked_tests_(false),
        tag_summary_(false),
        tags_(tags)
    {
      write("<?xml version=\"1.0\"?>\n\n"
            "<crpcut xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
            "\n        xsi:noNamespaceSchemaLocation="
            "\"http://crpcut.sourceforge.net/crpcut-"
            CRPCUT_VERSION_STRING
            ".xsd\""
            "\n        starttime=\"");

      char time_string[sizeof("2009-01-09T23:59:59Z")];
      time_t now = wrapped::time(0);
      struct tm *tmdata = wrapped::gmtime(&now);
      int len = wrapped::snprintf(time_string, sizeof(time_string),
                                  "%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2dZ",
                                  tmdata->tm_year + 1900,
                                  tmdata->tm_mon + 1,
                                  tmdata->tm_mday,
                                  tmdata->tm_hour,
                                  tmdata->tm_min,
                                  tmdata->tm_sec);
      assert(len < int(sizeof(time_string)));
      assert(time_string[len] == 0);
      write(time_string);

      char machine_string[PATH_MAX];
      if (wrapped::gethostname(machine_string, sizeof(machine_string)))
        {
          machine_string[0] = 0;
        }
      write("\"\n        host=\"");
      write(machine_string, wrapped::strlen(machine_string), translated);

      write("\"\n        command=\"");
      for (int i = 0; i < argc; ++i)
        {
          if (i > 0) write(" ", 1);
          write(argv[i], translated);
        }
      if (id)
        {
          write("\"\n        id=\"");
          write(id, translated);
        }
      write("\">\n");
    }


    xml_formatter
    ::~xml_formatter()
    {
    }

    datatypes::fixed_string
    xml_formatter
    ::escape(char c) const
    {
      const char *illegal = xml_replacement(test_case_factory::get_illegal_rep());
      const std::size_t ill_len = wrapped::strlen(illegal);
      datatypes::fixed_string esc = { 0, 0 };
      switch (c)
        {
        case '\t' : break;
        case '\r' : break;
        case '\n' : break;
        case '<' : esc.str = "&lt;";   esc.len = 4; break;
        case '>' : esc.str = "&gt;";   esc.len = 4; break;
        case '&' : esc.str = "&amp;";  esc.len = 5; break;
        case '"' : esc.str = "&quot;"; esc.len = 6; break;
        case '\'': esc.str = "&apos;"; esc.len = 6; break;
        default:
          if (ISPRINT(c)) break;
          if (c < 32)
                {
                  esc.str = illegal;
                  esc.len = ill_len;
                  break;
                }
          static char buff[8];
          stream::oastream out(buff);
          out << "&#x" << std::hex << unsigned(c) << ';';
          esc.str = out.begin();
          esc.len = std::size_t(out.end() - out.begin());
        }
      return esc;
    }

    void
    xml_formatter
    ::begin_case(datatypes::fixed_string name,
                 bool                    result,
                 bool                    critical)
    {
      write("  <test name=\"");
      write(name, translated);
      write("\" critical=\"");
      write(critical ? "true" : "false");
      write("\" result=");
      static const char *rstring[] = { "\"FAILED\"", "\"PASSED\"" };
      write(rstring[result]);
      last_closed_ = false;
    }

    void
    xml_formatter
    ::end_case()
    {
      if (last_closed_)
        {
          write("    </log>\n  </test>\n");
        }
      else
        {
          write("/>\n");
        }
    }

    void
    xml_formatter
    ::terminate(test_phase              phase,
                datatypes::fixed_string msg,
                datatypes::fixed_string dirname)
    {
      make_closed();
      write("      <violation phase=");
      const datatypes::fixed_string &ps = phase_str(phase);
      write(ps.str, ps.len);
      if (dirname)
        {
          write(" nonempty_dir=\"");
          write(dirname, translated);
          write("\"");
        }
      if (!msg)
        {
          write("/>\n");
          return;
        }
      write(">");
      write(msg, translated);
      write("</violation>\n");
    }

    void
    xml_formatter
    ::print(datatypes::fixed_string label,
            datatypes::fixed_string data)
    {
      make_closed();
      write("    <");
      write(label);
      if (!label)
        {
          write("/>\n");
          return;
        }
      write(">");
      write(data, translated);
      write("</");
      write(label);
      write(">\n");
    }

    void
    xml_formatter
    ::statistics(unsigned num_registered,
                 unsigned num_selected,
                 unsigned num_run,
                 unsigned num_failed)
    {
      tag_list_root::const_iterator i = tags_.begin();
      tag_list_root::const_iterator const end = tags_.end();
      do
        {
          tag_summary(*i);
        }
      while (i++ != end);
      if (tag_summary_)
        {
          write("  </tag_summary>\n");
        }

      write("  <statistics>\n"
            "    <registered_test_cases>");
      write(num_registered);
      write("</registered_test_cases>\n"
            "    <selected_test_cases>");
      write(num_selected);
      write("</selected_test_cases>\n"
            "    <untested_test_cases>");
      write(num_selected - num_run);
      write("</untested_test_cases>\n"
            "    <run_test_cases>");
      write(num_run);
      write("</run_test_cases>\n"
            "    <failed_test_cases>");
      write(num_failed);
      write("</failed_test_cases>\n"
            "    <failed_non_critical_test_cases>");
      write(non_critical_fail_sum_);
      write("</failed_non_critical_test_cases>\n"
            "  </statistics>\n"
            "</crpcut>\n");
    }

    void
    xml_formatter
    ::nonempty_dir(const char *s)
    {
      write("  <remaining_files nonempty_dir=\"");
      write(s);
      write("\"/>\n");
    }

    void
    xml_formatter
    ::blocked_test(datatypes::fixed_string name)
    {
      if (!blocked_tests_)
        {
          write("  <blocked_tests>\n");
          blocked_tests_ = true;
        }
      write("    <test name=\"");
      write(name, translated);
      write("\"/>\n");
    }

    void
    xml_formatter
    ::tag_summary(const tag &t)
    {
      if (blocked_tests_)
        {
          write("  </blocked_tests>\n");
          blocked_tests_ = false;
        }
      if (t.get_importance() != tag::critical)
        {
          non_critical_fail_sum_+= t.num_failed();
        }
      datatypes::fixed_string name = t.get_name();
      if (!name) return;
      if (!tag_summary_)
        {
          write("  <tag_summary>\n");
          tag_summary_ = true;
        }
      write("    <tag name=\"");
      write(name);
      write("\" passed=\"");
      write(t.num_passed());
      write("\" failed=\"");
      write(t.num_failed());
      write("\" critical=\"");
      write(t.get_importance() == tag::critical ? "true" : "false");
      write("\"/>\n");
    }

    void
    xml_formatter
    ::make_closed()
    {
      if (!last_closed_)
        {
          write(">\n    <log>\n");
          last_closed_ = true;
        }
    }
  }
}