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

#include "symtable.hpp"
#include <crpcut.hpp>

class have_symtable
{
protected:
  symtable s;
};

class stdinsert : public have_symtable
{
protected:
  stdinsert()
  {
    s.add("one", 1);
    s.add("two", 2);
  }
};

class nullinsert : public have_symtable
{
protected:
  nullinsert()
  {
    s.add(0, 1);
  }
};

TEST(insert_and_lookup, stdinsert)
{
  ASSERT_EQ(s.lookup("one"), 1);
  ASSERT_EQ(s.lookup("two"), 2);
}

TEST(lookup_nonexisting, stdinsert, EXPECT_EXCEPTION(std::out_of_range))
{
  s.lookup("three");
}

TEST(add_null, nullinsert)
{
}

TEST(lookup_null, stdinsert, EXPECT_SIGNAL_DEATH(SIGABRT), NO_CORE_FILE)
{
  s.lookup(0);
}

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
