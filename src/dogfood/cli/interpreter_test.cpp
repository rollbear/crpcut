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


#include "../../cli/interpreter.hpp"
#include <crpcut.hpp>

namespace {
      static const char usage[] =
"Usage: testprog [flags] {testcases}\n"
"  where flags can be:\n"
#ifdef USE_BACKTRACE
"   -b / --backtrace-heap\n"
"        Store stack backtrace for all heap objects for\n"
"        better error pinpointing of heap violations (slow)\n"
"\n"
#endif
"   -c number / --children=number\n"
"        Control number of concurrently running test processes\n"
"        number must be at least 1\n"
"\n"
"   -C charset / --output-charset=charset\n"
"        Specify the output character set to convert text output\n"
"        to. Does not apply for XML output\n"
"\n"
"   -d dirname / --working-dir=dirname\n"
"        Specify working directory (must exist)\n"
"\n"
"   -i \"id string\" / --identity=\"id string\"\n"
"        Specify an identity string for the XML-header\n"
"\n"
"   -I string / --illegal-char=string\n"
"        Specify how characters that are illegal for the chosen\n"
"        output character set are to be represented\n"
"\n"
"   -l / --list\n"
"        List test cases\n"
"\n"
"   -L / --list-tags\n"
"        List all tags used by tests in the test program\n"
"\n"
"   -n / --nodeps\n"
"        Ignore dependencies\n"
"\n"
"   -o filename / --output=filename\n"
"        Direct XML output to a named file. A brief summary will be\n"
"        displayed on stdout\n"
"\n"
"   -p name=value / --param=name=value\n"
"        Defined a named variable for access from the test cases\n"
"\n"
"   -q / --quiet\n"
"        Don't display the -o brief summary\n"
"\n"
"   -s / --single-shot\n"
"        Run only one test case, and run it in the main process\n"
"        for ease of debugging\n"
"\n"
"   --timeout-multiplier=factor\n"
"        Multiply all timeout times with a factor\n"
"\n"
"   -t / --disable-timeouts\n"
"        Never fail a test due to time consumption\n"
"\n"
"   -T {select}{/non-critical} / --tags={select}{/non-critical}\n"
"        Select tests to run or list based on their tag, and which\n"
"        tags represent non-critical tests. Both \"select\"\n"
"        and \"non-critical\" are comma separated lists of tags.\n"
"        both lists can be empty. If a list begins with \"-\",\n"
"        the list is subtractive from the full set.\n"
"        Untagged tests cannot be made non-critical\n"
"\n"
"   -v / --verbose\n"
"        Verbose mode - include results from passed tests\n"
"\n"
"   -V / --version\n"
"        Print version string and exit\n"
"\n"
"   -x {boolean value} / --xml{=boolean value}\n"
"        XML output on stdout or non-XML output on file\n"
"\n";
}

#define ARGV(...) static const char *argv[] = { "testprog", __VA_ARGS__, 0 }
TESTSUITE(cli)
{
  TESTSUITE(interpreter)
  {
    TEST(construction_without_flags_yields_first_param_as_test_list)
    {
      ARGV("apa", "katt", "orm");
      crpcut::cli::interpreter cli(argv);
      const char *const *rv = cli.get_test_list();
      ASSERT_TRUE(rv == argv + 1);
    }

    TEST(construction_with_unknown_short_form_flag_gives_usage)
    {
      ARGV("-_");
      ASSERT_THROW(crpcut::cli::interpreter cli(argv),
                   crpcut::cli::param::exception&,
                   usage);
    }

    TEST(construction_with_unknown_long_form_flag_gives_usage)
    {
      ARGV("--_");
      ASSERT_THROW(crpcut::cli::interpreter cli(argv),
                   crpcut::cli::param::exception&,
                   usage);
    }

#ifdef USE_BACKTRACE
    TEST(backtrace_is_disabled_when_not_specified_in_argv)
    {
      ARGV("-I", "c", "--output=/dev/null");
      crpcut::cli::interpreter cli(argv);
      const char *const *rv = cli.get_test_list();
      ASSERT_TRUE(rv == argv + 4);
      ASSERT_FALSE(cli.backtrace_enabled());
    }

    TEST(backtrace_is_enabled_when_specified_in_argv)
    {
      ARGV("-I", "c", "--backtrace-heap", "-o", "/dev/null");
      crpcut::cli::interpreter cli(argv);
      const char *const *rv = cli.get_test_list();
      ASSERT_TRUE(rv == argv + 6);
      ASSERT_TRUE(cli.backtrace_enabled());
    }
#endif

    TEST(default_to_one_test_process_if_not_specified)
    {
      ARGV("-x", "-I", "apa");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.num_parallel_tests() == 1U);
    }

    TEST(defined_number_of_test_process_are_returned_specified)
    {
      ARGV("-x", "--children=78931238", "-I", "apa");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.num_parallel_tests() == 78931238U);
      ASSERT_TRUE(cli.get_test_list() == argv + 5);
    }

    TEST(zero_test_processes_throws)
    {
      ARGV("-x", "-c", "0", "-I", "apa");
      ASSERT_THROW(crpcut::cli::interpreter cli(argv),
                   crpcut::cli::param::exception&,
                   "-c number / --children=number - number must be at least 1");
    }

    TEST(output_charset_is_null_if_not_specified_by_argv)
    {
      ARGV("-c", "7", "-o", "apafil");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.get_test_list() == argv + 5);
      ASSERT_FALSE(cli.output_charset());
    }


    TEST(output_charset_is_returned_as_specified_in_argv)
    {
      ARGV("-C", "apa", "-o", "/dev/null", "-x");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.get_test_list() == argv + 6);
      ASSERT_TRUE(cli.output_charset() == std::string("apa"));
    }

    TEST(output_charset_throws_with_xml_mode)
    {
      static const char str[] =
        "-C charset / --output-charset=charset cannot be used with XML reports";
      ARGV("-C", "apa", "-o", "/dev/null");
      ASSERT_THROW(crpcut::cli::interpreter cli(argv),
                   crpcut::cli::param::exception&,
                   str);
    }


    TEST(working_dir_is_null_if_not_specified_in_argv)
    {
      ARGV("-i", "apa", "-o", "/dev/null", "-x");
      crpcut::cli::interpreter cli(argv);
      const char *const *rv = cli.get_test_list();
      ASSERT_TRUE(rv == argv + 6);
      ASSERT_FALSE(cli.working_dir());
    }

    TEST(working_dir_is_returned_as_specified_in_argv)
    {
      ARGV("-i", "apa", "-d", "/dev/null", "-x");
      crpcut::cli::interpreter cli(argv);
      const char *const *rv = cli.get_test_list();
      ASSERT_TRUE(rv == argv + 6);
      ASSERT_TRUE(cli.working_dir() == std::string("/dev/null"));
    }

    TEST(id_string_is_null_if_not_specified_in_argv)
    {
      ARGV("--xml", "-o", "/dev/null");
      crpcut::cli::interpreter cli(argv);
      ASSERT_FALSE(cli.identity_string());
      ASSERT_TRUE(cli.get_test_list() == argv + 4);
    }

    TEST(id_string_is_returned_as_specified_in_argv)
    {
      ARGV("--xml", "--identity=apa");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.identity_string() == std::string("apa"));
      ASSERT_TRUE(cli.get_test_list() == argv + 3);
    }


    TEST(illegal_representation_is_null_if_not_specified_in_argv)
    {
      ARGV("--xml", "-i", "apa", "katt");
      crpcut::cli::interpreter cli(argv);
      ASSERT_FALSE(cli.illegal_representation());
      ASSERT_TRUE(cli.get_test_list() == argv + 4);
    }

    TEST(illegal_representation_is_returned_as_specified_in_argv)
    {
      ARGV("--xml", "-I", "apa", "katt");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.illegal_representation() == argv[3]);
      ASSERT_TRUE(cli.get_test_list() == argv + 4);
    }

    TEST(list_tests_is_false_if_not_specified_in_argv)
    {
      ARGV("--tags=-apa", "apa", "katt");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.get_test_list() == argv + 2);
      ASSERT_FALSE(cli.list_tests());
    }

    TEST(list_tests_is_true_when_specified_in_argv)
    {
      ARGV("--tags=-apa", "--list", "apa", "katt");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.list_tests());
      ASSERT_TRUE(cli.get_test_list() == argv + 3);
    }

    TEST(list_tags_is_false_if_not_specified_in_argv)
    {
      ARGV("--tags=-apa", "--list", "apa", "katt");
      crpcut::cli::interpreter cli(argv);
      ASSERT_FALSE(cli.list_tags());
      ASSERT_TRUE(cli.get_test_list() == argv + 3);
    }

    TEST(list_tags_is_true_if_specified_in_argv)
    {
      ARGV("--list-tags", "apa", "katt");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.list_tags());
      ASSERT_TRUE(cli.get_test_list() == argv + 2);
    }

    TEST(honour_dependencies_is_true_if_not_specified_otherwise)
    {
      ARGV("-q", "--xml", "-o", "/dev/null", "apa");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.honour_dependencies());
      ASSERT_TRUE(cli.get_test_list() == argv + 5);
    }

    TEST(honour_dependencies_is_false_when_so_specified)
    {
      ARGV("-q", "--xml", "--nodeps", "-o", "/dev/null", "apa");
      crpcut::cli::interpreter cli(argv);
      ASSERT_FALSE(cli.honour_dependencies());
      ASSERT_TRUE(cli.get_test_list() == argv + 6);
    }

    TEST(report_file_is_null_when_output_directive_is_missing_in_argv)
    {
      ARGV("-i", "7",  "-I", "apa");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.get_test_list() == argv + 5);
      ASSERT_FALSE(cli.report_file());
    }

    TEST(report_file_ordered_in_argv_is_returned)
    {
      ARGV("-i", "7",  "-o", "apa");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.get_test_list() == argv + 5);
      ASSERT_TRUE(cli.report_file() == std::string("apa"));
    }

    TEST(named_param_lookup_returns_first_exact_match)
    {
      ARGV("-q", "-p", "apa=katt", "--param=apa=ko", "lemur");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.get_test_list() == argv + 5);
      ASSERT_TRUE(cli.named_parameter("apa") == argv[3] + 4);
    }

    TEST(lookup_nonspecified_named_param_returns_null)
    {
      ARGV("-q", "-p", "apa=katt", "--param=ko=tupp", "lemur");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.get_test_list() == argv + 5);
      ASSERT_FALSE(cli.named_parameter("orm"));
    }

    TEST(quiet_mode_is_disabled_if_not_activated_in_argv)
    {
      ARGV("-i", "name", "--param=apa=katt", "-x", "orm", "ko");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.get_test_list() == argv + 5);
      ASSERT_FALSE(cli.quiet());
    }

    TEST(quiet_mode_is_enabled_when_activated_in_argv)
    {
      ARGV("-i", "name", "--param=apa=katt", "-q", "orm", "ko");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.get_test_list() == argv + 5);
      ASSERT_TRUE(cli.quiet());
    }

    TEST(single_shot_mode_is_not_active_if_missing_in_argv)
    {
      ARGV("apa");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.get_test_list() == argv + 1);
      ASSERT_FALSE(cli.single_shot_mode());
    }

    TEST(single_shot_mode_is_actived_by_argv)
    {
      ARGV("-s", "apa");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.get_test_list() == argv + 2);
      ASSERT_TRUE(cli.single_shot_mode());
    }

    TEST(timeout_multiplier_is_one_if_missing_in_argv)
    {
      ARGV("-x", "--param=apa=katt", "-n");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.timeout_multiplier() == 1U);
    }

    TEST(zero_timeout_multiplier_throws)
    {
      ARGV("-x", "--timeout-multiplier=0", "-n");
      ASSERT_THROW(crpcut::cli::interpreter cli(argv),
                   crpcut::cli::param::exception&,
                   "--timeout-multiplier=factor - factor must be at least 1");
    }

    TEST(timeout_multiplier_can_be_set_via_command_line_long_form)
    {
      ARGV("--param=apa=katt", "--timeout-multiplier=28", "-n");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.timeout_multiplier() == 28U);
    }

    TEST(single_shot_mode_cannot_be_combined_with_timeout_multiplier)
    {
      ARGV("--timeout-multiplier=5", "--param=apa=katt", "-s");
      ASSERT_THROW(crpcut::cli::interpreter cli(argv),
                   crpcut::cli::param::exception&,
                   "--timeout-multiplier=factor cannot be combined with -s / --single-shot");
    }

    TEST(single_shot_mode_cannot_be_combined_with_xml_output)
    {
      ARGV("-s", "-x", "apa");
      static const char msg[]= "-s / --single-shot cannot be combined with -x {boolean value} / --xml{=boolean value}";
      ASSERT_THROW(crpcut::cli::interpreter cli(argv),
                   crpcut::cli::param::exception&,
                   msg);
    }

    TEST(timeouts_are_honoured_when_not_disabled_in_argv)
    {
      ARGV("-x", "-o", "/dev/null", "apa", "katt");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.honour_timeouts());
      ASSERT_TRUE(cli.get_test_list() == argv + 4);
    }

    TEST(timeouts_are_disabled_by_argv_flag)
    {
      ARGV("-x", "-o", "/dev/null", "-t", "apa", "katt");
      crpcut::cli::interpreter cli(argv);
      ASSERT_FALSE(cli.honour_timeouts());
      ASSERT_TRUE(cli.get_test_list() == argv + 5);
    }

    TEST(disable_timeouts_and_single_shot_throws)
    {
      ARGV("-s", "-t", "apa", "katt");
      ASSERT_THROW(crpcut::cli::interpreter cli(argv),
                   crpcut::cli::param::exception&,
                   "-s / --single-shot cannot be combined with -t / --disable-timeouts");
    }

    TEST(disable_timeouts_and_timeout_multiplier_throws)
    {
      ARGV("-t", "--timeout-multiplier=5");
      ASSERT_THROW(crpcut::cli::interpreter cli(argv),
                   crpcut::cli::param::exception&,
                   "--timeout-multiplier=factor cannot be combined with -t / --disable-timeouts");
    }

    TEST(tags_specification_is_null_if_not_included_in_argv)
    {
      ARGV("-x", "--output=/dev/null", "--disable-timeouts", "apa");
      crpcut::cli::interpreter cli(argv);
      ASSERT_FALSE(cli.tag_specification());
      ASSERT_TRUE(cli.get_test_list() == argv + 4);
    }

    TEST(tag_specification_is_returned_as_in_argv)
    {
      ARGV("-x", "--output=/dev/null", "--tags=-apa/katt", "apa");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.tag_specification() == argv[3] + 7);
      ASSERT_TRUE(cli.get_test_list() == argv + 4);
    }

    TEST(verbose_mode_is_disabled_if_not_activated_by_argv)
    {
      ARGV("--xml=false", "--output=/dev/null", "--tags=-apa/katt", "apa");
      crpcut::cli::interpreter cli(argv);
      ASSERT_FALSE(cli.verbose_mode());
      ASSERT_TRUE(cli.get_test_list() == argv + 4);
    }

    TEST(verbose_mode_is_enabled_if_activated_in_argv)
    {
      ARGV("--xml=false", "--output=/dev/null", "--verbose", "apa");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.verbose_mode());
      ASSERT_TRUE(cli.get_test_list() == argv + 4);
    }

    TEST(output_mode_defaults_to_xml_if_output_file_is_ordered)
    {
      ARGV("-i", "7", "-o", "apafil");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.get_test_list() == argv + 5);
      ASSERT_TRUE(cli.xml_output());
    }


    TEST(output_mode_defaults_to_text_if_output_file_is_not_ordered)
    {
      ARGV("-i", "7", "-I", "apa");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.get_test_list() == argv + 5);
      ASSERT_FALSE(cli.xml_output());
    }

    TEST(output_mode_is_xml_without_file_if_ordered_by_flip)
    {
      ARGV("-i", "7", "-x", "-I", "apa");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.get_test_list() == argv + 6);
      ASSERT_TRUE(cli.xml_output());
    }

    TEST(output_mode_is_text_with_file_if_ordered_by_flip)
    {
      ARGV("-i", "7", "-x", "-o", "apa");
      crpcut::cli::interpreter cli(argv);
      ASSERT_TRUE(cli.get_test_list() == argv + 6);
      ASSERT_FALSE(cli.xml_output());
    }

    TEST(version_is_reported_as_param_exception)
    {
      ARGV("-i", "7", "--version", "-o", "apa");
      ASSERT_THROW(crpcut::cli::interpreter cli(argv),
                   crpcut::cli::param::exception&,
                   "crpcut-" CRPCUT_VERSION_STRING);
    }

    TEST(timeouts_are_not_honoured_in_single_shot_mode)
    {
      ARGV("-s", "apa");
      crpcut::cli::interpreter cli(argv);
      ASSERT_FALSE(cli.honour_timeouts());
    }

  }
}
