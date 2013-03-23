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

#ifndef REGISTRATOR_LIST_HPP
#define REGISTRATOR_LIST_HPP

#include <crpcut.hpp>

namespace crpcut {


  class registrator_list
    : public crpcut::datatypes::list_elem<crpcut_test_case_registrator>
  {
  public:
    template <typename E>
    std::pair<unsigned, unsigned> filter_out_or_throw(const char *const *names,
                                                      std::ostream      &err_or,
                                                      E                  e);
    void list_tests_to(std::ostream &os, size_t tag_margin) const;
  private:
    virtual std::pair<unsigned, unsigned>
            filter_out_unused(const char *const *names,
                              std::ostream &err_os);
  };

  template <typename E>
  std::pair<unsigned, unsigned>
  registrator_list
  ::filter_out_or_throw(const char *const *names,
                        std::ostream      &err_os,
                        E                  e)
  {
    std::pair<unsigned, unsigned> rv = filter_out_unused(names, err_os);
    if (rv.first > rv.second) throw e;
    return rv;
  }
}

#endif // REGISTRATOR_LIST_HPP
