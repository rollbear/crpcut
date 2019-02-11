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



#ifndef TAG_MOCKS_HPP
#define TAG_MOCKS_HPP

#include <trompeloeil.hpp>
#include <crpcut.hpp>

namespace mock
{
  class tag_list : public crpcut::tag_list_root
  {
  public:
    MAKE_MOCK0(fail, void());
    MAKE_MOCK0(pass, void());
    MAKE_CONST_MOCK0(num_failed, size_t());
    MAKE_CONST_MOCK0(num_passed, size_t());
    MAKE_CONST_MOCK0(get_name, crpcut::datatypes::fixed_string());
    MAKE_CONST_MOCK0(longest_tag_name, size_t());
    MAKE_MOCK1(set_importance, void(crpcut::tag::importance));
    MAKE_CONST_MOCK0(get_importance, crpcut::tag::importance());
  };

  class test_tag : public crpcut::tag
  {
  public:
    template <size_t N>
    test_tag(const char (&f)[N], tag_list *root)
      : crpcut::tag(N, root),
        name_{f},
        x(NAMED_REQUIRE_CALL(*this, get_name())
          .RETURN(std::ref(name_)))
    {
    }
    MAKE_MOCK0(fail, void());
    MAKE_MOCK0(pass, void());
    MAKE_CONST_MOCK0(num_failed, size_t());
    MAKE_CONST_MOCK0(num_passed, size_t());
    MAKE_CONST_MOCK0(get_name, crpcut::datatypes::fixed_string());
    MAKE_MOCK1(set_importance, void(crpcut::tag::importance));
    MAKE_CONST_MOCK0(get_importance, crpcut::tag::importance());
    crpcut::datatypes::fixed_string name_{};
  private:
    std::unique_ptr<trompeloeil::expectation> x;
  };

}






#endif // TAG_MOCKS_HPP
