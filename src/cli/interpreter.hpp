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



#ifndef INTERPRETER_HPP_
#define INTERPRETER_HPP_

#include "activation_param.hpp"
#include "boolean_flip.hpp"
#include "named_param.hpp"
#include "value_param.hpp"

namespace crpcut {
  namespace cli {
    class interpreter
    {
    public:
      interpreter(const char *const *argv);
      const char *const *get_test_list() const;
#ifdef USE_BACKTRACE
      bool               backtrace_enabled() const;
#endif
      unsigned           num_parallel_tests() const;
      const char *       output_charset() const;
      const char *       working_dir() const;
      const char *       identity_string() const;
      const char *       illegal_representation() const;
      bool               list_tests() const;
      bool               list_tags() const;
      bool               honour_dependencies() const;
      const char *       report_file() const;
      const char *       named_parameter(const char *name);
      bool               quiet() const;
      bool               single_shot_mode() const;
      unsigned           timeout_multiplier() const;
      bool               honour_timeouts() const;
      const char *       tag_specification() const;
      bool               verbose_mode() const;
      bool               xml_output() const;
      const char *const *argv() const;
      const char *       program_name() const;
    private:
      void usage() const;
      const char*const *match_argv();

      param_list               list_;
# ifdef USE_BACKTRACE
      activation_param         backtrace_;
# endif
      value_param<unsigned>    num_children_;
      value_param<const char*> charset_;
      value_param<const char*> working_dir_;
      value_param<const char*> id_string_;
      value_param<const char*> illegal_rep_;
      activation_param         list_tests_;
      activation_param         list_tags_;
      activation_param         nodeps_;
      value_param<const char*> output_;
      named_param              param_;
      activation_param         quiet_;
      activation_param         single_shot_;
      value_param<unsigned>    timeout_multiplier_;
      activation_param         disable_timeouts_;
      value_param<const char*> tags_;
      activation_param         verbose_;
      activation_param         version_;
      boolean_flip             xml_;
      const char *const       *argv_;
      const char *const       *end_;
    };
  }
}






#endif // INTERPRETER_HPP_
