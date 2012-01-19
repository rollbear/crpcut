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

namespace crpcut {
  regex::regex(const regex& r)
    : p(r.p)
  {
  }

  std::ostream& operator<<(std::ostream &os, const regex &r)
  {
    return os << *r.p;
  }

  void regex::type::init(const char *s, int flags)
  {
    int i = wrapped::regcomp(&r, s, flags | REG_NOSUB);
    if (i != 0)
      {
        size_t n = wrapped::regerror(i, &r, 0, 0);
        errmsg = new char[n];
        wrapped::regerror(i, &r, errmsg, n);
      }
  }

  bool regex::type::match(const char *s)
  {
    if (errmsg) return false;
    int i = wrapped::regexec(&r, s, 0, 0, 0);
    if (i == 0) return true;
    if (i == REG_NOMATCH)
      {
        static const char tail[] = "\" does not match";
        std::size_t len = wrapped::strlen(s) + sizeof(tail) + 1;
        errmsg = new char[len];
        stream::oastream os(errmsg, len);
        os << "\"" << s << tail << '\0';
      }
    else
      {
        size_t n = wrapped::regerror(i, &r, 0, 0);
        errmsg = new char[n];
        wrapped::regerror(i, &r, errmsg, n);
      }
    return false;
  }

  regex::type::~type()
  {
    wrapped::regfree(&r);
    delete[] errmsg;
  }

  std::ostream& operator<<(std::ostream &os, const regex::type &obj)
  {
    if (obj.errmsg)
      return os << obj.errmsg;
    return os << "did not match";
  }

}
