/*
 * Copyright 2012 Bjorn Fahller <bjorn@fahller.se>
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


#include "interpreter.hpp"


namespace {
  template <typename T1, typename T2>
  void throw_if_illegal_combination(T1 &t1, T2 &t2)
  {
    if (t1 && t2)
      {
        std::ostringstream os;
        t1.syntax(os) << " cannot be combined with ";
        t2.syntax(os);
        throw crpcut::cli::param::exception(os.str());
      }
  }
}
namespace crpcut {
  namespace cli {
    interpreter::
    interpreter(const char *const *args)
      : list_(),
#ifdef USE_BACKTRACE
        backtrace_('b', "backtrace-heap",
                   "Store stack backtrace for all heap objects for\n"
                   "better error pinpointing of heap violations (slow)",
                   list_),
#endif
        num_children_('c', "children", "number",
                      "Control number of concurrently running test processes\n"
                      "number must be at least 1",
                      list_),
        charset_('C', "output-charset", "charset",
                 "Specify the output character set to convert text output\n"
                 "to. Does not apply for XML output",
                 list_),
        working_dir_('d', "working-dir", "dirname",
                     "Specify working directory (must exist)",
                     list_),
        id_string_('i', "identity", "\"id string\"",
                   "Specify an identity string for the XML-header",
                   list_),
        illegal_rep_('I', "illegal-char", "string",
                     "Specify how characters that are illegal for the chosen\n"
                     "output character set are to be represented",
                     list_),
        list_tests_('l', "list", "List test cases",
                    list_),
        list_tags_('L', "list-tags",
                   "List all tags used by tests in the test program",
                   list_),
        nodeps_('n', "nodeps", "Ignore dependencies",
                list_),
        output_('o', "output", "filename",
                "Direct XML output to a named file. A brief summary will be\n"
                "displayed on stdout",
                list_),
        param_('p', "param",
               "Defined a named variable for access from the test cases",
               list_),
        quiet_('q', "quiet", "Don't display the -o brief summary",
               list_),
        single_shot_('s', "single-shot",
                     "Run only one test case, and run it in the main process\n"
                     "for ease of debugging",
                     list_),
        timeout_multiplier_(0, "timeout-multiplier", "factor",
                  "Multiply all timeout times with a factor",
                  list_),
        disable_timeouts_('t', "disable-timeouts",
                          "Never fail a test due to time consumption",
                          list_),
        tags_('T', "tags", "{select}{/non-critical}",
              "Select tests to run or list based on their tag, and which\n"
              "tags represent non-critical tests. Both \"select\"\n"
              "and \"non-critical\" are comma separated lists of tags.\n"
              "both lists can be empty. If a list begins with \"-\",\n"
              "the list is subtractive from the full set.\n"
              "Untagged tests cannot be made non-critical",
              list_),
        verbose_('v', "verbose",
                 "Verbose mode - include results from passed tests",
                 list_),
        version_('V', "version",
                 "Print version string and exit",
                 list_),
        xml_('x', "xml",
             "XML output on stdout or non-XML output on file",
             list_),
        argv_(args)
    {
      end_ = match_argv();
    }

    void
    interpreter
    ::usage() const
    {
      std::ostringstream os;
      os << "Usage: " << program_name() << " [flags] {testcases}\n"
          "  where flags can be:\n";
      for (const cli::param *p = list_.next();
          !list_.is_this(p);
          p = p->next())
      {
          os << *p << "\n\n";
      }
      throw cli::param::exception(os.str());
    }

    const char *const *
    interpreter
    ::match_argv()
    {
      const char *const *rv = list_.match_all(argv_ + 1);
      assert(rv);
      if (*rv && (*rv)[0] == '-') usage();
      if (version_)
        {
          throw cli::param::exception("crpcut-" CRPCUT_VERSION_STRING);
        }
#ifdef USE_BACKTRACE
      throw_if_illegal_combination(single_shot_, backtrace_);
#endif
      throw_if_illegal_combination(single_shot_, num_children_);
      throw_if_illegal_combination(single_shot_, tags_);
      throw_if_illegal_combination(single_shot_, list_tests_);
      throw_if_illegal_combination(single_shot_, list_tags_);
      throw_if_illegal_combination(single_shot_, nodeps_);
      throw_if_illegal_combination(single_shot_, output_);
      throw_if_illegal_combination(single_shot_, disable_timeouts_);
      throw_if_illegal_combination(single_shot_, illegal_rep_);
      throw_if_illegal_combination(single_shot_, id_string_);
      throw_if_illegal_combination(single_shot_, charset_);
      throw_if_illegal_combination(single_shot_, output_);
      throw_if_illegal_combination(single_shot_, quiet_);
      throw_if_illegal_combination(single_shot_, verbose_);
      throw_if_illegal_combination(single_shot_, xml_);

      throw_if_illegal_combination(id_string_, list_tests_);
      throw_if_illegal_combination(id_string_, list_tags_);

#ifdef USE_BACKTRACE
      throw_if_illegal_combination(list_tests_, backtrace_);
#endif
      throw_if_illegal_combination(list_tests_, num_children_);
      throw_if_illegal_combination(list_tests_, charset_);
      throw_if_illegal_combination(list_tests_, working_dir_);
      throw_if_illegal_combination(list_tests_, illegal_rep_);
      throw_if_illegal_combination(list_tests_, list_tags_);
      throw_if_illegal_combination(list_tests_, nodeps_);
      throw_if_illegal_combination(list_tests_, output_);
      throw_if_illegal_combination(list_tests_, quiet_);
      throw_if_illegal_combination(list_tests_, single_shot_);
      throw_if_illegal_combination(list_tests_, disable_timeouts_);
      throw_if_illegal_combination(list_tests_, verbose_);
      throw_if_illegal_combination(list_tests_, xml_);

#ifdef USE_BACKTRACE
      throw_if_illegal_combination(list_tags_, backtrace_);
#endif
      throw_if_illegal_combination(list_tags_, num_children_);
      throw_if_illegal_combination(list_tags_, charset_);
      throw_if_illegal_combination(list_tags_, working_dir_);
      throw_if_illegal_combination(list_tags_, illegal_rep_);
      throw_if_illegal_combination(list_tags_, nodeps_);
      throw_if_illegal_combination(list_tags_, output_);
      throw_if_illegal_combination(list_tags_, quiet_);
      throw_if_illegal_combination(list_tags_, single_shot_);
      throw_if_illegal_combination(list_tags_, disable_timeouts_);
      throw_if_illegal_combination(list_tags_, tags_);
      throw_if_illegal_combination(list_tags_, verbose_);
      throw_if_illegal_combination(list_tags_, xml_);
      throw_if_illegal_combination(timeout_multiplier_ ,disable_timeouts_);
      throw_if_illegal_combination(timeout_multiplier_, single_shot_);

      if (xml_output() && output_charset())
        {
          std::ostringstream os;
          charset_.syntax(os) << " cannot be used with XML reports";
          throw param::exception(os.str());
        }

      if (num_children_ && num_children_.get_value() == 0)
        {
          std::ostringstream os;
          num_children_.syntax(os) << " - number must be at least 1";
          throw param::exception(os.str());
        }

      if (timeout_multiplier_ && timeout_multiplier_.get_value() == 0)
        {
          std::ostringstream os;
          timeout_multiplier_.syntax(os) << " - factor must be at least 1";
          throw param::exception(os.str());
        }
      return rv;
    }

    const char *const *
    interpreter
    ::get_test_list() const
    {
      return end_;
    }

#ifdef USE_BACKTRACE
    bool
    interpreter
    ::backtrace_enabled() const
    {
      return backtrace_;
    }
#endif

    unsigned
    interpreter::num_parallel_tests() const
    {
      return num_children_ ? num_children_.get_value() : 1U;
    }

    const char *
    interpreter
    ::output_charset() const
    {
      return charset_ ? charset_.get_value() : 0;
    }

    const char *
    interpreter
    ::working_dir() const
    {
      return working_dir_ ? working_dir_.get_value() : 0;
    }

    const char *
    interpreter
    ::identity_string() const
    {
      return id_string_ ? id_string_.get_value() : 0;
    }

    const char *
    interpreter
    ::illegal_representation() const
    {
      return illegal_rep_ ? illegal_rep_.get_value() : 0;
    }

    bool
    interpreter
    ::list_tests() const
    {
      return list_tests_;
    }

    bool
    interpreter
    ::list_tags() const
    {
      return list_tags_;
    }

    bool
    interpreter
    ::honour_dependencies() const
    {
      return !nodeps_;
    }

    const char *
    interpreter
    ::report_file() const
    {
      return output_ ? output_.get_value() : 0;
    }

    const char *
    interpreter
    ::named_parameter(const char *name)
    {
      return param_.value_for(argv_, name);
    }

    bool
    interpreter
    ::quiet() const
    {
      return quiet_;
    }

    bool
    interpreter
    ::single_shot_mode() const
    {
      return single_shot_;
    }

    unsigned
    interpreter
    ::timeout_multiplier() const
    {
      return timeout_multiplier_ ? timeout_multiplier_.get_value() : 1U;
    }

    bool
    interpreter
    ::honour_timeouts() const
    {
      return !(single_shot_  || disable_timeouts_);
    }

    const char *
    interpreter
    ::tag_specification() const
    {
      return tags_ ? tags_.get_value() : 0;
    }

    bool
    interpreter
    ::verbose_mode() const
    {
      return verbose_;
    }

    bool
    interpreter
    ::xml_output() const
    {
      return xml_.get_value(output_);
    }

    const char *const *
    interpreter
    ::argv() const
    {
      return argv_;
    }

    const char *
    interpreter
    ::program_name() const
    {
      return argv_[0];
    }
  }
}





