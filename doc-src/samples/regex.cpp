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

std::string a_string()
{
  return "192.168.0.1 is a valid IP address";
}

const char re[] = "^([0-9]{1,3}\\.){3}[0-9]{1,3}.*";

TEST(begins_with_digits_and_dots)
{
  const char num_dot_start[] = "^[0-9\\.]* ";
  ASSERT_TRUE(a_string() =~ crpcut::regex(num_dot_start));
}

TEST(begins_with_alpha)
{
  const char alpha_start[] = "^[[:alpha:]]* ";
  ASSERT_TRUE(a_string() =~ crpcut::regex(alpha_start));
}

TEST(fail_non_extended)
{
  ASSERT_PRED(crpcut::match<crpcut::regex>(re), a_string());
}

TEST(pass_extended)
{
  ASSERT_PRED(crpcut::match<crpcut::regex>(re, crpcut::regex::e), a_string());
}

TEST(exception_what_string_mismatch)
{
  ASSERT_THROW(std::string().at(3), std::exception, crpcut::regex(re));
}

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
