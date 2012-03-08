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

class unstreamable {
public:
  unstreamable(int i) : i_(i) {}
private:
  int i_;
};

class unstreamable_special {
public:
  unstreamable_special(int i) : i_(i) {}
private:
  int i_;
  friend void crpcut::show_value<>(std::ostream&, const unstreamable_special&);
};

class streamable_special {
public:
  streamable_special(int i) : i_(i) {}
private:
  int i_;
  friend std::ostream &operator<<(std::ostream &os,
                                  const streamable_special &obj)
  {
    return os << "stream(" << obj.i_ << ")";
  }
  friend void crpcut::show_value<>(std::ostream &, const streamable_special&);
};

namespace crpcut { // for specializations of crpcut::show_value<>()
  template <>
  void show_value(std::ostream &os, const unstreamable_special &obj)
  {
    os << "special(" << obj.i_ << ")";
  }
  template <>
  void show_value(std::ostream &os, const streamable_special &obj)
  {
    os << "streamable_special(" << obj.i_ << ")";
  }
}

TEST(show_it_all)
{
  int i(0);
  unstreamable u(1);
  unstreamable_special us(2);
  streamable_special ss(3);

  INFO << "i=" << i;   // default stream insertion
  INFO << "u=" << u;   // hex dump
  INFO << "us=" << us; // specialized
  INFO << "ss=" << ss; // specialized
  std::ostringstream os;
  os << ss;
  INFO << "ss from std::ostream = " << os.str(); // default stream insertion
}

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
