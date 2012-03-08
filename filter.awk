#!/usr/bin/awk -f

#  Copyright 2009 Bjorn Fahller <bjorn@fahller.se>
#  All rights reserved
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.

#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
#  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
#  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
#  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
#  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
#  SUCH DAMAGE.

BEGIN{
    succeeded=0;
}
/<blocked_tests>/ { counting_blocks=1; next }
/<\/blocked_tests/ { counting_blocks=0; next }
counting_blocks==1 && /<test name=.*\"\/>/  {
    if (nodeps && (match($0, "\"disabled\"") == 0)) {
        unexpected_blocked[unexpected_block_count++]=$0;
    }
    else
    {
        block_count++;
    }
    next;
}
/<test name=.*should_fail.*result=\"FAILED\">/,/<\/test>/ {
    if ($0 ~ /<\/test>/)
    {
        if (files_left)
        {
            if (testname !~ /left_behind/)
            {
                unexpected_files_behind[unexpected_files_behind_count++]=testname;
            }
        }
        else
        {
            if (testname ~ /left_behind/)
            {
                unexpected_clean[unexpected_clean_count++]=testname;
                if (testname ~ /core/) {
                    core_msg="Is your ulimit setting preventing core dumps?";
                }
            }
        }
        files_left=0;
        testname="";
        next;
    }
    if ($0 ~ /<test name/) {
        testname=$0;
        ++counted_failures;
        if ($1 ~ /should_not_run/ && !nodeps) {
            unexpected_run[unexpected_run_count++]=$0;
        }
        next;
    }
}
/<test name=.*should_fail.*result=\"PASSED\"/ {
    unexpected_success[failed_ok++]=$0; next;
}
/<test name=.*succ.*FAILED/ {
    unexpected_fail[ok_fail++]=$0; next;
}
/<test name=.*succ.*PASSED\"\/>/ {
    ++counted_succeeded;
    if (($0 ~ /should_not_run/) && !nodeps) {
        unexpected_run[unexpected_run_count++]=$0;
        echo "apa"
    }
    next;
}
/<test name=.*succ.*PASSED\">/,/<\/test>/ {
    if ($0 ~ /<\/test>/) {
        if (files_left) {
            unexpected_files_behind[unexpected_files_behind_count++]=testname;
        } else  {
            if (testname ~ /left_behind/) {
                unexpected_clean[unexpected_clean_count++]=testname;
                if (testname ~ /core/) {
                    core_msg="Is your ulimit setting preventing core dumps?";
                }
            }
        }
        files_left=0;
        testname="";
        next;
    }
    if ($0 ~ /<test name/) {
        testname=$0;
        ++counted_succeeded;
        if (($0 ~ /should_not_run/) && !nodeps) {
            echo "apa"
            unexpected_run[unexpected_run_count++]=$0;
        }
        next;
    }
}
/<registered_test_cases>/ {
    stat_registered=gensub(/<\/?[a-z_]*>/, "", "g") + 0; next;
}
/<run_test_cases>/ {
    stat_run=gensub(/<\/?[a-z_]*>/, "", "g") + 0; next;
}
/<failed_test_cases>/ {
    stat_failed=gensub(/<\/?[a-z_]*>/, "", "g") + 0; next;
}
/nonempty_dir/ {
    split($0, parts, /"/);
    dirname=parts[2];
    if (testname!="")
    {
        files_left=1
    }
}
END {
    report=0;
    if (registered != stat_registered) {
        printf("ERROR: Expected %d registered tests and found %d in stats\n",
               registered, stat_registered);
        report=1;
    }
    if (run != stat_run) {
        printf("ERROR: Expected %d run tests and found %d in stats\n",
               run, stat_run);
        report=1;
    }
    if (failed != stat_failed) {
        printf("ERROR: Expected %d failed and %d found in stats\n",
               failed, stat_failed);
        report=1
    }
    if (stat_failed != counted_failures) {
        printf("ERROR stats:%d failed and %d found in log\n",
               stat_failed, counted_failures);
        report=1;
    }
    if (rv != stat_failed) {
        printf("ERROR return value was %d end %d tests failed\n",
               rv, stat_failed);
        report=1
    }
    if (verbose && (stat_run - stat_failed != counted_succeeded)) {
        printf("ERROR stats:%d successful, and %d found in log\n",
               stat_run - stat_failed, counted_succeeded);
        report=1;
    }
    if (!verbose && counted_succeeded) {
        printf("ERROR: PASSED tests printed without verbose flag\n");
        report=1;
    }
    if (blocked != block_count)
    {
        printf("ERROR: Expected %d blocked tests and found %d\n",
               blocked, block_count)
        report=1;
    }
    if (failed_ok) {
        report=1;
        print "ERROR: The following tests shouldn't have succeeded:";
        for (a in unexpected_success)
        {
            reason=""
            line=unexpected_success[a]
            print line reason
        }
    }
    if (ok_failed) {
        report=1;
        print "ERROR: The following tests shouldn't have failed:"
        for (a in unexpected_fail)
        {
            print unexpected_fail[a];
        }
    }
    if (unexpected_run_count) {
        report=1;
        print "ERROR: The following tests shouldn't have run:"
        for (a in unexpected_run)
        {
            print unexpected_run[a];
        }
    }
    if (unexpected_blocked_count) {
        report=1;
        print "ERROR: The following tests shouldn't have been blocekd:"
        for (a in unexpecteded_block)
        {
            print unexpected_blocked[a]
        }
    }
    if (unexpected_files_behind_count) {
        report=1;
        print "ERROR: The following tests shouldn't have left files behind:"
        for (a in unexpected_files_behind)
        {
            print unexpected_files_behind[a];
        }
    }
    if (unexpected_clean_count) {
        report=1;
        print "ERROR: The following files should've left files behind:"
        for (a in unexpected_clean)
        {
            print unexpected_clean[a];
        }
        print "    " core_msg
    }
    if (report)
    {
        if (dirname != "")
        {
            print dirname " contains files left behind by test cases";
        }
    }
    else
    {
        if (dirname != "") {
            system("rm -r " dirname);
        }

    }
    exit(report);
}
