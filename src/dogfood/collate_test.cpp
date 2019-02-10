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


#include <crpcut.hpp>

namespace {
  template <typename T>
  void check_result(const T& t, const char *msg = nullptr)
  {
    bool r = t;
    ASSERT_TRUE(r != bool(msg));
    std::ostringstream os;
    if (msg) {
      os << t;
      ASSERT_TRUE(os.str() == msg);
    }
  }
}


TESTSUITE(collate)
{
  static const std::locale c("C");

  TESTSUITE(equals)
  {
    TEST(identical_strings_are_true)
    {
      check_result(std::string("apa") == crpcut::collate("apa", c));
    }

    TEST(different_strings_are_false)
    {
      static const char result[] =
        "Failed in locale \"C\"\n"
        "  with left hand value = \"katt\"\n"
        "  and right hand value = \"apa\"";
      check_result(std::string("katt") == crpcut::collate("apa", c), result);
    }

    TEST(different_strings_left_collate_are_false)
    {
      static const char result[] =
        "Failed in locale \"C\"\n"
        "  with left hand value = \"apa\"\n"
        "  and right hand value = \"katt\"";
      check_result(crpcut::collate("apa", c) == std::string("katt"), result);
    }
  }

  TESTSUITE(not_equal)
  {
    TEST(identical_strings_are_false)
    {
      static const char result[] =
        "Failed in locale \"C\"\n"
        "  with left hand value = \"apa\"\n"
        "  and right hand value = \"apa\"";
      check_result(std::string("apa") != crpcut::collate("apa", c), result);
    }

    TEST(different_strings_are_true)
    {
      check_result(std::string("katt") != crpcut::collate("apa", c));
    }
  }

  TESTSUITE(greater_than)
  {
    TEST(identical_strings_are_false)
    {
      static const char result[] =
        "Failed in locale \"C\"\n"
        "  with left hand value = \"apa\"\n"
        "  and right hand value = \"apa\"";
      check_result(std::string("apa") > crpcut::collate("apa", c), result);
    }

    TEST(greater_with_lesser_is_true)
    {
      check_result(std::string("apan") > crpcut::collate("apa", c));
    }

    TEST(lesser_with_greater_is_false)
    {
      static const char result[] =
        "Failed in locale \"C\"\n"
        "  with left hand value = \"apa\"\n"
        "  and right hand value = \"apan\"";
      check_result(std::string("apa") > crpcut::collate("apan", c), result);
    }
  }

  TESTSUITE(greater_than_or_equal)
  {
    TEST(identical_strings_are_true)
    {
      check_result(std::string("apa") >= crpcut::collate("apa", c));
    }

    TEST(greater_with_lesser_is_true)
    {
      check_result(std::string("apan") >= crpcut::collate("apa", c));
    }

    TEST(lesser_with_greater_is_false)
    {
      static const char result[] =
        "Failed in locale \"C\"\n"
        "  with left hand value = \"apa\"\n"
        "  and right hand value = \"apan\"";
      check_result(std::string("apa") >= crpcut::collate("apan", c), result);
    }
  }

  TESTSUITE(less_than)
  {
    TEST(identical_strings_is_false)
    {
      static const char result[] =
        "Failed in locale \"C\"\n"
        "  with left hand value = \"apa\"\n"
        "  and right hand value = \"apa\"";
      check_result(std::string("apa") < crpcut::collate("apa", c), result);
    }

    TEST(greater_with_lesser_is_false)
    {
      static const char result[] =
        "Failed in locale \"C\"\n"
        "  with left hand value = \"apan\"\n"
        "  and right hand value = \"apa\"";
      check_result(std::string("apan") < crpcut::collate("apa", c), result);
    }

    TEST(lesser_with_greater_is_true)
    {
      check_result(std::string("apa") < crpcut::collate("apan", c));
    }
  }

  TESTSUITE(less_than_or_equal)
  {
    TEST(identical_strings_are_true)
    {
      check_result(std::string("apa") <= crpcut::collate("apa", c));
    }

    TEST(greater_with_lesser_is_false)
    {
      static const char result[] =
        "Failed in locale \"C\"\n"
        "  with left hand value = \"apan\"\n"
        "  and right hand value = \"apa\"";
      check_result(std::string("apan") <= crpcut::collate("apa", c), result);
    }

    TEST(lesser_with_greater_is_true)
    {
      check_result(std::string("apa") <= crpcut::collate("apan", c));
    }
  }
}
