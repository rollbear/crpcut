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
#include "../current_process.hpp"
#define S(...) #__VA_ARGS__, (sizeof(#__VA_ARGS__) - 1)
TESTSUITE(test_wrapper)
{
  using namespace testing;
  class mock_reporter : public crpcut::comm::reporter
  {
  public:
    MOCK_CONST_METHOD3(report, void(crpcut::comm::type, const char*, size_t));
  };

  class mock_test : public crpcut::crpcut_test_case_base
  {
  public:
    MOCK_METHOD0(test, void());
    MOCK_CONST_METHOD0(crpcut_get_reg, crpcut::crpcut_test_case_registrator&());
    MOCK_METHOD0(crpcut_run_test, void());
  };

  class mock_registrator : public crpcut::crpcut_test_case_registrator
  {
  public:
    ~mock_registrator();
    void setup(crpcut::poll<crpcut::fdreader>&,pid_t,int,int,int) {};
    MOCK_CONST_METHOD0(get_location, crpcut::datatypes::fixed_string());
    MOCK_CONST_METHOD0(crpcut_tag, crpcut::tag&());
    MOCK_CONST_METHOD0(get_importance, crpcut::tag::importance());
    MOCK_METHOD0(run_test_case, void());
    MOCK_CONST_METHOD1(crpcut_is_expected_exit, bool(int));
    MOCK_CONST_METHOD1(crpcut_is_expected_signal, bool(int));
    MOCK_CONST_METHOD1(crpcut_on_ok_action, void(const char*));
    MOCK_CONST_METHOD1(crpcut_calc_deadline, unsigned long(unsigned long));
    MOCK_CONST_METHOD1(crpcut_expected_death, void(std::ostream&));
  };

  class mock_process : public crpcut::current_process
  {
  public:
	mock_process()
	: crpcut::current_process(crpcut::datatypes::fixed_string::make(""))
	{
	}
	MOCK_CONST_METHOD0(get_location, crpcut::datatypes::fixed_string());
	MOCK_CONST_METHOD0(is_naughty_child, bool());
	MOCK_CONST_METHOD0(freeze, void());
  };


  mock_registrator::~mock_registrator() {}
  struct fix
  {
	fix() : loc(crpcut::datatypes::fixed_string::make("apa:3")) {}
	crpcut::datatypes::fixed_string loc;
    StrictMock<mock_reporter>       reporter;
    StrictMock<mock_test>           test_case;
    StrictMock<mock_registrator>    registrator;
    StrictMock<mock_process>        process;
  };

  struct my_error {};
  TEST(void_wrapper_does_nothing_when_test_function_returns, fix)
  {
    EXPECT_CALL(test_case, test());
    crpcut::test_wrapper<void>::run(&test_case, reporter);
  }

  TEST(void_wrapper_lets_exceptions_through, fix)
  {
    EXPECT_CALL(test_case, test()).
      WillOnce(Throw(my_error()));
    typedef crpcut::test_wrapper<void> wrapper;
    ASSERT_THROW(wrapper::run(&test_case, reporter), my_error);
  }

  void expected_death_report(std::ostream &os)
  {
    os << "signal 9";
  }
  TEST(deaths_wrapper_reports_unexpected_return, fix)
  {
    EXPECT_CALL(test_case, test());
    EXPECT_CALL(test_case, crpcut_get_reg()).
      WillRepeatedly(ReturnRef(registrator));
    EXPECT_CALL(registrator, get_location()).
    		WillOnce(Return(loc));
    EXPECT_CALL(registrator, crpcut_expected_death(_)).
      WillOnce(Invoke(expected_death_report));
    EXPECT_CALL(reporter, report(crpcut::comm::exit_fail, _, _)).
      With(Args<1,2>(ElementsAreArray(
                     S(apa:3\nUnexpectedly survived\nExpected signal 9))));
    typedef crpcut::test_wrapper<crpcut::policies::deaths::wrapper> wrapper;
    wrapper::run(&test_case, reporter);
  }

  TEST(deaths_wrapper_does_nothing_on_exception, fix)
  {
    EXPECT_CALL(test_case, test()).
      WillOnce(Throw(my_error()));

    typedef crpcut::test_wrapper<crpcut::policies::deaths::wrapper> wrapper;

    ASSERT_THROW(wrapper::run(&test_case, reporter), my_error);
  }

  TEST(any_exception_wrapper_reports_when_test_returns, fix)
  {
    EXPECT_CALL(test_case, test());
    EXPECT_CALL(test_case, crpcut_get_reg()).
      WillRepeatedly(ReturnRef(registrator));
    EXPECT_CALL(registrator, get_location()).
      WillOnce(Return(loc));
    EXPECT_CALL(reporter, report(crpcut::comm::exit_fail, _, _)).
      With(Args<1,2>(ElementsAreArray(S(apa:3\nUnexpectedly did not throw))));
    typedef crpcut::test_wrapper<crpcut::policies::any_exception_wrapper> w;
    w::run(&test_case, reporter);
  }

  TEST(any_exception_wrapper_does_nothing_on_exception, fix)
  {
    EXPECT_CALL(test_case, test()).
      WillOnce(Throw(my_error()));
    typedef crpcut::test_wrapper<crpcut::policies::any_exception_wrapper> w;
    w::run(&test_case, reporter);
  }

  TEST(std_exception_wrapper_does_nothing_when_std_exception_thrown, fix)
  {
    EXPECT_CALL(test_case, test()).
      WillOnce(Throw(std::bad_alloc()));

    typedef crpcut::policies::exception_wrapper<std::exception> policy;
    typedef crpcut::test_wrapper<policy > w;
    w::run(&test_case, reporter);
  }

  TEST(std_exception_wrapper_reports_when_test_returns, fix)
  {
    EXPECT_CALL(test_case, test());
    EXPECT_CALL(test_case, crpcut_get_reg()).
      WillRepeatedly(ReturnRef(registrator));
    EXPECT_CALL(process, get_location()).
      WillOnce(Return(loc));
    reporter.set_process_control(&process);
    EXPECT_CALL(reporter, report(crpcut::comm::exit_fail,_,_)).
      With(Args<1,2>(ElementsAreArray(S(apa:3\nUnexpectedly did not throw))));

    typedef crpcut::policies::exception_wrapper<std::exception> policy;
    typedef crpcut::test_wrapper<policy > w;
    w::run(&test_case, reporter);
  }

  TEST(exception_wrapper_reports_when_test_returns, fix)
  {
    EXPECT_CALL(test_case, test());
    EXPECT_CALL(test_case, crpcut_get_reg()).
      WillRepeatedly(ReturnRef(registrator));
    EXPECT_CALL(process, get_location()).
      WillOnce(Return(loc));
    reporter.set_process_control(&process);
    EXPECT_CALL(reporter, report(crpcut::comm::exit_fail,_,_)).
        With(Args<1,2>(ElementsAreArray(S(apa:3\nUnexpectedly did not throw))));
    typedef crpcut::policies::exception_wrapper<my_error> policy;
    typedef crpcut::test_wrapper<policy> w;
    w::run(&test_case, reporter);
  }

  TEST(exception_wrapper_is_silent_when_expected_exception_is_thrown, fix)
  {
    EXPECT_CALL(test_case, test()).
        WillOnce(Throw(my_error()));
    typedef crpcut::policies::exception_wrapper<my_error> policy;
    typedef crpcut::test_wrapper<policy> w;
    w::run(&test_case, reporter);

  }
}
