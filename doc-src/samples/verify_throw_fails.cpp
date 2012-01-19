/*
 * Copyright 2011-2012 Bjorn Fahller <bjorn@fahller.se>
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
#include <vector>

class int_matcher
{
  int n_, memory_;
 public:
  int_matcher(int n) : n_(n) {}
  bool operator()(int val) { memory_ = val; return val == n_; }
  friend std::ostream& operator<<(std::ostream &os, const int_matcher &m)
  {
    return os << m.memory_ << " does not match the expected " << m.n_;
  }
};

TEST(verify_throw_succeeds)
{
  std::vector<int> v;
  VERIFY_THROW(v.at(3), std::out_of_range);
  VERIFY_THROW(throw 3, int, int_matcher(3));
}

TEST(verify_throw_fails)
{
  std::vector<int> v;
  VERIFY_THROW(v.at(3), std::domain_error);
  VERIFY_THROW(v.at(3), std::bad_alloc);
  VERIFY_THROW(throw 3, int, int_matcher(5));
}

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
