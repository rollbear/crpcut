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
  template <size_t N>
  struct unstreamable
  {
    unstreamable()  { for (size_t i = 0; i < N; ++i) buff[i] = char(i);}
    char buff[N];
  };
  struct streamable
  {
    streamable(int i) : i_(i) {}
    int i_;
  };
  std::ostream &operator<<(std::ostream &os, const streamable &obj)
  {
    return os << "streamable(" << obj.i_ << ")";
  }
  struct specialized_unstreamable
  {
    specialized_unstreamable(int i) : i_(i) {}
    int i_;
  };

  struct specialized_streamable
  {
    specialized_streamable(int i) : i_(i) {}
    int i_;
  };
  std::ostream &operator<<(std::ostream &os, const specialized_streamable &obj)
  {
    return os << obj.i_;
  }
}

namespace crpcut {
  template <>
  void show_value(std::ostream &os, const specialized_unstreamable &obj)
  {
    os << "special(" << obj.i_ << ")";
  }

  template <>
  void show_value(std::ostream &os, const specialized_streamable &obj)
  {
    os << "specialized(" << obj.i_ << ")";
  }
}
TESTSUITE(show_value)
{

  TEST(a_small_unstreamable_object_is_shown_as_a_hex_dump)
  {
    std::ostringstream os;
    unstreamable<8> obj;
    crpcut::show_value(os, obj);
    ASSERT_TRUE(os.str() == "8-byte object <0001 0203  0405 0607>");
  }

  TEST(a_medium_sized_unstreamable_object_is_shown_as_a_grouped_hex_dump)
  {
    std::ostringstream os;
    unstreamable<16> obj;
    crpcut::show_value(os, obj);

    ASSERT_TRUE(os.str() ==
                "16-byte object <\n"
                "    0001 0203  0405 0607  0809 0a0b  0c0d 0e0f\n"
                "    >");
  }

  TEST(a_large_unstreamable_object_is_shown_as_multiline_hex_dump)
  {
    std::ostringstream os;
    unstreamable<40> obj;
    crpcut::show_value(os, obj);
    ASSERT_TRUE(os.str() ==
                "40-byte object <\n"
                "    0001 0203  0405 0607  0809 0a0b  0c0d 0e0f\n"
                "    1011 1213  1415 1617  1819 1a1b  1c1d 1e1f\n"
                "    2021 2223  2425 2627\n"
                "    >");

  }
  TEST(a_streamable_object_is_shown_using_its_output_stream_operator)
  {
    std::ostringstream os;
    streamable obj(100);
    crpcut::show_value(os, obj);
    ASSERT_TRUE(os.str() == "streamable(100)");
  }


  TEST(an_unstreamable_object_with_show_value_specialization_is_shown_using_it)
  {
    std::ostringstream os;
    specialized_unstreamable obj(100);
    crpcut::show_value(os, obj);
    ASSERT_TRUE(os.str() == "special(100)");
  }

  TEST(a_streamable_object_with_show_value_spelialization_is_shown_using_it)
  {
    std::ostringstream os;
    specialized_streamable obj(100);
    crpcut::show_value(os, obj);
    ASSERT_TRUE(os.str() == "specialized(100)");
  }
}





