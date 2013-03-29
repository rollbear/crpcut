/*
 * Copyright 2011-2013 Bjorn Fahller <bjorn@fahller.se>
 * All rights reserved

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

namespace {
  void
  report_unexpected_survive(crpcut::crpcut_test_case_base *t,
                            crpcut::comm::reporter        &report)
  {
    crpcut::heap::set_limit(crpcut::heap::system);
    crpcut::crpcut_test_case_registrator &current_test = t->crpcut_get_reg();
    std::ostringstream os;
    os << "Unexpectedly survived\nExpected ";
    current_test.crpcut_expected_death(os);
    report(crpcut::comm::exit_fail, os, current_test.get_location(), &current_test);
  }

  void
  report_did_not_throw(crpcut::crpcut_test_case_base *t,
                       crpcut::comm::reporter        &report)
  {
    crpcut::heap::set_limit(crpcut::heap::system);
    std::ostringstream os;
    crpcut::crpcut_test_monitor &current_test = t->crpcut_get_reg();
    os << "Unexpectedly did not throw";
    report(crpcut::comm::exit_fail, os, current_test.get_location(), &current_test);

  }
}
namespace crpcut {
  template <>
  void
  test_wrapper<void>
  ::run(crpcut_test_case_base *t, comm::reporter &)
  {
    t->test();
  }

  template <>
  void
  test_wrapper<policies::deaths::wrapper>
  ::run(crpcut_test_case_base *t, comm::reporter &report)
  {
    t->test();
    report_unexpected_survive(t, report);
  }

  template <>
  void
  test_wrapper<policies::deaths::timeout_wrapper>
  ::run(crpcut_test_case_base *t, comm::reporter &report)
  {
    t->test();
    if (!timeouts_are_enabled()) return;
    report_unexpected_survive(t, report);
  }

  template <>
  void
  test_wrapper<policies::exception_wrapper<std::exception> >
  ::run(crpcut_test_case_base* t, comm::reporter &report)
  {
    try {
      t->test();
    }
    catch (std::exception&) {
      return;
    }
    catch (...) {
      heap::set_limit(heap::system);
      std::ostringstream out;
      const crpcut_test_monitor &mon = t->crpcut_get_reg();
      out << "Unexpectedly caught "
          << policies::crpcut_exception_translator::try_all();
      report(comm::exit_fail, out, mon.get_location(), &mon);
    }
    report_did_not_throw(t, report);
  }

  template <>
  void
  test_wrapper<policies::any_exception_wrapper>
  ::run(crpcut_test_case_base *t, comm::reporter &report)
  {
    try {
      t->test();
    }
    catch (...) {
      return;
    }
    report_did_not_throw(t, report);
  }
}
