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

#define S(...) #__VA_ARGS__, (sizeof(#__VA_ARGS__) - 1)
TESTSUITE(test_wrapper)
{
  class mock_reporter : public crpcut::comm::reporter
  {
  public:
    MAKE_CONST_MOCK5(report, void(crpcut::comm::type,
                                  const char*,
                                  size_t,
                                  std::string loc,
                                  const crpcut::crpcut_test_monitor*));
    void report(crpcut::comm::type t, const char *msg, size_t len,
                crpcut::datatypes::fixed_string location,
                const crpcut::crpcut_test_monitor* mon) const
    {
      report(t, msg, len, std::string(location.str, location.len), mon);
    }
  };

  class mock_test : public crpcut::crpcut_test_case_base
  {
  public:
    MAKE_MOCK0(test, void());
    MAKE_CONST_MOCK0(crpcut_get_reg, crpcut::crpcut_test_case_registrator&());
    MAKE_MOCK0(crpcut_run_test, void());
  };

  class mock_registrator : public crpcut::crpcut_test_case_registrator
  {
  public:
    ~mock_registrator();
    void setup(crpcut::poll<crpcut::fdreader>&,int,int,int) {};
    MAKE_CONST_MOCK0(get_location, crpcut::datatypes::fixed_string());
    MAKE_CONST_MOCK0(crpcut_tag, crpcut::tag&());
    MAKE_CONST_MOCK0(get_importance, crpcut::tag::importance());
    MAKE_MOCK0(run_test_case, void());
    MAKE_CONST_MOCK1(crpcut_is_expected_exit, bool(int));
    MAKE_CONST_MOCK1(crpcut_is_expected_signal, bool(int));
    MAKE_CONST_MOCK1(crpcut_on_ok_action, void(const char*));
    MAKE_CONST_MOCK1(crpcut_calc_deadline, unsigned long(unsigned long));
    MAKE_CONST_MOCK1(crpcut_expected_death, void(std::ostream&));
    MAKE_CONST_MOCK0(is_naughty_child, bool());
    MAKE_CONST_MOCK0(freeze, void());
  };


  mock_registrator::~mock_registrator() {}

  using trompeloeil::_;

  struct fix
  {
    fix() : loc(crpcut::datatypes::fixed_string::make("apa:3")),
            get_x(NAMED_ALLOW_CALL(test_case, crpcut_get_reg())
                  .RETURN(std::ref(registrator))),
            child_x(NAMED_ALLOW_CALL(registrator, is_naughty_child())
                    .RETURN(false))
    {
    }
    crpcut::datatypes::fixed_string           loc;
    mock_reporter                             reporter;
    mock_test                                 test_case;
    mock_registrator                          registrator;
    std::unique_ptr<trompeloeil::expectation> get_x;
    std::unique_ptr<trompeloeil::expectation> child_x;
  };

  struct my_error {};
  TEST(void_wrapper_does_nothing_when_test_function_returns, fix)
  {
    REQUIRE_CALL(test_case, test());
    crpcut::test_wrapper<void>::run(&test_case, reporter);
  }

  TEST(void_wrapper_lets_exceptions_through, fix)
  {
    REQUIRE_CALL(test_case, test())
      .THROW(my_error{});
    using wrapper = crpcut::test_wrapper<void>;
    ASSERT_THROW(wrapper::run(&test_case, reporter), my_error);
  }

  void expected_death_report(std::ostream &os)
  {
    os << "signal 9";
  }
  TEST(deaths_wrapper_reports_unexpected_return, fix)
  {
    REQUIRE_CALL(test_case, test());
    ALLOW_CALL(test_case, crpcut_get_reg())
      .RETURN(std::ref(registrator));
    REQUIRE_CALL(registrator, get_location())
      .RETURN(loc);
    REQUIRE_CALL(registrator, crpcut_expected_death(_))
      .SIDE_EFFECT(expected_death_report(_1));
    REQUIRE_CALL(reporter, report(crpcut::comm::exit_fail, _, _, "apa:3", &registrator))
      .WITH(std::string(_2,_3) == "Unexpectedly survived\nExpected signal 9");

    using wrapper = crpcut::test_wrapper<crpcut::policies::deaths::wrapper>;
    wrapper::run(&test_case, reporter);
  }

  TEST(deaths_wrapper_does_nothing_on_exception, fix)
  {
    REQUIRE_CALL(test_case, test())
      .THROW(my_error{});

    using wrapper = crpcut::test_wrapper<crpcut::policies::deaths::wrapper>;

    ASSERT_THROW(wrapper::run(&test_case, reporter), my_error);
  }

  TEST(any_exception_wrapper_reports_when_test_returns, fix)
  {
    REQUIRE_CALL(test_case, test());
    ALLOW_CALL(test_case, crpcut_get_reg())
      .RETURN(std::ref(registrator));
    REQUIRE_CALL(registrator, get_location())
      .RETURN(loc);
    REQUIRE_CALL(reporter, report(crpcut::comm::exit_fail, _, _, "apa:3", &registrator))
      .WITH(std::string(_2,_3) == "Unexpectedly did not throw");
    using w = crpcut::test_wrapper<crpcut::policies::any_exception_wrapper>;
    w::run(&test_case, reporter);
  }

  TEST(any_exception_wrapper_does_nothing_on_exception, fix)
  {
    REQUIRE_CALL(test_case, test())
      .THROW(my_error{});

    using w =  crpcut::test_wrapper<crpcut::policies::any_exception_wrapper>;
    w::run(&test_case, reporter);
  }

  TEST(std_exception_wrapper_does_nothing_when_std_exception_thrown, fix)
  {
    REQUIRE_CALL(test_case, test())
      .THROW(std::bad_alloc{});

    using policy = crpcut::policies::exception_wrapper<std::exception>;
    using w = crpcut::test_wrapper<policy >;
    w::run(&test_case, reporter);
  }

  TEST(std_exception_wrapper_reports_when_test_returns, fix)
  {
    REQUIRE_CALL(test_case, test());
    ALLOW_CALL(test_case, crpcut_get_reg())
      .RETURN(std::ref(registrator));
    REQUIRE_CALL(registrator, get_location())
      .RETURN(loc);
    REQUIRE_CALL(reporter, report(crpcut::comm::exit_fail,_,_, "apa:3", &registrator))
      .WITH(std::string(_2,_3) == "Unexpectedly did not throw");

    using policy = crpcut::policies::exception_wrapper<std::exception>;
    using w = crpcut::test_wrapper<policy >;
    w::run(&test_case, reporter);
  }

  TEST(exception_wrapper_reports_when_test_returns, fix)
  {
    REQUIRE_CALL(test_case, test());
    ALLOW_CALL(test_case, crpcut_get_reg())
      .RETURN(std::ref(registrator));
    REQUIRE_CALL(registrator, get_location())
      .RETURN(loc);
    REQUIRE_CALL(reporter, report(crpcut::comm::exit_fail,_,_,"apa:3", &registrator))
      .WITH(std::string(_2,_3) == "Unexpectedly did not throw");

    using policy = crpcut::policies::exception_wrapper<my_error> ;
    using w = crpcut::test_wrapper<policy>;
    w::run(&test_case, reporter);
  }

  TEST(exception_wrapper_is_silent_when_expected_exception_is_thrown, fix)
  {
    REQUIRE_CALL(test_case, test())
      .THROW(my_error{});

    using policy = crpcut::policies::exception_wrapper<my_error>;
    using w =  crpcut::test_wrapper<policy>;
    w::run(&test_case, reporter);

  }
}
