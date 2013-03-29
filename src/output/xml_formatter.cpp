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

#include "xml_formatter.hpp"
#include "../wrapped/posix_encapsulation.hpp"
#include "../posix_error.hpp"
#include <iomanip>
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
    ::xml_formatter(output::buffer      &buffer,
                    const char          *id,
                    const char * const   argv[],
                    const tag_list_root &tags,
                    std::size_t          num_registered,
                    std::size_t          num_selected)
      : writer(buffer,
               "UTF-8",
               xml_replacement(get_illegal_char_representation())),
        non_critical_fail_sum_(0),
        last_closed_(false),
        blocked_tests_(false),
        tag_summary_(false),
        tags_(tags),
        num_registered_(num_registered),
        num_selected_(num_selected)
    {
      char machine_string[HOST_NAME_MAX + 1];
      int rv = wrapped::gethostname(machine_string, sizeof(machine_string));
      assert(rv == 0);
      (void)rv; // silense warning if built with NDEBUG
      time_t now = wrapped::time(0);
      assert(now != time_t(-1));

      struct tm *tmdata = wrapped::gmtime(&now);
      assert(tmdata);

      stream::toastream<sizeof("2009-01-09T23:59:59Z")> timestamp;
      timestamp << std::setfill('0')
                << std::setw(4) << tmdata->tm_year + 1900
                << '-'
                << std::setw(2) << tmdata->tm_mon + 1
                << '-'
                << std::setw(2) << tmdata->tm_mday
                << 'T'
                << std::setw(2) << tmdata->tm_hour
                << ':'
                << std::setw(2) << tmdata->tm_min
                << ':'
                << std::setw(2) << tmdata->tm_sec
                << 'Z';
      assert(timestamp.size() == 20U);

      write("<?xml version=\"1.0\"?>\n\n"
            "<crpcut xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
            "\n        xsi:noNamespaceSchemaLocation="
            "\"http://crpcut.sourceforge.net/crpcut-"
            CRPCUT_VERSION_STRING
            ".xsd\""
            "\n        starttime=\"");
      write(timestamp);

      write("\"\n        host=\"");
      write(machine_string, wrapped::strlen(machine_string), translated);

      write("\"\n        command=\"");
      for (int i = 0; argv[i]; ++i)
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
      const char *illegal = xml_replacement(get_illegal_char_representation());
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
          break;
        }
      return esc;
    }

    void
    xml_formatter
    ::begin_case(std::string   name,
                 bool          result,
                 bool          critical,
                 unsigned long duration_us)
    {
      write("  <test name=\"");
      write(name, translated);
      write("\" critical=\"");
      write(critical ? "true" : "false");
      write("\" duration_us=\"");
      std::ostringstream os;
      os << duration_us;
      write(os.str());
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
                datatypes::fixed_string location,
                std::string             dirname)
    {
      assert(dirname.length() || msg);
      make_closed();
      write("      <violation phase=");
      const datatypes::fixed_string &ps = phase_str(phase);
      write(ps.str, ps.len);
      if (dirname.length())
        {
          write(" nonempty_dir=\"");
          write(dirname, translated);
          write("\"");
        }

      write(" location=\"");
      write(location, translated);
      write("\"");
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
            datatypes::fixed_string data,
            datatypes::fixed_string location)
    {
      assert(label);
      assert(data);
      make_closed();
      write("    <");
      write(label);
      if (location)
        {
          write(" location=\"");
          write(location, translated);
          write("\"");
        }
      write(">");
      write(data, translated);
      write("</");
      write(label);
      write(">\n");
    }

    void
    xml_formatter
    ::statistics(unsigned num_run,
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
      write(num_registered_);
      write("</registered_test_cases>\n"
            "    <selected_test_cases>");
      write(num_selected_);
      write("</selected_test_cases>\n"
            "    <untested_test_cases>");
      write(num_selected_ - num_run);
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

#define CRPCUT_STR_FIRST(p1, p2) #p1

    void
    xml_formatter
    ::blocked_test(tag::importance i, std::string name)
    {
      static const char *istr[] = { CRPCUT_TEST_IMPORTANCE(CRPCUT_STR_FIRST) };
      if (!blocked_tests_)
        {
          write("  <blocked_tests>\n");
          blocked_tests_ = true;
        }
      write("    <test name=\"");
      write(name, translated);
      write("\" importance=\"");
      write(istr[i]);
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
