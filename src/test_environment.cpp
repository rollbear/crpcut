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

#include "test_environment.hpp"
#include "cli/interpreter.hpp"
#include "wrapped/posix_encapsulation.hpp"
namespace crpcut {

  const char *test_environment::default_charset = "UTF-8";
  test_environment
  ::test_environment(cli::interpreter *cli)
  : cli_(cli),
    charset_(default_charset)
  {
    wrapped::getcwd(homedir_, sizeof(homedir_));
  }

  test_environment
  ::~test_environment()
  {
  }



  const char *
  test_environment
  ::get_output_charset() const
  {
    return cli_->output_charset();
  }

  bool
  test_environment
  ::tests_as_child_procs() const
  {
    return !cli_->single_shot_mode();
  }

  bool
  test_environment
  ::timeouts_enabled() const
  {
    return cli_->honour_timeouts();
  }

  unsigned
  test_environment
  ::timeout_multiplier() const
  {
    return cli_->timeout_multiplier();
  }

  const char*
  test_environment
  ::get_start_dir() const
  {
    return homedir_;
  }

  const char*
  test_environment
  ::get_parameter(const char *name) const
  {
    return cli_->named_parameter(name);
  }

  const char*
  test_environment
  ::get_illegal_rep() const
  {
    return cli_->illegal_representation();
  }

  void
  test_environment
  ::set_charset(const char* set_name)
  {
    charset_ = set_name;
  }

  const char*
  test_environment
  ::get_charset() const
  {
    return charset_;
  }

  void
  test_environment
  ::set_default_charset(const char *charset)
  {
    default_charset = charset;
  }
}






