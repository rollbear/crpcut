/*
 * Copyright 2011 Bjorn Fahller <bjorn@fahller.se>
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

namespace crpcut {
    template <>
  void test_wrapper<void>::run(test_case_base *t)
  {
    t->test();
  }

  template <>
  void test_wrapper<policies::deaths::wrapper>::run(test_case_base *t)
  {
    t->test();
    stream::toastream<128> os;
    os << "Unexpectedly survived\nExpected ";
    t->crpcut_get_reg().crpcut_expected_death(os);
    comm::report(comm::exit_fail, os.begin(), os.size());
  }

  template <>
  void test_wrapper<policies::deaths::timeout_wrapper>::run(test_case_base *t)
  {
    t->test();
    if (!test_case_factory::timeouts_enabled()) return;
    stream::toastream<128> os;
    os << "Unexpectedly survived\nExpected ";
    t->crpcut_get_reg().crpcut_expected_death(os);
    comm::report(comm::exit_fail, os.begin(), os.size());
  }

  template <>
  void
  test_wrapper<policies::exception_wrapper<std::exception> >
  ::run(test_case_base* t)
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
      out << "Unexpectedly caught "
          << policies::crpcut_exception_translator::try_all();
      comm::report(comm::exit_fail, out);
    }
    comm::report(comm::exit_fail, "Unexpectedly did not throw");
  }

  template <>
  void
  test_wrapper<policies::any_exception_wrapper>::run(test_case_base *t)
  {
    try {
      t->test();
    }
    catch (...) {
      return;
    }
    comm::report(comm::exit_fail,
                 "Unexpectedly did not throw");
  }

}
