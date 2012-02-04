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

namespace crpcut {


  int
  run(int argc, char *argv[], std::ostream &os)
  {
    return test_case_factory::run_test(argc, argv, os);
  }

  int
  run(int argc, const char *argv[], std::ostream &os)
  {
    return test_case_factory::run_test(argc, argv, os);
  }

  const char *
  get_parameter(const char *name)
  {
    return test_case_factory::get_parameter(name);
  }

  const char *get_start_dir()
  {
    return test_case_factory::get_start_dir();
  }

  void set_charset(const char *charset)
  {
    return test_case_factory::set_charset(charset);
  }

  bool timeouts_are_enabled()
  {
    return test_case_factory::timeouts_enabled();
  }

  bool tests_as_child_processes()
  {
    return test_case_factory::tests_as_child_procs();
  }

  void present_test_data(pid_t pid, comm::type t, test_phase phase,
                         size_t len, const char *buff)
  {
    test_case_factory::present(pid, t, phase, len, buff);
  }


} // namespace crpcut

