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
#include "test_runner.hpp"

namespace crpcut {

  void hexdump(std::ostream &os, std::size_t bytes, const void *addr)
  {
    static const char lf[] = "\n    ";
    os << bytes << "-byte object <";
    if (bytes > 8) os << lf;
    const char *p = static_cast<const char *>(addr);
    char old_fill = os.fill();
    std::ios_base::fmtflags old_flags = os.flags();
    os   << std::setfill('0') ;
    size_t n = 0;
    for (; n < bytes; ++n)
      {
        os << std::hex << std::setw(2)
           << (static_cast<unsigned>(p[n]) & 0xff);
        if ((n & 15) == 15)
          {
            os << lf;
          }
        else if ((n & 3) == 3 && n != bytes - 1)
          {
            os << "  ";
          }
        else if ((n & 1) == 1 && n != bytes - 1)
          {
            os << ' ';
          }
      }
    if (bytes > 8 && (n & 15) != 0)
      {
        os << lf;
      }
    os.flags(old_flags);
    os.fill(old_fill);
    os  << '>';
  }

  int
  run(int argc, char *argv[], std::ostream &os)
  {
    return test_runner::run_test(argc, argv, os);
  }

  int
  run(int argc, const char *argv[], std::ostream &os)
  {
    return test_runner::run_test(argc, argv, os);
  }

  const char *
  get_parameter(const char *name)
  {
    return test_runner::obj().environment().get_parameter(name);
  }

  const char *get_start_dir()
  {
    return test_runner::obj().environment().get_start_dir();
  }

  void set_charset(const char *charset)
  {
    return test_runner::obj().environment().set_charset(charset);
  }

  bool timeouts_are_enabled()
  {
    return test_runner::obj().environment().timeouts_enabled();
  }

  unsigned timeout_multiplier()
  {
    return test_runner::obj().environment().timeout_multiplier();
  }

  bool tests_as_child_processes()
  {
    return test_runner::obj().environment().tests_as_child_procs();
  }

  void present_test_data(pid_t pid, comm::type t, test_phase phase,
                         size_t len, const char *buff)
  {
    test_runner::obj().present(pid, t, phase, len, buff);
  }

  bool is_backtrace_enabled()
  {
    return test_runner::obj().environment().is_backtrace_enabled();
  }

  const char *get_output_charset()
  {
    return test_runner::obj().environment().get_output_charset();
  }

  const char *get_illegal_char_representation()
  {
    return test_runner::obj().environment().get_illegal_rep();
  }

  const char *get_program_charset()
  {
    return test_runner::obj().environment().get_charset();
  }

  test_runner* default_test_runner()
  {
    return &test_runner::obj();
  }
} // namespace crpcut

