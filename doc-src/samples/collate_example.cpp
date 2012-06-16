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
#include "sorted-names.hpp"

#define aring "\xc3\xa5"
#define auml  "\xc3\xa4"
#define ouml  "\xc3\xb6"
#define Aring "\xc3\x85"
#define Auml  "\xc3\x84"
#define Ouml  "\xc3\x96"

template <const char *(&locname)>
class name_fixture
{
protected:
  name_fixture()
  {
    names.push(Auml "ngla");
    names.push(Ouml "rjan");
    names.push(Auml "rlig");
    names.push("Bj" ouml "rn");
  }
  sorted_names<locname> names;
};

template <const char *(&locname)>
class sort_checker
{
public:
  template <typename iter>
  static void verify(iter b, iter e)
  {
    iter i = b++;
    while (b != e)
      {
        INFO << *i << "<=" << *b;
        ASSERT_TRUE(crpcut::collate(*i, std::locale(locname)) <= *b);
        i = b++;
      }
  }
};

const char *sv_SE = "sv_SE.utf8";
const char *de_DE = "de_DE.utf8";


TEST(coll_equal, name_fixture<sv_SE>)
{
  sort_checker<sv_SE>::verify(names.begin(), names.end());
}

TEST(coll_mismatch, name_fixture<de_DE>)
{
  sort_checker<sv_SE>::verify(names.begin(), names.end());
}

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}


