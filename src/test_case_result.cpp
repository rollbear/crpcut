/*
 * Copyright 2011 Bjorn Fahller <bjorn@fahller.se>
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

#include "posix_encapsulation.hpp"
#include "test_case_result.hpp"
#include "fix_allocator.hpp"
namespace crpcut {
  typedef fix_allocator<test_case_result,
                        test_case_factory::max_parallel*3> allocator;

  test_case_result
  ::test_case_result(pid_t pid)
    :list_elem<test_case_result>(this),
     id(pid),
     explicit_fail(false),
     success(false),
     nonempty_dir(false),
     name(0),
     name_len(0),
     termination(0),
     term_len(0)
  {
  }

  test_case_result
  ::~test_case_result()
  {
    wrapped::free(termination);
    wrapped::free(name);
    while (!history.is_empty())
      {
        event *e = history.next();
        delete e;
      }
  }

  void *
  test_case_result
  ::operator new(size_t)
  {
    return allocator::alloc();
  }

  void
  test_case_result
  ::operator delete(void *p)
  {
    allocator::release(p);
  }

}
