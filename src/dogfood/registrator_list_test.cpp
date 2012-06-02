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

#include <gmock/gmock.h>
#include <crpcut.hpp>
#include "../test_case_factory.hpp"
#include "../poll.hpp"

namespace {
  class mock_poll : public crpcut::poll<crpcut::fdreader>
  {
  public:
    MOCK_METHOD3(do_add_fd, void(int, crpcut::fdreader*, int));
    MOCK_METHOD1(do_del_fd, void(int));
    MOCK_METHOD1(do_wait, descriptor(int));
    MOCK_CONST_METHOD0(do_num_fds, std::size_t());
  };
}

TESTSUITE(registrator_list)
{
  typedef crpcut::registrator_list list;
  TEST(construction_does_nothing)
  {
    list l;
  }

  TEST(run_test_case_aborts,
       DEPENDS_ON(construction_does_nothing),
       EXPECT_SIGNAL_DEATH(SIGABRT),
       NO_CORE_FILE)
  {
    list l;
    l.run_test_case();
  }

  TEST(crpcut_tag_aborts,
       DEPENDS_ON(construction_does_nothing),
       EXPECT_SIGNAL_DEATH(SIGABRT),
       NO_CORE_FILE)
  {
    list l;
    l.crpcut_tag();
  }

  TEST(get_importance_aborts,
       DEPENDS_ON(construction_does_nothing),
       EXPECT_SIGNAL_DEATH(SIGABRT),
       NO_CORE_FILE)
  {
    list l;
    l.get_importance();
  }

  TEST(setup_aborts,
       DEPENDS_ON(construction_does_nothing),
       EXPECT_SIGNAL_DEATH(SIGABRT),
       NO_CORE_FILE)
  {
    list l;
    testing::StrictMock<mock_poll> poller;
    l.setup(poller, pid_t(1), 1,2,3,4);
  }
}







