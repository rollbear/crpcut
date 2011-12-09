/*
 * Copyright 2009-2011 Bjorn Fahller <bjorn@fahller.se>
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
#include <iconv.h>
#include "output.hpp"
#include "posix_encapsulation.hpp"



namespace {
#define STR(s) { "\"" #s "\"", sizeof(#s) + 1 }

  crpcut::output::fixed_string phase_str[] = {
    CRPCUT_TEST_PHASES(STR)
  };

#undef STR

}




namespace crpcut
{
  namespace wrapped
  {
#ifndef isprint
    CRPCUT_WRAP_FUNC(libc, isprint, int, (int c), (c))
#endif
    CRPCUT_WRAP_FUNC(libc, iconv_open, iconv_t,
                     (const char *to, const char *from),
                     (to, from))
    CRPCUT_WRAP_FUNC(libc, iconv_close, int, (iconv_t handle), (handle))
    CRPCUT_WRAP_FUNC(libc, iconv, size_t,
                     (iconv_t h, char **i, size_t *iz, char **o, size_t *oz),
                     (h, i, iz, o, oz))
  }
  namespace output
  {
    ssize_t buffer::do_write(const char *buff, size_t len)
    {
      if (!*current)
        {
          *current = new block;
        }
      size_t size = len < block::size - (*current)->len
        ? len
        : block::size - (*current)->len;
      wrapped::memcpy((*current)->mem + (*current)->len, buff, size);
      (*current)->len += size;
      if ((*current)->len == block::size)
        {
          current = &(*current)->next;
        }
      return ssize_t(size);
    }

    buffer::~buffer()
    {
      while (head)
        {
          block *tmp = head;
          head = head->next;
          delete tmp;
        }
    }

    std::pair<const char *, size_t> buffer::do_get_buffer() const
    {
      static const char *null = 0;
      static const size_t zero = 0;

      if (!head) return std::make_pair(null, zero);

      return std::make_pair(head->mem, head->len);
    }

    void buffer::do_advance()
    {
      if (head)
        {
          block *tmp = head;
          head = tmp->next;
          delete tmp;
        }
      if (!head) current=&head;
    }

    formatter::formatter(const char *to_charset, const char *subst)
      : iconv_handle(wrapped::iconv_open(to_charset,
                                         test_case_factory::get_charset())),
        illegal_substitute(subst),
        illegal_substitute_len(wrapped::strlen(subst))
    {
      if (iconv_handle == iconv_t(-1))
        {
          std::ostringstream oss;
          oss << "Can't convert from \"" << test_case_factory::get_charset()
              << "\" to \"" << to_charset << "\"";
          throw std::runtime_error(oss.str());
        }
    }

    formatter::~formatter()
    {
      wrapped::iconv_close(iconv_handle);
    }
    size_t formatter::write(const char *str, size_t len, type t) const
    {
      if (t == verbatim)
        {
          return do_write(str, len);
        }

      size_t prev_n = 0;
      for (size_t n = 0; n < len; ++n)
        {
          const fixed_string esc = escape(str[n]);
          if (!esc) continue;
          do_write_converted(str + prev_n, n - prev_n);
          prev_n = n + 1;
          if (esc) do_write(esc.str, esc.len);
        }
      do_write_converted(str + prev_n, len - prev_n);
      return len;
    }

    fixed_string formatter::escape(char) const
    {
      static fixed_string no_escape = { 0, 0 };
      return no_escape;

    }
    size_t formatter::do_write(const char *p, size_t len) const
    {
      size_t bytes_written = 0;
      while (bytes_written < len)
        {
          ssize_t rv = buffer::write(p + bytes_written, len - bytes_written);
          assert(rv >= 0);
          bytes_written += size_t(rv);
        }
      return bytes_written;
    }

    size_t formatter::do_write_converted(const char *in, size_t len) const
    {
      while (len > 0)
        {
          const size_t conversion_buffer_size = 2048;
          static char conversion_buffer[conversion_buffer_size];
          size_t out_remaining = conversion_buffer_size;

          char *output_ptr = conversion_buffer;
          const char *const out_begin  = output_ptr;
          const size_t rv = wrapped::iconv(iconv_handle,
                                     const_cast<char**>(&in), &len,
                                     &output_ptr, &out_remaining);
          const size_t out_length(size_t(output_ptr - out_begin));
          do_write(out_begin, out_length);
          if (rv != size_t(-1))
            {
              continue;
            }
          switch (errno)
            {
            case E2BIG:
              break;
            case EINVAL:
            case EILSEQ:
              if (len) { ++in; --len; }
              do_write(illegal_substitute, illegal_substitute_len);
              break;
            default:
              wrapped::abort();
            }
        }
      return len;
    }

    namespace {
      const char *xml_replacement(const char *p)
      {
        return p ? p : "&#xfffd;";
      }
    }

    xml_formatter::xml_formatter(const char *id, int argc_, const char *argv_[])
      : formatter("UTF-8",
                  xml_replacement(test_case_factory::get_illegal_rep())),
        last_closed(false),
        blocked_tests(false),
        argc(argc_),
        argv(argv_)
    {
      write("<?xml version=\"1.0\"?>\n\n"
            "<crpcut xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
            " xsi:noNamespaceSchemaLocation="
            "\"http://crpcut.sourceforge.net/crpcut-" CRPCUT_VERSION_STRING ".xsd\""
            " starttime=\"");

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
      write("\" host=\"");
      write(machine_string, wrapped::strlen(machine_string), translated);

      write("\" command=\"");
      for (int i = 0; i < argc; ++i)
        {
          if (i > 0) write(" ", 1);
          write(argv[i], translated);
        }
      if (id)
        {
          write("\" id=\"");
          write(id, translated);
        }
      write("\">\n");
    }


    xml_formatter::~xml_formatter()
    {
    }

    fixed_string xml_formatter::escape(char c) const
    {
      const char *illegal = xml_replacement(test_case_factory::get_illegal_rep());
      const size_t ill_len = wrapped::strlen(illegal);
      fixed_string esc = { 0, 0 };
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
          if (wrapped::isprint(c)) break;
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
          esc.len = size_t(out.end() - out.begin());
        }
      return esc;
    }
    void xml_formatter::begin_case(const char *name,
                                   size_t      name_len,
                                   bool        result)
    {
      write("  <test name=\"");
      write(name, name_len, translated);
      write("\" result=");
      static const char *rstring[] = { "\"FAILED\"", "\"PASSED\"" };
      write(rstring[result]);
      last_closed=false;
    }

    void xml_formatter::end_case()
    {
      if (last_closed)
        {
          write("    </log>\n  </test>\n");
        }
      else
        {
          write("/>\n");
        }
    }

    void xml_formatter::terminate(test_phase  phase,
                                  const char *msg,
                                  size_t      msg_len,
                                  const char *dirname,
                                  size_t      dn_len)
    {
      make_closed();
      write("      <violation phase=");
      write(phase_str[phase].str, phase_str[phase].len);
      if (dirname)
        {
          write(" nonempty_dir=\"");
          write(dirname, dn_len, translated);
          write("\"");
        }
      if (msg_len == 0)
        {
          write("/>\n");
          return;
        }
      write(">");
      write(msg, msg_len, translated);
      write("</violation>\n");
    }

    void xml_formatter::print(const char *tag,
                              size_t      tlen,
                              const char *data,
                              size_t      dlen)
    {
      make_closed();
      write("    <");
      write(tag, tlen);
      if (dlen == 0)
        {
          write("/>\n");
          return;
        }
      write(">");
      write(data, dlen, translated);
      write("</");
      write(tag);
      write(">\n");
    }

    void xml_formatter::statistics(unsigned num_registered,
                                   unsigned num_selected,
                                   unsigned num_run,
                                   unsigned num_failed)
    {
      if (blocked_tests)
        {
          write("  </blocked_tests>\n");
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
            "  </statistics>\n"
            "</crpcut>\n");
    }

    void xml_formatter::nonempty_dir(const char *s)
    {
      write("  <remaining_files nonempty_dir=\"");
      write(s);
      write("\"/>\n");
    }

    void
    xml_formatter::blocked_test(const test_case_reg *i)
    {
      if (!blocked_tests)
        {
          write("  <blocked_tests>\n");
          blocked_tests = true;
        }
      const size_t len = i->crpcut_full_name_len() + 1;
      char *name = static_cast<char*>(alloca(len));
      stream::oastream os(name, name + len);
      write("    <test name=\"");
      os << *i;
      write(os, translated);
      write("\"/>\n");
    }

    void xml_formatter::make_closed()
    {
      if (!last_closed)
        {
          write(">\n    <log>\n");
          last_closed = true;
        }
    }
  }
}

namespace {
  static const char barrier[] =
    "===============================================================================\n";
  static const char rlabel[2][9] = { "FAILED: ", "PASSED: " };
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
}

namespace crpcut {
  namespace output {


    text_formatter::text_formatter(const char *, int, const char**)
      : formatter(output_charset(test_case_factory::get_output_charset()),
                  illegal_replacement(test_case_factory::get_illegal_rep())),
        conversion_type(test_case_factory::get_output_charset()
                        ? translated
                        : verbatim)
    {
    }

    void text_formatter::begin_case(const char *name,
                                    size_t      name_len,
                                    bool        result)
    {
      did_output = false;
      write(rlabel[result], 8, conversion_type);
      write(name, name_len, conversion_type);
      write("\n", conversion_type);
    }

    void text_formatter::end_case()
    {
      write(barrier, conversion_type);
    }

    void text_formatter::terminate(test_phase   phase,
                                   const char  *msg,
                                   size_t       msg_len,
                                   const char  *dirname,
                                   size_t       dn_len)
    {
      if (did_output)
        {
          write(delim, conversion_type);
        }
      did_output = true;
      if (dirname)
        {
          write(dirname, dn_len, conversion_type);
          write(" is not empty!!\n", conversion_type);
        }
      if (msg_len)
        {
          write("phase=", conversion_type);
          write(phase_str[phase].str, phase_str[phase].len, conversion_type);
          write("  ", conversion_type);
          write(delim + 8 + phase_str[phase].len,
                sizeof(delim) - 8 - phase_str[phase].len - 1,
                conversion_type);
          write(msg, msg_len, conversion_type);
          write("\n", conversion_type);
          write(delim, conversion_type);
        }
    }

    void text_formatter::print(const char *tag,
                               size_t      tlen,
                               const char *data,
                               size_t      dlen)
    {
      if (did_output)
        {
          write(delim, conversion_type);
        }
      did_output = true;
      const size_t len = write(tag, tlen, conversion_type);
      if (len < sizeof(delim))
        {
          write(delim + len, sizeof(delim) - len - 1, conversion_type);
        }
      write(data, dlen, conversion_type);
      write("\n", conversion_type);
    }

    void text_formatter::statistics(unsigned /* num_registered */,
                                    unsigned num_selected,
                                    unsigned num_run,
                                    unsigned num_failed)
    {
      write("Total ", conversion_type);
      write(num_selected);
      write(" test cases selected", conversion_type);
      write("\nUNTESTED : ", conversion_type);
      write(num_selected - num_run);
      write("\nPASSED   : ", conversion_type);
      write(num_run - num_failed);
      write("\nFAILED   : ", conversion_type);
      write(num_failed);
      write("\n", conversion_type);
    }

    void text_formatter::nonempty_dir(const char *s)
    {
      write("Files remain under ", conversion_type);
      write(s, conversion_type);
      write("\n", conversion_type);
    }

    void text_formatter::blocked_test(const test_case_reg *i)
    {
      if (!blocked_tests)
        {
          write("The following tests were blocked from running:\n",
                conversion_type);
          blocked_tests = true;
        }
      const size_t len = i->crpcut_full_name_len() + 1;
      char * name = static_cast<char*>(alloca(len));
      stream::oastream os(name, name+len+2);
      os << "  " << *i << '\n';
      write(os, conversion_type);
    }

  }
}
