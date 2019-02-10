/*
 * Copyright 2009 Bjorn Fahller <bjorn@fahller.se>
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

class is_substring
{
public:
  template <typename T>
  class implementation
  {
  public:
    implementation(const T t) : needle(t) {}
    template <typename U>
    bool operator()(const U* haystack) const
    {
      return operator()(std::basic_string<U>(haystack));
    }
    template <typename U>
    bool operator()(const std::basic_string<U> &haystack) const
    {
      return haystack.find(needle) != std::basic_string<U>::npos;
    }
    friend std::ostream &operator<<(std::ostream &os, const implementation& i)
    {
      return os << "failed when searcing for " << i.needle;
    }
  private:
    T needle;
  };
};


namespace crpcut {

  template <typename T>
  struct match_traits<is_substring, const T*>
  {
    using type = is_substring::template implementation<const T*>;
  };
  template <typename T>
  struct match_traits<is_substring, T*>
  {
    using type = is_substring::template implementation<const T*>;
  };

  template <typename T>
  struct match_traits<is_substring, std::basic_string<T> >
  {
    using type = is_substring::template implementation<std::basic_string<T> >;
  };
}

TEST(using_char_array)
{
  char needle[]    = "steel";
  const char hay[] = "hay";
  char haystack[]  = "a mountain of hay";
  ASSERT_PRED(crpcut::match<is_substring>(hay), haystack);
  ASSERT_PRED(crpcut::match<is_substring>(needle), haystack);
}

TEST(using_const_char_ptr)
{
  const char *needle   = "steel";
  const char *hay      = "hay";
  const char *haystack = "a mountain of hay";
  ASSERT_PRED(crpcut::match<is_substring>(hay), haystack);
  ASSERT_PRED(crpcut::match<is_substring>(needle), haystack);
}

TEST(using_string)
{
  std::string needle("steel");
  const std::string hay("hay");
  std::string haystack("a mountain of hay");
  ASSERT_PRED(crpcut::match<is_substring>(hay), haystack);
  ASSERT_PRED(crpcut::match<is_substring>(needle), haystack);
}

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
