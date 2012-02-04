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
#include "../../wrapped/posix_encapsulation.hpp"
extern "C" {
#include <sys/resource.h>
#include <sys/stat.h>
}

namespace {
  void wipe_dir(char *begin, char *end)
  {
    using namespace crpcut;
    assert(*end == 0);
    DIR *d = wrapped::opendir(begin);
    if (!d) return;
    char buff[sizeof(dirent) + PATH_MAX];
    dirent *ent = reinterpret_cast<dirent*>(buff);
    dirent *result = ent;
    *end = '/';
    while (result && (wrapped::readdir_r(d, ent, &result) == 0))
      {
        if (!result) break;
        if (wrapped::strcmp(ent->d_name, ".") == 0 ||
            wrapped::strcmp(ent->d_name, "..") == 0)
          {
            continue;
          }
        char *name_end = lib::strcpy(end + 1, ent->d_name);
        assert(*name_end == 0);
        wipe_dir(begin, name_end); // no-op if name is not a dir
        (void)wrapped::remove(begin);
      }
    *end = 0;
    (void)wrapped::closedir(d);
  }
}

namespace crpcut {
  namespace policies {
    namespace deaths {

      void
      wipe_working_dir
      ::crpcut_on_ok_action(const char *wd_name) const
      {
        if (tests_as_child_processes())
          {
            static char namebuff[PATH_MAX] = "";
            char *end = lib::strcpy(namebuff, wd_name);
            wipe_dir(namebuff, end);
          }
      }
    }
  }
}
