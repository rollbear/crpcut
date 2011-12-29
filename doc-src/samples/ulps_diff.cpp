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
#include <numeric>
#include <limits>

template <typename T>
class moving_avg
{
public:
  moving_avg() : avg(T()), n(T()) {}
  moving_avg& operator+=(T t) { ++n; avg-= (avg - t)/n; return *this; }
  operator T() const { return avg; }
private:
  T avg;
  T n;
};

static const unsigned count = 300;

TEST(too_narrow)
{
  moving_avg<float> mavg;
  float sum = 0.0;
  for (unsigned n = 0; n < count; ++n)
    {
      mavg+= 1.0f/3 + float(n);
      sum+= 1.0f/3 + float(n);
   }
  float avg = sum/count;
  ASSERT_PRED(crpcut::match<crpcut::ulps_diff>(2U), float(mavg), avg);
}

TEST(close_enough)
{
  moving_avg<float> mavg;
  float sum = 0.0;
  for (unsigned n = 0; n < count; ++n)
    {
      mavg+= 1.0f/3 + float(n);
      sum+= 1.0f/3 + float(n);
    }
  float avg = sum/count;
  ASSERT_PRED(crpcut::match<crpcut::ulps_diff>(10U), float(mavg), avg);
}

int main(int argc, char *argv[])
{
  return crpcut::run(argc, argv);
}
