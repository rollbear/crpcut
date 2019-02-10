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

#include <trompeloeil.hpp>
#include <crpcut.hpp>
#include "../test_runner.hpp"
#include "../process_control.hpp"
#include "../filesystem_operations.hpp"
#include "../poll.hpp"
#include <fstream>

extern "C"
{
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <fcntl.h>
#  include <unistd.h>
#  include <sys/wait.h>
}
TESTSUITE(test_runner)
{
  using trompeloeil::_;

  class mock_runner : public crpcut::test_runner
  {
  public:
    MAKE_MOCK2(introduce_test, void(pid_t,
                                    const crpcut::crpcut_test_case_registrator *));
    MAKE_MOCK5(present, void(pid_t, crpcut::comm::type, crpcut::test_phase,
                             std::size_t, const char *));
    MAKE_MOCK1(set_deadline, void(crpcut::crpcut_test_case_registrator*));
    MAKE_MOCK1(clear_deadline, void(crpcut::crpcut_test_case_registrator*));
    MAKE_MOCK1(return_dir, void(unsigned));
    MAKE_MOCK1(calc_cputime, unsigned long(const struct timeval&));
  };

  class mock_environment : public crpcut::test_environment
  {
  public:
    MAKE_CONST_MOCK0(tests_as_child_procs, bool());
    MAKE_CONST_MOCK0(timeouts_enabled, bool());
    MAKE_MOCK1(set_charset, void(const char *));
    MAKE_CONST_MOCK0(get_charset, const char *());
    MAKE_CONST_MOCK0(get_output_charset, const char*());
    MAKE_CONST_MOCK0(get_illegal_rep, const char*());
    MAKE_CONST_MOCK0(timeout_multiplier, unsigned());
    MAKE_CONST_MOCK0(get_start_dir, const char *());
    MAKE_CONST_MOCK1(get_parameter, const char *(const char *name));

  };
  class mock_reporter : public crpcut::comm::reporter
  {
  public:
    mock_reporter(std::ostream &os) : reporter(os) {}
    MAKE_CONST_MOCK5(report, void(crpcut::comm::type,
                                  const char *p,
                                  size_t len,
                                  std::string,
                                  const crpcut::crpcut_test_monitor*));
    void report(crpcut::comm::type t, const char *msg, size_t len,
                crpcut::datatypes::fixed_string loc,
                const crpcut::crpcut_test_monitor* mon) const
    {
      report(t, msg, len, std::string(loc.str, loc.len), mon);
    }
  };

  class mock_process_control : public crpcut::process_control
  {
  public:
    MAKE_MOCK2(getrusage, int(int, struct rusage*));
    MAKE_MOCK2(killpg, int(int, int));
    MAKE_MOCK4(waitid, int(idtype_t, id_t, siginfo_t*, int));
  };

  class mock_fsops : public crpcut::filesystem_operations
  {
  public:
    MAKE_MOCK1(chdir, int(const char*));
    MAKE_MOCK2(mkdir, int(const char *n, mode_t m));
    MAKE_MOCK2(rename, int(const char *, const char*));
  };


  class mock_poll : public crpcut::poll<crpcut::fdreader>
  {
  public:
    MAKE_MOCK3(do_add_fd, void(int, crpcut::fdreader *, int));
    MAKE_MOCK1(do_del_fd, void(int));
    MAKE_MOCK1(do_wait, descriptor(int));
    MAKE_CONST_MOCK0(do_num_fds, std::size_t());
  };

  template <typename dump_handler>
    struct test_registrator : public crpcut::crpcut_test_case_registrator,
                              public virtual dump_handler
  {
  public:
    test_registrator(const char                    *name,
                     const crpcut::namespace_info  &ns,
                     unsigned long                  cpulimit,
                     crpcut::comm::reporter        *reporter,
                     crpcut::process_control       *proc,
                     crpcut::filesystem_operations *fsops,
                     crpcut::test_runner     *factory)
    : crpcut_test_case_registrator(name,
                                   crpcut::datatypes::fixed_string::make(""),
                                   ns,
                                   cpulimit,
                                   reporter,
                                   proc,
                                   fsops,
                                   factory)
    {
    }
    using crpcut::crpcut_test_case_registrator::set_test_environment;
    using crpcut::crpcut_test_case_registrator::set_cputime_at_start;
    using crpcut::crpcut_test_case_registrator::prepare_construction;
    using crpcut::crpcut_test_case_registrator::manage_test_case_execution;
    using crpcut::crpcut_test_case_registrator::prepare_destruction;
    MAKE_MOCK4(setup, void(crpcut::poll<crpcut::fdreader>&, int, int, int));
    MAKE_CONST_MOCK0(get_location, crpcut::datatypes::fixed_string());
    MAKE_CONST_MOCK0(crpcut_tag, crpcut::tag&());
    MAKE_CONST_MOCK0(get_importance, crpcut::tag::importance());
    MAKE_MOCK0(run_test_case, void());
    MAKE_CONST_MOCK1(crpcut_is_expected_exit, bool(int));
    MAKE_CONST_MOCK1(crpcut_is_expected_signal, bool(int));
    MAKE_CONST_MOCK1(crpcut_on_ok_action, void(const char*));
    MAKE_CONST_MOCK1(crpcut_calc_deadline, unsigned long(unsigned long));
    MAKE_CONST_MOCK1(crpcut_expected_death, void(std::ostream&));
  };

  class mock_testcase : public crpcut::crpcut_test_case_base
  {
  public:
    MAKE_MOCK0(crpcut_run_test, void());
    MAKE_MOCK0(test, void());
    MAKE_CONST_MOCK0(crpcut_tag, crpcut::tag&());
    MAKE_CONST_MOCK0(crpcut_get_reg, crpcut::crpcut_test_case_registrator&());
  };

  static const char loc_str[] = "apa.cpp:3";
  static const size_t loc_len = sizeof(loc_str) - 1U;

  template <unsigned long cpulimit,
            class dump_policy = crpcut::policies::core_dumps::crpcut_default_handler>
  struct fix
  {
    using fixed_string = crpcut::datatypes::fixed_string;
    fix()
    : default_out(),
      runner(),
      reporter(default_out),
      process_control(),
      fsops(),
      top_namespace{nullptr,nullptr},
      current_namespace{"apa", &top_namespace},
      reg("test", current_namespace, cpulimit * 1000,
          &reporter, &process_control, &fsops, &runner)
    {
      reg.set_test_environment(&env);
    }
    template <size_t N1, size_t N2>
    void make_message(char (&buffer)[N1], const char (&msg)[N2])
    {
      const size_t len = loc_len;
      memcpy(buffer, &len, sizeof(len));
      memcpy(buffer + sizeof(len), loc_str, len);
      memcpy(buffer + sizeof(len) + len, msg, N2 - 1);
    }
    void setup(pid_t pid)
    {
      mock_poll poller;
      reg.set_pid(pid);
      //reg.setup(poller, -1, -1, -1);
    }
    void prepare_siginfo(pid_t pid, int status, int code, trompeloeil::sequence&)
    {
      siginfo_t info;
      info.si_pid    = pid;
      info.si_signo  = SIGCHLD;
      info.si_status = status;
      info.si_code   = code;

      auto idpid = id_t(pid);
      waitid_x2 = NAMED_REQUIRE_CALL(process_control, waitid(P_PGID, idpid, _, WEXITED))
        .SIDE_EFFECT(errno = ECHILD)
        .RETURN(-1);

      waitid_x1 = NAMED_REQUIRE_CALL(process_control, waitid(P_PGID, idpid, _, WEXITED))
        .SIDE_EFFECT(*_3 = info)
        .RETURN(0);
    }

    std::ostringstream                    default_out;
    mock_environment          env;
    mock_runner               runner;
    mock_reporter                         reporter;
    mock_process_control      process_control;
    mock_fsops                fsops;
    crpcut::namespace_info                top_namespace;
    crpcut::namespace_info                current_namespace;
    test_registrator<dump_policy>         reg;
    std::unique_ptr<trompeloeil::expectation> waitid_x1;
    std::unique_ptr<trompeloeil::expectation> waitid_x2;
  };

  TESTSUITE(just_constructed)
  {
    TEST(prints_its_full_name_on_ostream, fix<100U>)
    {
      std::ostringstream os;
      os << reg;
      ASSERT_TRUE(os.str() == "apa::test");
      ASSERT_TRUE(reg.full_name_len() == 9U);
    }

    TEST(reports_true_on_exact_name_match, fix<100U>)
    {
      ASSERT_TRUE(reg.match_name("apa::test"));
    }

    TEST(reports_false_on_mismatching_suite_for_name_match, fix<1U>)
    {
      ASSERT_FALSE(reg.match_name("katt::test"));
    }

    TEST(reports_false_on_mismatching_test_name_in_matching_suite, fix<1U>)
    {
      ASSERT_FALSE(reg.match_name("apa::tet"));
    }

    TEST(reports_true_on_matching_suite_name, fix<1U>)
    {
      ASSERT_TRUE(reg.match_name("apa"));
    }

    TEST(reports_false_on_colon_errors, fix<1U>)
    {
      ASSERT_FALSE(reg.match_name("apa:"));
    }

    TEST(reports_false_on_suite_match_with_trailing_double_colon, fix<1U>)
    {
      ASSERT_FALSE(reg.match_name("apa::"));
    }

    TEST(reports_false_on_suite_match_with_prepended_double_colon, fix<1U>)
    {
      ASSERT_FALSE(reg.match_name("::apa"));
    }
  }

#define SET_WD(num)                              \
  REQUIRE_CALL(fsops, mkdir(_, 0700U))           \
    .WITH(_1 == std::string(#num))               \
    .RETURN(0);                                  \
    reg.set_wd(num)

  TESTSUITE(set_wd)
  {
    TEST(creates_working_dir, fix<100U>)
    {
      REQUIRE_CALL(fsops, mkdir(_, 0700U))
        .WITH(_1 == std::string("100"))
        .RETURN(0);
      reg.set_wd(100);
    }

    TEST(accepts_already_existing_dir, fix<100U>)
    {
      REQUIRE_CALL(fsops, mkdir(_, 0700U))
        .WITH(_1 == std::string("100"))
        .SIDE_EFFECT(errno = EEXIST)
        .RETURN(-1);
      reg.set_wd(100);
    }

    TEST(aborts_on_failure, fix<100U>,
         EXPECT_SIGNAL_DEATH(SIGABRT),
         NO_CORE_FILE,
         DEPENDS_ON(accepts_already_existing_dir,
                    creates_working_dir))
    {
      REQUIRE_CALL(fsops, mkdir(_, 0700U))
        .WITH(std::string(_1) == "100")
        .SIDE_EFFECT(errno = EACCES)
        .RETURN(-1);
      reg.set_wd(100);
    }
  }

  TESTSUITE(goto_wd, DEPENDS_ON(ALL_TESTS(set_wd)))
  {
    TEST(changes_to_working_dir, fix<100U>)
    {
      REQUIRE_CALL(fsops, mkdir(_,_)).RETURN(0);
      reg.set_wd(101);
      REQUIRE_CALL(fsops, chdir(_))
        .WITH(_1 == std::string("101"))
        .RETURN(0);
      reg.goto_wd();
    }

    class my_exit {};

    TEST(reports_failure_to_enter, fix<100U>,
         DEPENDS_ON(changes_to_working_dir))
    {
      REQUIRE_CALL(fsops, mkdir(_,_)).RETURN(0);
      REQUIRE_CALL(fsops, chdir(_))
        .WITH(_1 == std::string("101"))
        .SIDE_EFFECT(errno = EACCES)
        .RETURN(-1);

      REQUIRE_CALL(reg, get_location())
        .RETURN(fixed_string::make(loc_str));
      REQUIRE_CALL(reporter, report(crpcut::comm::exit_fail,
                                    _,
                                    _,
                                    "apa.cpp:3",
                                    _))
        .WITH(std::string(_2,_3) == "Couldn't chdir working dir")
        .THROW(my_exit{});
      reg.set_wd(101);
      ASSERT_THROW(reg.goto_wd(), my_exit);
    }

    TEST(returning_report_exit_fail_after_failure_to_enter_aborts, fix<100U>,
         DEPENDS_ON(reports_failure_to_enter),
         EXPECT_SIGNAL_DEATH(SIGABRT),
         NO_CORE_FILE)
    {
      REQUIRE_CALL(fsops, mkdir(_,_)).RETURN(0);
      REQUIRE_CALL(fsops, chdir(_))
        .WITH(_1 == std::string("101"))
        .SIDE_EFFECT(errno = EACCES)
        .RETURN(-1);
      REQUIRE_CALL(reg, get_location())
        .RETURN(fixed_string::make(loc_str));
      REQUIRE_CALL(reporter, report(crpcut::comm::exit_fail,
                                    _,
                                    _,
                                    "apa.cpp:3",
                                    _))
        .WITH(std::string(_2,_3) == "Couldn't chdir working dir");
      reg.set_wd(101);
      reg.goto_wd();
    }
  }

  TEST(init_on_registrator_adds_fds_to_poller_and_stores_pid, fix<100U>)
  {
    setup(101);
    ASSERT_TRUE(reg.get_pid() == 101);
  }

  TESTSUITE(prepare_construction)
  {
    TEST(reports_deadline_multiplied_when_run_as_child_process, fix<10U>)
    {
      unsigned factor = 20;
      unsigned requested_timeout = 100;

      REQUIRE_CALL(env, tests_as_child_procs())
        .RETURN(true);
      REQUIRE_CALL(env, timeout_multiplier())
        .RETURN(factor);
      unsigned long result = requested_timeout*factor;
      const void *addr = &result;
      const char *result_str = static_cast<const char*>(addr);
      REQUIRE_CALL(reporter, report(crpcut::comm::set_timeout,
                                   _,
                                   sizeof(result),
                                   "",
                                   _))
        .WITH(memcmp(_2, result_str, _3) == 0);
      reg.prepare_construction(requested_timeout);
    }

    TEST(does_nothing_when_run_in_single_process, fix<10U>)
    {
      REQUIRE_CALL(env, tests_as_child_procs())
        .RETURN(false);
      unsigned requested_timeout(100);
      reg.prepare_construction(requested_timeout);
    }
}

  TESTSUITE(prepare_destruction)
  {
    TEST(reports_deadline_multiplied_if_timeouts_are_enabled, fix<10U>)
    {
      unsigned factor = 20;
      unsigned requested_timeout = 100;

      REQUIRE_CALL(env, timeouts_enabled())
        .RETURN(true);

      REQUIRE_CALL(env, timeout_multiplier())
        .RETURN(factor);

      unsigned long result = requested_timeout*factor;
      const void *addr = &result;
      const char *result_str = static_cast<const char*>(addr);
      REQUIRE_CALL(reporter, report(crpcut::comm::set_timeout,
                                    _,
                                    sizeof(result),
                                    "",
                                    _))
        .WITH(memcmp(_2, result_str, _3) == 0);
      reg.prepare_destruction(requested_timeout);
    }

    TEST(does_nothing_if_timeouts_are_disabled, fix<10U>)
    {
      REQUIRE_CALL(env, timeouts_enabled())
        .RETURN(false);
      reg.prepare_destruction(100U);
    }
  }


  TESTSUITE(manage_test_case_execution)
  {
    TEST(reports_cputime_if_run_as_child_procs, fix<10U>)
    {
      ALLOW_CALL(env, tests_as_child_procs())
        .RETURN(true);

      static const struct timeval utime = { 1, 5 };
      static const struct timeval stime = { 3, 6 };
      trompeloeil::sequence s;
      struct rusage usage;
      usage.ru_utime = utime;
      usage.ru_stime = stime;
      ALLOW_CALL(process_control, getrusage(RUSAGE_SELF, _))
        .SIDE_EFFECT(*_2 = usage)
        .RETURN(0);

      struct timeval result = { 4, 11 };
      const void *addr = &result;
      const char *result_str = static_cast<const char*>(addr);

      REQUIRE_CALL(reporter, report(crpcut::comm::begin_test, _, sizeof(result), "",_))
        .WITH(memcmp(_2, result_str, _3) == 0)
        .IN_SEQUENCE(s);

      mock_testcase test_obj;
      REQUIRE_CALL(test_obj, crpcut_run_test())
        .IN_SEQUENCE(s);
      reg.manage_test_case_execution(&test_obj);
    }

    TEST(reports_success_if_not_run_as_child_procs, fix<10U>)
    {
      ALLOW_CALL(env, tests_as_child_procs())
          .RETURN(false);

      mock_testcase test_obj;

      ASSERT_FALSE(reg.crpcut_failed());
      ASSERT_FALSE(reg.crpcut_succeeded());
      ASSERT_TRUE(reg.crpcut_can_run());
      REQUIRE_CALL(test_obj, crpcut_run_test());
      reg.manage_test_case_execution(&test_obj);
      ASSERT_FALSE(reg.crpcut_failed());
      ASSERT_TRUE(reg.crpcut_succeeded());
    }
  }

  TESTSUITE(manage_death)
  {
    class test_tag_root : public crpcut::tag_list_root
    {
    public:
      crpcut::datatypes::fixed_string get_name() const
      {
        return crpcut::datatypes::fixed_string::make("");
      }
    };
    test_tag_root tag_root;
#define TAG(name)                                              \
    class name ## _tag : public crpcut::tag                    \
    {                                                          \
    public:                                                    \
      name ## _tag(crpcut::tag_list_root *r)                   \
        : crpcut::tag(sizeof(#name) - 1, r) {}                 \
      virtual crpcut::datatypes::fixed_string get_name() const \
      {                                                        \
        return crpcut::datatypes::fixed_string::make(#name);   \
      }                                                        \
    };                                                         \
    name ## _tag name(&tag_root)
    TAG(apa);

    struct end_data
    {
      bool          importance;
      unsigned long duration_us;
    };
    TEST(expected_exit_without_prior_fail_reports_success, fix<10U>)
    {

      trompeloeil::sequence s;

      const pid_t test_pid = 101;
      setup(test_pid);
      SET_WD(9);
      reg.set_phase(crpcut::destroying);
      prepare_siginfo(test_pid, 0, CLD_EXITED, s);

      REQUIRE_CALL(runner, calc_cputime(_))
        .RETURN(10U);

      REQUIRE_CALL(reg, crpcut_is_expected_exit(0))
        .IN_SEQUENCE(s)
        .RETURN(true);

      REQUIRE_CALL(reg, crpcut_on_ok_action(_));

      REQUIRE_CALL(runner, present(test_pid, crpcut::comm::exit_ok,
                                   crpcut::destroying,
                                   0U, _))
        .IN_SEQUENCE(s);

      REQUIRE_CALL(runner, return_dir(9U));

      REQUIRE_CALL(runner, present(test_pid, crpcut::comm::end_test,
                                   crpcut::destroying,
                                   sizeof(end_data), _))
        .IN_SEQUENCE(s);

      ALLOW_CALL(reg, crpcut_tag())
        .RETURN(std::ref(apa));

      reg.manage_death();
    }

    TEST(core_dump_gives_file_and_fail_report, fix<100U>)
    {
      ::mkdir("99", 0750);
      ::creat("99/junk_file", 0640);
      {
        trompeloeil::sequence s;
        const pid_t test_pid = 155;
        setup(test_pid);
        reg.set_phase(crpcut::running);
        SET_WD(99);
        prepare_siginfo(test_pid, 0, CLD_DUMPED, s);


        REQUIRE_CALL(runner, calc_cputime(_))
          .RETURN(10U);

        REQUIRE_CALL(runner, present(test_pid,
                                     crpcut::comm::dir,
                                     crpcut::running,
                                     0U, nullptr))
          .IN_SEQUENCE(s);

        ALLOW_CALL(reg, get_location())
          .RETURN(fixed_string::make(loc_str));
        REQUIRE_CALL(fsops, rename(_,_))
          .WITH(_1 == std::string("99"))
          .WITH(_2 == std::string("apa::test"))
          .RETURN(0);
        const char msg[] = "Died with core dump";
        char buffer[sizeof(msg) - 1 + loc_len + sizeof(size_t)];
        make_message(buffer, msg);
        REQUIRE_CALL(runner, present(test_pid,
                                     crpcut::comm::exit_fail,
                                     crpcut::running,
                                     _, _))
          .WITH(_5 != nullptr)
          .WITH(memcmp(_5, buffer, _4) == 0)
          .IN_SEQUENCE(s);

        REQUIRE_CALL(runner, return_dir(99U));

        REQUIRE_CALL(runner, present(test_pid, crpcut::comm::end_test,
                                     crpcut::running,
                                     sizeof(end_data), _))
          .IN_SEQUENCE(s);
        ALLOW_CALL(reg, crpcut_tag())
          .RETURN(std::ref(apa));

        reg.manage_death();
        ASSERT_TRUE(apa.num_failed() == 0U);
        ASSERT_TRUE(apa.num_passed() == 0U);
      }
      ::remove("99/junk_file");
      ::rmdir("99");
    }

    template <int n>
    void set_expected_exit_msg(std::ostream &os)
    {
      os << "exit " << n;
    }


    TEST(exit_with_wrong_code_gives_fail_report, fix<100U>)
    {
      trompeloeil::sequence s;
      const pid_t test_pid = 20123;
      setup(test_pid);
      const crpcut::test_phase phase = crpcut::destroying;
      reg.set_phase(phase);
      SET_WD(3);
      prepare_siginfo(test_pid, 3, CLD_EXITED, s);

      REQUIRE_CALL(runner, calc_cputime(_))
        .RETURN(10U);

      REQUIRE_CALL(reg, crpcut_is_expected_exit(3))
        .IN_SEQUENCE(s)
        .RETURN(false);

      REQUIRE_CALL(reg, crpcut_expected_death(_))
        .IN_SEQUENCE(s)
        .SIDE_EFFECT(set_expected_exit_msg<0>(_1));

      REQUIRE_CALL(reg, get_location())
        .RETURN(fixed_string::make(loc_str));

      const char msg[] = "Exited with code 3\nExpected exit 0";
      char buffer[sizeof(msg) - 1 + loc_len  + sizeof(size_t)];
      make_message(buffer, msg);
      REQUIRE_CALL(runner, present(test_pid, crpcut::comm::exit_fail, phase, _,_))
        .WITH(memcmp(_5, buffer, _4) == 0)
        .IN_SEQUENCE(s);

      REQUIRE_CALL(runner, return_dir(3U));

      ALLOW_CALL(reg, crpcut_tag())
        .RETURN(std::ref(apa));

      REQUIRE_CALL(runner, present(test_pid, crpcut::comm::end_test, phase, _,_))
        .IN_SEQUENCE(s);

      reg.manage_death();
      ASSERT_TRUE(apa.num_failed() == 0U);
      ASSERT_TRUE(apa.num_passed() == 0U);
    }

    TEST(exit_with_wrong_signal_gives_fail_report, fix<100U>)
    {
      trompeloeil::sequence s;
      const pid_t test_pid = 3216;
      setup(test_pid);
      const crpcut::test_phase phase = crpcut::running;
      reg.set_phase(phase);
      SET_WD(100);
      prepare_siginfo(test_pid, 15, CLD_KILLED, s);

      REQUIRE_CALL(runner, calc_cputime(_))
        .RETURN(10U);

      REQUIRE_CALL(reg, crpcut_is_expected_signal(15))
        .IN_SEQUENCE(s)
        .RETURN(false);

      REQUIRE_CALL(reg, get_location())
        .RETURN(fixed_string::make(loc_str));
      REQUIRE_CALL(reg, crpcut_expected_death(_))
        .IN_SEQUENCE(s)
        .SIDE_EFFECT(set_expected_exit_msg<0>(_1));

      const char msg[] = "Died on signal 15\nExpected exit 0";
      char buffer[sizeof(msg) - 1 + loc_len  + sizeof(size_t)];
      make_message(buffer, msg);

      REQUIRE_CALL(runner, present(test_pid, crpcut::comm::exit_fail, phase, _,_))
        .WITH(memcmp(_5, buffer, _4) == 0)
        .IN_SEQUENCE(s);

      REQUIRE_CALL(runner, return_dir(100U));

      ALLOW_CALL(reg, crpcut_tag())
        .RETURN(std::ref(apa));

      REQUIRE_CALL(runner, present(test_pid, crpcut::comm::end_test, phase, _,_))
        .IN_SEQUENCE(s);

      reg.manage_death();
      ASSERT_TRUE(apa.num_failed() == 0U);
      ASSERT_TRUE(apa.num_passed() == 0U);
    }
    TEST(exit_with_wrong_signal_gives_timeout_report_if_killed,
         fix<100U>)
    {
      trompeloeil::sequence s;
      const pid_t test_pid = 5158;
      setup(test_pid);
      const crpcut::test_phase phase = crpcut::running;
      reg.set_phase(phase);
      SET_WD(1);

      REQUIRE_CALL(process_control, killpg(test_pid, 9))
        .IN_SEQUENCE(s)
        .RETURN(0);

      prepare_siginfo(test_pid, 9, CLD_KILLED, s);

      REQUIRE_CALL(runner, calc_cputime(_))
        .RETURN(1000000U);

      REQUIRE_CALL(reg, crpcut_is_expected_signal(9))
        .IN_SEQUENCE(s)
        .RETURN(false);

      REQUIRE_CALL(reg, get_location())
        .RETURN(fixed_string::make(loc_str));

      REQUIRE_CALL(reg, crpcut_expected_death(_))
        .IN_SEQUENCE(s)
        .SIDE_EFFECT(set_expected_exit_msg<0>(_1));

      const char msg[] = "Timed out - killed\nExpected exit 0";
      char buffer[sizeof(msg) - 1 + loc_len  + sizeof(size_t)];
      make_message(buffer, msg);

      REQUIRE_CALL(runner, present(test_pid, crpcut::comm::exit_fail, phase, _,_))
        .WITH(memcmp(_5, buffer, _4) == 0)
        .IN_SEQUENCE(s);

      REQUIRE_CALL(runner, return_dir(1U));

      ALLOW_CALL(reg, crpcut_tag())
        .RETURN(std::ref(apa));

      REQUIRE_CALL(runner, present(test_pid, crpcut::comm::end_test, phase, _,_))
        .IN_SEQUENCE(s);

      reg.kill();
      reg.manage_death();
      ASSERT_TRUE(apa.num_failed() == 0U);
      ASSERT_TRUE(apa.num_passed() == 0U);
    }

    TEST(expected_signal_gives_timeout_report_after_excessive_cputime,
         fix<100U>)
    {
      trompeloeil::sequence s;
      const pid_t test_pid = 5158;
      setup(test_pid);
      const crpcut::test_phase phase = crpcut::running;
      reg.set_phase(phase);
      SET_WD(1);

      prepare_siginfo(test_pid, 6, CLD_KILLED, s);

      REQUIRE_CALL(runner, calc_cputime(_))
        .RETURN(1000000U);

      REQUIRE_CALL(reg, crpcut_is_expected_signal(6))
        .IN_SEQUENCE(s)
        .RETURN(true);

      REQUIRE_CALL(reg, get_location())
        .RETURN(fixed_string::make(loc_str));
      REQUIRE_CALL(env, timeouts_enabled())
        .RETURN(true);

      const char msg[] = "Test consumed 1000ms CPU-time\nLimit was 100ms";
      char buffer[sizeof(msg) - 1 + loc_len  + sizeof(size_t)];
      make_message(buffer, msg);

      REQUIRE_CALL(runner, present(test_pid, crpcut::comm::exit_fail, phase, _,_))
        .WITH(memcmp(_5, buffer, _4) == 0)
        .IN_SEQUENCE(s);

      REQUIRE_CALL(runner, return_dir(1U));

      ALLOW_CALL(reg, crpcut_tag())
        .RETURN(std::ref(apa));

      REQUIRE_CALL(runner, present(test_pid, crpcut::comm::end_test, phase, _,_))
        .IN_SEQUENCE(s);

      reg.manage_death();
      ASSERT_TRUE(apa.num_failed() == 0U);
      ASSERT_TRUE(apa.num_passed() == 0U);
    }

    TEST(expected_signal_gives_no_timeout_report_when_timeouts_are_disabled,
         fix<100U>)
    {
      trompeloeil::sequence s;
      const pid_t test_pid = 5158;
      setup(test_pid);
      const crpcut::test_phase phase = crpcut::running;
      reg.set_phase(phase);
      SET_WD(1);

      prepare_siginfo(test_pid, 6, CLD_KILLED, s);

      REQUIRE_CALL(runner, calc_cputime(_))
        .RETURN(1000000U);

      REQUIRE_CALL(reg, crpcut_is_expected_signal(6))
        .IN_SEQUENCE(s)
        .RETURN(true);

      REQUIRE_CALL(reg, crpcut_on_ok_action(_));
      REQUIRE_CALL(env, timeouts_enabled())
        .RETURN(false);

      REQUIRE_CALL(runner, present(test_pid, crpcut::comm::exit_ok, phase, 0U,_))
        .IN_SEQUENCE(s);

      REQUIRE_CALL(runner, return_dir(1U));

      ALLOW_CALL(reg, crpcut_tag())
        .RETURN(std::ref(apa));

      REQUIRE_CALL(runner, present(test_pid, crpcut::comm::end_test, phase, _,_))
        .IN_SEQUENCE(s);

      reg.manage_death();
      ASSERT_TRUE(apa.num_failed() == 0U);
      ASSERT_TRUE(apa.num_passed() == 0U);
    }

    TEST(expected_signal_is_successful_if_si_code_is_CLD_DUMPED_if_dumps_are_ignored,
         fix<100U, crpcut::policies::core_dumps::crpcut_ignore>)
    {
      trompeloeil::sequence s;
      const pid_t test_pid = 5158;
      setup(test_pid);
      const crpcut::test_phase phase = crpcut::running;
      reg.set_phase(phase);
      SET_WD(1);

      prepare_siginfo(test_pid, 6, CLD_DUMPED, s);

      REQUIRE_CALL(runner, calc_cputime(_))
        .RETURN(1000000U);

      REQUIRE_CALL(reg, crpcut_is_expected_signal(6))
        .IN_SEQUENCE(s)
        .RETURN(true);

      REQUIRE_CALL(reg, crpcut_on_ok_action(_));
      REQUIRE_CALL(env, timeouts_enabled())
        .RETURN(false);

      REQUIRE_CALL(runner, present(test_pid, crpcut::comm::exit_ok, phase, 0U,_))
        .IN_SEQUENCE(s);

      REQUIRE_CALL(runner, return_dir(1U));

      ALLOW_CALL(reg, crpcut_tag())
        .RETURN(std::ref(apa));

      REQUIRE_CALL(runner, present(test_pid, crpcut::comm::end_test, phase, _,_))
        .IN_SEQUENCE(s);

      reg.manage_death();
      ASSERT_TRUE(apa.num_failed() == 0U);
      ASSERT_TRUE(apa.num_passed() == 0U);
    }

    TEST(expected_signal_reports_core_dump_by_default_if_si_code_is_CLD_DUMPED,
         fix<100U>)
    {
      trompeloeil::sequence s;
      const pid_t test_pid = 5158;
      setup(test_pid);
      const crpcut::test_phase phase = crpcut::running;
      reg.set_phase(phase);
      SET_WD(1);

      prepare_siginfo(test_pid, 6, CLD_DUMPED, s);

      REQUIRE_CALL(runner, calc_cputime(_))
        .RETURN(1000000U);

      REQUIRE_CALL(reg, get_location())
        .RETURN(fixed_string::make(loc_str));

      const char msg[] = "Died with core dump";
      char buffer[sizeof(msg) - 1 + loc_len  + sizeof(size_t)];
      make_message(buffer, msg);

      REQUIRE_CALL(runner, present(test_pid, crpcut::comm::exit_fail, phase, _,_))
        .WITH(memcmp(_5, buffer, _4) == 0)
        .IN_SEQUENCE(s);

      REQUIRE_CALL(runner, return_dir(1U));

      ALLOW_CALL(reg, crpcut_tag())
        .RETURN(std::ref(apa));

      REQUIRE_CALL(runner, present(test_pid, crpcut::comm::end_test, phase, _,_))
        .IN_SEQUENCE(s);

      reg.manage_death();
      ASSERT_TRUE(apa.num_failed() == 0U);
      ASSERT_TRUE(apa.num_passed() == 0U);
    }

    TEST(unknown_death_cause_gives_failed_test_with_code_number_in_msg,
         fix<100U>)
    {
      trompeloeil::sequence s;
      const pid_t test_pid = 5158;
      setup(test_pid);
      const crpcut::test_phase phase = crpcut::running;
      reg.set_phase(phase);
      SET_WD(1);

      prepare_siginfo(test_pid, 0, -1, s);

      REQUIRE_CALL(runner, calc_cputime(_))
        .RETURN(1000000U);

      REQUIRE_CALL(reg, get_location())
        .RETURN(fixed_string::make(loc_str));

      const char msg[] = "Died for unknown reason, code=-1";
      char buffer[sizeof(msg) - 1 + loc_len  + sizeof(size_t)];
      make_message(buffer, msg);

      REQUIRE_CALL(runner, present(test_pid,
                                   crpcut::comm::exit_fail,
                                   phase, _, _))
        .WITH(memcmp(_5, buffer, _4) == 0)
        .IN_SEQUENCE(s);


      REQUIRE_CALL(runner, return_dir(1U));

      ALLOW_CALL(reg, crpcut_tag())
        .RETURN(std::ref(apa));

      REQUIRE_CALL(runner, present(test_pid, crpcut::comm::end_test, phase, _,_))
        .IN_SEQUENCE(s);

      reg.manage_death();
      ASSERT_TRUE(apa.num_failed() == 0U);
      ASSERT_TRUE(apa.num_passed() == 0U);
    }

  }
}



