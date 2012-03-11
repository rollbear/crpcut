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
#include <string>
#include <list>
#include <sstream>

class event_list
{
public:
  event_list() : num(0) {}
  void push(std::string text)
  {
    event_text.push_back(text);
  }
  std::string pop()
  {
    std::ostringstream os;
    os << num++ << " " << event_text.front();
    event_text.pop_front();
    return os.str();
  }
private:
  std::list<std::string> event_text;
  size_t num;
};

const char fmt[]
= "^(\\+)?[[:digit:]]+" "[[:space:]]+" "[[:alnum:]]([[:alnum:]|[:space:]])*$";

// i.e. LINE_FMT is a positive integer followed by at least one space and
// then a string of at least one alphanumerical character and spaces.

TEST(verify_output_format)
{
  event_list el;
  el.push("something happened");
  el.push("what else happened?");
  ASSERT_TRUE(el.pop() =~ crpcut::regex(fmt, crpcut::regex::e));
  ASSERT_TRUE(el.pop() =~ crpcut::regex(fmt, crpcut::regex::e));
}


int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
