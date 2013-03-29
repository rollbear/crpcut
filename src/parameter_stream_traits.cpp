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

#include <crpcut.hpp>
#include "test_runner.hpp"
namespace {
  const char *get_parameter_string(const char *n,
                                   crpcut::crpcut_test_monitor *current_test)
  {
    using namespace crpcut;
    const char *p = get_parameter(n);
    if (!p)
      {
        std::ostringstream os;
        os << "No parameter named \"" << n << "\"";
        comm::report(comm::exit_fail, os, current_test->get_location());
      }
    return p;
  }
}

namespace crpcut {
  istream_wrapper
  parameter_stream_traits<std::istream>
  ::make_value(const char *n, crpcut_test_monitor *current_test)
  {
    return istream_wrapper(get_parameter_string(n, current_test));
  }

  stream::iastream
  parameter_stream_traits<relaxed<std::istream> >
  ::make_value(const char *n, crpcut_test_monitor *current_test)
  {
    return stream::iastream(get_parameter_string(n, current_test));
  }
}
