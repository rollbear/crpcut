/*
 * Copyright 2009 Bjorn Fahller <bjorn@fahller.se>
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
#include <fstream>
#include <ostream>
extern "C" {
#include <signal.h> // raise
#include <sys/stat.h> // mkdir
#include <sys/types.h>
}

DEFINE_TEST_TAG(filesystem)

TESTSUITE(death)
{
  TESTSUITE(by_signal)
  {
    TEST(should_fail_with_left_behind_core_dump_due_to_death_on_signal_11,
         WITH_TEST_TAG(filesystem))
    {
      raise(11);
    }

    TEST(should_fail_without_core_dump_with_death_on_signal_11, NO_CORE_FILE)
    {
      raise(11);
    }

    TEST(should_succeed_with_death_on_signal_11,
         NO_CORE_FILE,
         EXPECT_SIGNAL_DEATH(11))
    {
      raise(11);
    }

    TEST(should_fail_with_wrong_signal, NO_CORE_FILE, EXPECT_SIGNAL_DEATH(11))
    {
      raise(6);
    }

    TEST(should_fail_with_normal_exit, EXPECT_SIGNAL_DEATH(11))
    {
    }

    TEST(should_succeed_with_wiped_working_dir,
         EXPECT_SIGNAL_DEATH(9, WIPE_WORKING_DIR), WITH_TEST_TAG(filesystem))
    {
      {
        mkdir("katt", 0777);
        std::ofstream of("katt/apa");
        of << "lemur\n";
      }
      raise(9);
    }

    TEST(should_fail_wipe_with_left_behind_files_due_to_wrong_signal,
         EXPECT_SIGNAL_DEATH(SIGABRT, WIPE_WORKING_DIR),
         WITH_TEST_TAG(filesystem))
    {
      {
        mkdir("katt", 0777);
        std::ofstream of("katt/apa");
        of << "lemur\n";
      }
      raise(9);
    }

    TEST(should_fail_wipe_with_left_behind_files_due_to_exit,
         EXPECT_SIGNAL_DEATH(SIGABRT, WIPE_WORKING_DIR),
         WITH_TEST_TAG(filesystem))
    {
      {
        mkdir("katt", 0777);
        std::ofstream of("katt/apa");
        of << "lemur\n";
      }
      exit(0);
    }
  }
}
