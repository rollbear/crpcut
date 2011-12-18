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
extern "C"
{
#include <sys/stat.h> // mkdir
#include <sys/types.h>
}

DEFINE_TEST_TAG(filesystem)

TESTSUITE(death)
{
  TESTSUITE(by_exit)
  {
    TEST(should_fail_with_exit_code_3)
    {
      exit(3);
    }

    TEST(should_succeed_with_exit_code_3, EXPECT_EXIT(3))
    {
      exit(3);
    }

    TEST(should_fail_with_no_exit, EXPECT_EXIT(3))
    {
    }

    TEST(should_fail_with_wrong_exit_code, EXPECT_EXIT(3))
    {
      exit(4);
    }

    TEST(should_succeed_with_wiped_working_dir,
         EXPECT_EXIT(3, WIPE_WORKING_DIR),
         WITH_TEST_TAG(filesystem))
    {
      {
        mkdir("katt", 0777);
        std::ofstream of("katt/apa");
        of << "lemur\n";
      }
      exit(3);
    }

    TEST(should_fail_wipe_with_left_behind_files_due_to_wrong_exit_code,
         EXPECT_EXIT(3, WIPE_WORKING_DIR),
         WITH_TEST_TAG(filesystem))
    {
      {
        mkdir("katt", 0777);
        std::ofstream of("katt/apa");
        of << "lemur\n";
      }
      exit(0);
    }

    TEST(should_fail_wipe_with_left_behind_files_due_to_signal_death,
         EXPECT_EXIT(3, WIPE_WORKING_DIR),
         WITH_TEST_TAG(filesystem))
    {
      {
        mkdir("katt", 0777);
        std::ofstream of("katt/apa");
        of << "lemur\n";
      }
      raise(9);
    }
  }
}
