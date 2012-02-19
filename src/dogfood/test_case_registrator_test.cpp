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

#include <gmock/gmock.h>
#include <crpcut.hpp>
#include "../test_case_factory.hpp"
#include "../process_control.hpp"
#include "../filesystem_operations.hpp"
#include "../poll.hpp"
#include <fstream>

extern "C"
{
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <fcntl.h>
}
TESTSUITE(test_case_registrator)
{
  using namespace testing;
  class mock_factory : public crpcut::test_case_factory
  {
  public:
    MOCK_METHOD0(tests_as_child_procs, bool());
    MOCK_METHOD0(timeouts_enabled, bool());
    MOCK_METHOD3(introduce_name, void(pid_t, const char*, size_t));
    MOCK_METHOD5(present, void(pid_t, crpcut::comm::type, crpcut::test_phase,
                               std::size_t, const char *));
    MOCK_METHOD1(set_deadline, void(crpcut::crpcut_test_case_registrator*));
    MOCK_METHOD1(clear_deadline, void(crpcut::crpcut_test_case_registrator*));
    MOCK_METHOD1(return_dir, void(unsigned));
    MOCK_METHOD1(test_succeeded, void(crpcut::crpcut_test_case_registrator*));
    MOCK_METHOD1(calc_cputime, unsigned long(const struct timeval&));
  };

  class mock_reporter : public crpcut::comm::reporter
  {
  public:
    mock_reporter(std::ostream &os) : reporter(os) {}
    MOCK_CONST_METHOD3(report, void(crpcut::comm::type, const char *, size_t));
  };

  class mock_process_control : public crpcut::process_control
  {
  public:
    MOCK_METHOD2(getrusage, int(int, struct rusage*));
    MOCK_METHOD2(killpg, int(int, int));
    MOCK_METHOD4(waitid, int(idtype_t, id_t, siginfo_t*, int));
  };

  class mock_fsops : public crpcut::filesystem_operations
  {
  public:
    MOCK_METHOD1(chdir, int(const char*));
    MOCK_METHOD2(mkdir, int(const char *n, mode_t m));
    MOCK_METHOD2(rename, int(const char *, const char*));
  };


  class mock_poll : public crpcut::poll<crpcut::fdreader>
  {
  public:
    MOCK_METHOD3(do_add_fd, void(int, crpcut::fdreader *, int));
    MOCK_METHOD1(do_del_fd, void(int));
    MOCK_METHOD1(do_wait, descriptor(int));
    MOCK_CONST_METHOD0(do_num_fds, std::size_t());
  };

  struct test_registrator : public crpcut::crpcut_test_case_registrator
  {
  public:
    test_registrator(const char                    *name,
                     const crpcut::namespace_info  &ns,
                     unsigned long                  cpulimit,
                     crpcut::comm::reporter        *reporter,
                     crpcut::process_control       *proc,
                     crpcut::filesystem_operations *fsops,
                     crpcut::test_case_factory     *factory)
    : crpcut_test_case_registrator(name, ns, cpulimit, reporter, proc, fsops, factory)
    {
    }
    using crpcut::crpcut_test_case_registrator::set_cputime_at_start;
    using crpcut::crpcut_test_case_registrator::prepare_construction;
    using crpcut::crpcut_test_case_registrator::manage_test_case_execution;
    using crpcut::crpcut_test_case_registrator::prepare_destruction;
    void setup(crpcut::poll<crpcut::fdreader> &,
               pid_t pid,
               int, int, int, int)
    {
      set_pid(pid);
    }
    MOCK_CONST_METHOD0(crpcut_tag, crpcut::tag&());
    MOCK_CONST_METHOD0(get_importance, crpcut::tag::importance());
    MOCK_METHOD0(run_test_case, void());
    MOCK_CONST_METHOD1(crpcut_is_expected_exit, bool(int));
    MOCK_CONST_METHOD1(crpcut_is_expected_signal, bool(int));
    MOCK_CONST_METHOD1(crpcut_on_ok_action, void(const char*));
    MOCK_CONST_METHOD1(crpcut_calc_deadline, unsigned long(unsigned long));
    MOCK_CONST_METHOD1(crpcut_expected_death, void(std::ostream&));
  };

  class mock_testcase : public crpcut::crpcut_test_case_base
  {
  public:
    MOCK_METHOD0(crpcut_run_test, void());
    MOCK_METHOD0(test, void());
    MOCK_CONST_METHOD0(crpcut_tag, crpcut::tag&());
    MOCK_CONST_METHOD0(crpcut_get_reg, crpcut::crpcut_test_case_registrator&());
  };

  template <unsigned long cpulimit>
  struct fix
  {
    fix()
    : default_out(),
      factory(),
      reporter(default_out),
      process_control(),
      fsops(),
      top_namespace(0,0),
      current_namespace("apa", &top_namespace),
      reg("test", current_namespace, cpulimit,
          &reporter, &process_control, &fsops, &factory)
    {
    }
    void setup(pid_t pid)
    {
      StrictMock<mock_poll> poller;
      EXPECT_CALL(factory, introduce_name(pid, StartsWith("apa::test"), 9));
      reg.setup(poller, pid, -1, -1, -1, -1);
    }
    void prepare_siginfo(pid_t pid, int status, int code, Sequence s)
    {
      siginfo_t info;
      info.si_pid    = pid;
      info.si_signo  = SIGCHLD;
      info.si_status = status;
      info.si_code   = code;
      EXPECT_CALL(process_control, waitid(P_PGID, id_t(pid), _, WEXITED)).
          InSequence(s).
          WillOnce(DoAll(SetArgumentPointee<2>(info),
                         Return(0)));
      EXPECT_CALL(process_control, waitid(P_PGID, id_t(pid), _, WEXITED)).
          InSequence(s).
          WillOnce(SetErrnoAndReturn(ECHILD, -1));
    }

    std::ostringstream                   default_out;
    StrictMock<mock_factory>             factory;
    mock_reporter                        reporter;
    StrictMock<mock_process_control>     process_control;
    StrictMock<mock_fsops>               fsops;
    crpcut::namespace_info               top_namespace;
    crpcut::namespace_info               current_namespace;
    test_registrator                     reg;
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
  EXPECT_CALL(fsops, mkdir(StrEq(#num), 0700)).  \
    WillOnce(Return(0));                         \
    reg.set_wd(num)

  TESTSUITE(set_wd)
  {
    TEST(creates_working_dir, fix<100U>)
    {
      EXPECT_CALL(fsops, mkdir(StrEq("100"), 0700)).
          WillOnce(Return(0));
      reg.set_wd(100);
    }

    TEST(accepts_already_existing_dir, fix<100U>)
    {
      EXPECT_CALL(fsops, mkdir(StrEq("100"), 0700)).
          WillOnce(SetErrnoAndReturn(EEXIST, -1));
      reg.set_wd(100);
    }

    TEST(aborts_on_failure, fix<100U>,
         EXPECT_SIGNAL_DEATH(SIGABRT),
         NO_CORE_FILE,
         DEPENDS_ON(accepts_already_existing_dir,
                    creates_working_dir))
    {
      EXPECT_CALL(fsops, mkdir(StrEq("100"), 0700)).
          WillOnce(SetErrnoAndReturn(EACCES, -1));
      reg.set_wd(100);
    }
  }

  TESTSUITE(goto_wd, DEPENDS_ON(ALL_TESTS(set_wd)))
  {
    TEST(changes_to_working_dir, fix<100U>)
    {
      EXPECT_CALL(fsops, mkdir(_,_)).WillOnce(Return(0));
      reg.set_wd(101);
      EXPECT_CALL(fsops, chdir(StrEq("101"))).
        WillOnce(Return(0));
      reg.goto_wd();
    }

    class my_exit {};

    TEST(reports_failure_to_enter, fix<100U>,
         DEPENDS_ON(changes_to_working_dir))
    {
      EXPECT_CALL(fsops, mkdir(_,_)).WillOnce(Return(0));
      EXPECT_CALL(fsops, chdir(StrEq("101"))).
        WillOnce(SetErrnoAndReturn(EACCES, -1));
      EXPECT_CALL(reporter, report(crpcut::comm::exit_fail,
                                   StrEq("Couldn't chdir working dir"),
                                   26)).
        WillOnce(Throw(my_exit()));
      reg.set_wd(101);
      ASSERT_THROW(reg.goto_wd(), my_exit);
    }

    TEST(returning_report_exit_fail_after_failure_to_enter_aborts, fix<100U>,
         DEPENDS_ON(reports_failure_to_enter),
         EXPECT_SIGNAL_DEATH(SIGABRT),
         NO_CORE_FILE)
    {
      EXPECT_CALL(fsops, mkdir(_,_)).WillOnce(Return(0));
      EXPECT_CALL(fsops, chdir(StrEq("101"))).
        WillOnce(SetErrnoAndReturn(EACCES, -1));
      EXPECT_CALL(reporter, report(crpcut::comm::exit_fail,
                                   StrEq("Couldn't chdir working dir"),
                                   26));
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
    TEST(reports_deadline_if_run_as_child_process, fix<10U>)
    {
      EXPECT_CALL(factory, tests_as_child_procs()).
          WillOnce(Return(true));

      unsigned long result = 100U;
      const void *addr = &result;
      const char *result_str = static_cast<const char*>(addr);
      EXPECT_CALL(reporter, report(crpcut::comm::set_timeout, _, sizeof(result))).
          With(Args<1,2>(ElementsAreArray(result_str, sizeof(result))));
      reg.prepare_construction(100U);
    }

    TEST(does_nothing_if_not_run_as_child_process, fix<10U>)
    {
      EXPECT_CALL(factory, tests_as_child_procs()).
          WillOnce(Return(false));
      reg.prepare_construction(100U);
    }
  }

  TESTSUITE(manage_test_case_execution)
  {
    TEST(reports_cputime_if_run_as_child_procs, fix<10U>)
    {

      EXPECT_CALL(factory, tests_as_child_procs()).
          WillRepeatedly(Return(true));

      static const struct timeval utime = { 1, 5 };
      static const struct timeval stime = { 3, 6 };
      Sequence s;
      struct rusage usage;
      usage.ru_utime = utime;
      usage.ru_stime = stime;
      EXPECT_CALL(process_control, getrusage(RUSAGE_SELF, _)).
          InSequence(s).
          WillOnce(DoAll(SetArgumentPointee<1>(usage),
                          Return(0)));

      struct timeval result = { 4, 11 };
      const void *addr = &result;
      const char *result_str = static_cast<const char*>(addr);

      EXPECT_CALL(reporter, report(crpcut::comm::begin_test, _, _)).
          With(Args<1,2>(ElementsAreArray(result_str, sizeof(result)))).
          InSequence(s);

      StrictMock<mock_testcase> test_obj;
      EXPECT_CALL(test_obj, crpcut_run_test()).
          InSequence(s);
      reg.manage_test_case_execution(&test_obj);
    }

    TEST(reports_success_if_not_run_as_child_procs, fix<10U>)
    {
      EXPECT_CALL(factory, tests_as_child_procs()).
          WillRepeatedly(Return(false));

      StrictMock<mock_testcase> test_obj;
      ASSERT_FALSE(reg.crpcut_failed());
      ASSERT_FALSE(reg.crpcut_succeeded());
      ASSERT_TRUE(reg.crpcut_can_run());
      EXPECT_CALL(test_obj, crpcut_run_test());
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

    TEST(expected_exit_without_prior_fail_reports_success, fix<10U>)
    {

      Sequence s;

      const pid_t test_pid = 101;
      setup(test_pid);
      SET_WD(9);
      reg.set_phase(crpcut::destroying);
      prepare_siginfo(test_pid, 0, CLD_EXITED, s);

      EXPECT_CALL(factory, calc_cputime(_)).
          WillOnce(Return(10));

      EXPECT_CALL(reg, crpcut_is_expected_exit(0)).
          InSequence(s).
          WillOnce(Return(true));

      EXPECT_CALL(reg, crpcut_on_ok_action(_));

      EXPECT_CALL(factory, present(test_pid, crpcut::comm::exit_ok,
                                   crpcut::destroying,
                                   0, _)).
          InSequence(s);

      EXPECT_CALL(factory, return_dir(9));

      EXPECT_CALL(factory, present(test_pid, crpcut::comm::end_test,
                                   crpcut::destroying,
                                   sizeof(bool), _)).
          InSequence(s);
      EXPECT_CALL(reg, crpcut_tag()).
          WillRepeatedly(ReturnRef(apa));

      EXPECT_CALL(factory, test_succeeded(&reg)).InSequence(s);
      reg.manage_death();
    }

    TEST(core_dump_gives_file_and_fail_report, fix<100U>)
    {
      ::mkdir("99", 0750);
      ::creat("99/junk_file", 0640);
      {
        Sequence s;
        const pid_t test_pid = 155;
        setup(test_pid);
        reg.set_phase(crpcut::running);
        SET_WD(99);
        prepare_siginfo(test_pid, 0, CLD_DUMPED, s);


        EXPECT_CALL(factory, calc_cputime(_)).
            WillOnce(Return(10));

        EXPECT_CALL(factory, present(test_pid,
                                     crpcut::comm::dir,
                                     crpcut::running,
                                     0, 0)).
           InSequence(s);
        EXPECT_CALL(fsops, rename(StrEq("99"), StrEq("apa::test")));
        EXPECT_CALL(factory, present(test_pid,
                                     crpcut::comm::exit_fail,
                                     crpcut::running,
                                     19, StartsWith("Died with core dump"))).
           InSequence(s);
        EXPECT_CALL(factory, return_dir(99));

        EXPECT_CALL(factory, present(test_pid, crpcut::comm::end_test,
                                     crpcut::running,
                                     sizeof(bool), _)).
           InSequence(s);
        EXPECT_CALL(reg, crpcut_tag()).
          WillRepeatedly(ReturnRef(apa));

        reg.manage_death();
        ASSERT_TRUE(apa.num_failed() == 1U);
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


#define S(...) #__VA_ARGS__, (sizeof(#__VA_ARGS__)-1)
    TEST(exit_with_wrong_code_gives_fail_report, fix<100U>)
    {
      Sequence s;
      const pid_t test_pid = 20123;
      setup(test_pid);
      const crpcut::test_phase phase = crpcut::destroying;
      reg.set_phase(phase);
      SET_WD(3);
      prepare_siginfo(test_pid, 3, CLD_EXITED, s);

      EXPECT_CALL(factory, calc_cputime(_)).
          WillOnce(Return(10));

      EXPECT_CALL(reg, crpcut_is_expected_exit(3)).
          InSequence(s).
          WillOnce(Return(false));

      EXPECT_CALL(reg, crpcut_expected_death(_)).
          InSequence(s).
          WillOnce(Invoke(set_expected_exit_msg<0>));

      EXPECT_CALL(factory, present(test_pid, crpcut::comm::exit_fail, phase, _,_)).
          With(Args<4,3>(ElementsAreArray(S(Exited with code 3\nExpected exit 0)))).
          InSequence(s);

      EXPECT_CALL(factory, return_dir(3));

      EXPECT_CALL(reg, crpcut_tag()).
          WillRepeatedly(ReturnRef(apa));

      EXPECT_CALL(factory, present(test_pid, crpcut::comm::end_test, phase, _,_)).
          InSequence(s);

      reg.manage_death();
      ASSERT_TRUE(apa.num_failed() == 1U);
      ASSERT_TRUE(apa.num_passed() == 0U);
    }

    TEST(exit_with_wrong_signal_gives_fail_report, fix<100U>)
    {
      Sequence s;
      const pid_t test_pid = 3216;
      setup(test_pid);
      const crpcut::test_phase phase = crpcut::running;
      reg.set_phase(phase);
      SET_WD(100);
      prepare_siginfo(test_pid, 15, CLD_KILLED, s);

      EXPECT_CALL(factory, calc_cputime(_)).
          WillOnce(Return(10));

      EXPECT_CALL(reg, crpcut_is_expected_signal(15)).
          InSequence(s).
          WillOnce(Return(false));

      EXPECT_CALL(reg, crpcut_expected_death(_)).
          InSequence(s).
          WillOnce(Invoke(set_expected_exit_msg<0>));

      EXPECT_CALL(factory, present(test_pid, crpcut::comm::exit_fail, phase, _,_)).
          With(Args<4,3>(ElementsAreArray(S(Died on signal 15\nExpected exit 0)))).
          InSequence(s);

      EXPECT_CALL(factory, return_dir(100));

      EXPECT_CALL(reg, crpcut_tag()).
          WillRepeatedly(ReturnRef(apa));

      EXPECT_CALL(factory, present(test_pid, crpcut::comm::end_test, phase, _,_)).
          InSequence(s);

      reg.manage_death();
      ASSERT_TRUE(apa.num_failed() == 1U);
      ASSERT_TRUE(apa.num_passed() == 0U);
    }
    TEST(exit_with_wrong_signal_gives_timeout_report_if_killed,
         fix<100U>)
    {
      Sequence s;
      const pid_t test_pid = 5158;
      setup(test_pid);
      const crpcut::test_phase phase = crpcut::running;
      reg.set_phase(phase);
      SET_WD(1);

      EXPECT_CALL(process_control, killpg(test_pid, 9)).
          InSequence(s).
          WillOnce(Return(0));

      prepare_siginfo(test_pid, 9, CLD_KILLED, s);

      EXPECT_CALL(factory, calc_cputime(_)).
          WillOnce(Return(1000));

      EXPECT_CALL(reg, crpcut_is_expected_signal(9)).
          InSequence(s).
          WillOnce(Return(false));

      EXPECT_CALL(reg, crpcut_expected_death(_)).
          InSequence(s).
          WillOnce(Invoke(set_expected_exit_msg<0>));

      EXPECT_CALL(factory, present(test_pid, crpcut::comm::exit_fail, phase, _,_)).
          With(Args<4,3>(ElementsAreArray(S(Timed out - killed\nExpected exit 0)))).
          InSequence(s);

      EXPECT_CALL(factory, return_dir(1));

      EXPECT_CALL(reg, crpcut_tag()).
          WillRepeatedly(ReturnRef(apa));

      EXPECT_CALL(factory, present(test_pid, crpcut::comm::end_test, phase, _,_)).
          InSequence(s);

      reg.kill();
      reg.manage_death();
      ASSERT_TRUE(apa.num_failed() == 1U);
      ASSERT_TRUE(apa.num_passed() == 0U);
    }

    TEST(expected_signal_gives_timeout_report_after_excessive_cputime,
         fix<100U>)
    {
      Sequence s;
      const pid_t test_pid = 5158;
      setup(test_pid);
      const crpcut::test_phase phase = crpcut::running;
      reg.set_phase(phase);
      SET_WD(1);

      prepare_siginfo(test_pid, 6, CLD_KILLED, s);

      EXPECT_CALL(factory, calc_cputime(_)).
          WillOnce(Return(1000));

      EXPECT_CALL(reg, crpcut_is_expected_signal(6)).
          InSequence(s).
          WillOnce(Return(true));

      EXPECT_CALL(factory, timeouts_enabled()).
          WillOnce(Return(true));

      EXPECT_CALL(factory, present(test_pid, crpcut::comm::exit_fail, phase, _,_)).
          With(Args<4,3>(ElementsAreArray(S(Test consumed 1000ms CPU-time\nLimit was 100ms)))).
          InSequence(s);

      EXPECT_CALL(factory, return_dir(1));

      EXPECT_CALL(reg, crpcut_tag()).
          WillRepeatedly(ReturnRef(apa));

      EXPECT_CALL(factory, present(test_pid, crpcut::comm::end_test, phase, _,_)).
          InSequence(s);

      reg.manage_death();
      ASSERT_TRUE(apa.num_failed() == 1U);
      ASSERT_TRUE(apa.num_passed() == 0U);
    }

    TEST(expected_signal_gives_no_timeout_report_when_timeouts_are_disabled,
         fix<100U>)
    {
      Sequence s;
      const pid_t test_pid = 5158;
      setup(test_pid);
      const crpcut::test_phase phase = crpcut::running;
      reg.set_phase(phase);
      SET_WD(1);

      prepare_siginfo(test_pid, 6, CLD_KILLED, s);

      EXPECT_CALL(factory, calc_cputime(_)).
          WillOnce(Return(1000));

      EXPECT_CALL(reg, crpcut_is_expected_signal(6)).
          InSequence(s).
          WillOnce(Return(true));

      EXPECT_CALL(reg, crpcut_on_ok_action(_));
      EXPECT_CALL(factory, timeouts_enabled()).
          WillOnce(Return(false));

      EXPECT_CALL(factory, present(test_pid, crpcut::comm::exit_ok, phase, 0,_)).
          InSequence(s);

      EXPECT_CALL(factory, return_dir(1));

      EXPECT_CALL(reg, crpcut_tag()).
          WillRepeatedly(ReturnRef(apa));

      EXPECT_CALL(factory, present(test_pid, crpcut::comm::end_test, phase, _,_)).
          InSequence(s);

      EXPECT_CALL(factory, test_succeeded(&reg));

      reg.manage_death();
      ASSERT_TRUE(apa.num_failed() == 0U);
      ASSERT_TRUE(apa.num_passed() == 1U);
    }

    TEST(unknown_death_cause_gives_failed_test_with_code_number_in_msg,
         fix<100U>)
    {
      Sequence s;
      const pid_t test_pid = 5158;
      setup(test_pid);
      const crpcut::test_phase phase = crpcut::running;
      reg.set_phase(phase);
      SET_WD(1);

      prepare_siginfo(test_pid, 0, -1, s);

      EXPECT_CALL(factory, calc_cputime(_)).
          WillOnce(Return(1000));

      EXPECT_CALL(factory, present(test_pid,
                                   crpcut::comm::exit_fail,
                                   phase, _, _)).
        With(Args<4,3>(ElementsAreArray(S(Died for unknown reason, code=-1)))).
        InSequence(s);


      EXPECT_CALL(factory, return_dir(1));

      EXPECT_CALL(reg, crpcut_tag()).
          WillRepeatedly(ReturnRef(apa));

      EXPECT_CALL(factory, present(test_pid, crpcut::comm::end_test, phase, _,_)).
          InSequence(s);

      reg.manage_death();
      ASSERT_TRUE(apa.num_failed() == 1U);
      ASSERT_TRUE(apa.num_passed() == 0U);
    }

  }
}



