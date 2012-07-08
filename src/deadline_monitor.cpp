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


#include "deadline_monitor.hpp"
#include "clocks/clocks.hpp"
#include <algorithm>
namespace crpcut {

  deadline_monitor::deadline_monitor(void *space, std::size_t capacity)
    : buffer_vector<timeboxed*>(space, capacity)
  {
  }

  void
  deadline_monitor::insert(timeboxed *p)
  {
    assert(p->deadline_is_set());
    push_back(p);
    std::push_heap(begin(), end(), &timeboxed::compare);
  }

  void
  deadline_monitor::remove(timeboxed *p)
  {
    assert(p->deadline_is_set());
    timeboxed **found = std::find(begin(), end(), p);
    assert(found != end() && "clear deadline when none was ordered");

    size_t n = size_t(found - begin());

    for (;;)
      {
        size_t m = (n + 1) * 2 - 1;
        if (m >= size() - 1) break;

        if (timeboxed::compare(at(m + 1),
                                             at(m)))
          {
            at(n) = at(m);
          }
        else
          {
            at(n) = at(++m);
          }
        n = m;
      }

    at(n) = back();
    pop_back();
    if (n != size())
      {
        while (n && !timeboxed::compare(at(n),
                                                      at((n - 1) / 2)))
          {
            std::swap(at(n), at((n - 1) / 2));
            n = (n - 1) / 2;
          }
      }

  }

  timeboxed* deadline_monitor::remove_first()
  {
    assert(size());
    timeboxed *i = front();
    std::pop_heap(begin(), end(), &timeboxed::compare);
    pop_back();
    return i;
  }

  int
  deadline_monitor::ms_until_deadline(const clocks::monotonic &clock) const
  {
    if (size() == 0) return -1;
    int delta_us =  int(front()->absolute_deadline() - clock.now());
    return delta_us < 0 ? 0 : delta_us / 1000;
  }
}





