#!/usr/bin/ruby

#  Copyright 2009-2012 Bjorn Fahller <bjorn@fahller.se>
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

require "rexml/document"

class Test
  def initialize(result, phase)
    @result = result
    @phase  = phase
    @tag    = ''
    @log    = {}
    @files   = []
  end
  def result?()
    @result
  end
  def tag(t)
    @tag = t
    self
  end
  def expected_result?(re)
    @result =~ re
  end
  def matches_tag?(re)
    @tag =~ re
  end
  def log(type, re)
    if @log.has_key? type then
      @log[type].push re
    else
      @log[type] = [ re ]
    end
    self
  end
  def file(name)
    @files.push name
    self
  end
  def result_of(test)
    result = test.attributes['result']
    return "#{result} when #{@result} expected" if @result != result
    log = @log.clone
    dirname = nil
    log_index = {}
    test.elements.each('log/*') do |entry|
      text = entry.text || ""
      name = entry.name
      t = log[name]
      return "#{name} unexpected" if !t
      log_index[name] = 0 if !log_index[name]
      if name == 'violation' then
        actual_phase = entry.attributes['phase']
        if @phase != actual_phase then
          return "Expected phase=#{@phase} but found #{actual_phase}"
        end
        dirname = entry.attributes['nonempty_dir']
        if dirname then
          begin
            isdir = dirname && File.stat(dirname).directory?
          rescue
          end
          return "#{dirname} is not a directory" if !isdir
        end
      end
      re = t[log_index[name]]
      log_index[name] = log_index[name] + 1
      return "Too many #{name}'s" if !re
      return "#{text} doesn't match #{name} #{re}" if !re.match(text)
    end
    return "#{dirname} has unexpected files" if dirname && @files.empty?
    return "#{@files} is missing" if !dirname && !@files.empty?
    @files.each() do | name |
      path=dirname + '/' + name
      is_found=nil
      begin
        is_found = File.stat(path)
      rescue
      end
      return "#{name} is missing" if !is_found
      if is_found.directory? then
        Dir::rmdir(path)
      else
        File::unlink(path)
      end
    end
    begin
      dirname && Dir::rmdir(dirname)
    rescue
      return "working dir has unexpected files"
    end
    result
  end
end

class PassedTest < Test
  def initialize()
    super('PASSED', '')
  end
end

class FailedTest < Test
  def initialize(phase)
    super('FAILED', phase)
  end
end

def filter(names, tags, name, test)
  s = ''
  names.each do | n |
    s += '|' if s != ''
    s += n
 end
  name_re = Regexp.new("^(#{s})")
  s = ''
  tags.each do | t |
    s += '|' if s != ''
    s += t
  end
  tag_re = Regexp.new("^(#{s})$")
  name =~ name_re && (tags == [] || !test.matches_tag?(tag_re))
end

def concat(names)
  s = ''
  names.each do | n |
    s+= ' ' if s != ''
    s+= n
  end
  return s
end

def mk_tagflag(tags)
  s = ''
  tags.each do | t |
    s += "," if s != ''
    s += t
  end
  return "--tags=-#{s}" if tags != []
  return '                               '
end

A_H='asserts_and_depends\.cpp:\d+\s+'
P_H='parametrized\.cpp:\d+\s+'
PR_H='predicates\.cpp:\d+\s+'
RE_H='regex\.cpp:\d+\s+'
FP_H='fp\.cpp:\d+\s+'
A_T='Actual time to completion was'
S_E='std::exception\s+what\(\)'
R_E='std::range_error'
TESTS = {
  'asserts::should_fail_assert_exception_with_wrong_exception' =>
  FailedTest.new('running').
  log('violation',
      /caught std::exception\s+what\(\)=/me),

  'asserts::should_fail_assert_no_throw_with_std_exception_string_apa' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_NO_THROW\(throw #{R_E}\("apa"\)\)\s+caught #{S_E}=apa/me),

  'asserts::should_fail_assert_no_throw_with_unknown_exception' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_NO_THROW\(\s*throw\s+1\s*\)\s+caught\s+\.\.\./me),

  'asserts::should_fail_assert_throw_any_with_no_exception' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_THROW\(i=1, \.\.\.\)\s*Did not throw/me),

  'asserts::should_fail_assert_throw_with_no_exception' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_THROW\(i=1, std::exception\)\s+Did not throw/me),

  'asserts::should_fail_on_assert_eq_with_fixture' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_EQ\(num, 3\)\s+where\s+num\s*=\s*4/me),

  'asserts::should_fail_on_assert_false_with_fixture' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_FALSE\(num\)\n\s+is evaluated as:\n\s+3/me),

  'asserts::should_fail_on_assert_ge_with_fixture' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_GE\(num, 3\)\s+where\s+num\s*=\s*2/me),

  'asserts::should_fail_on_assert_gt_with_fixture' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_GT\(num, 3\)\s+where\s+num\s*=\s+3/me),

  'asserts::should_fail_on_assert_gt_with_unstreamable_param_i' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_GT\(i, num\)\s+where\s+i\s*=\s*\d+-byte object <[03 ]+>\s+num\s*=\s*3/me),

  'asserts::should_fail_on_assert_le_with_fixture' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_LE\(num, 3\)\s+where\s+num\s*=\s*4/me),

  'asserts::should_fail_on_assert_lt_with_fixture' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_LT\(num, 3\)\s+where\s+num\s*=\s*3/me),

  'asserts::should_fail_on_assert_ne_with_fixture' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_NE\(num, 3\)\s+where\s+num\s*=\s*3/me),

  'asserts::should_fail_on_assert_true_with_fixture' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_TRUE\(num\)\n\s+is evaluated as:\s+0/me),

  'asserts::should_succeed_assert_no_throw' =>
  PassedTest.new().
  log('stdout', /i=.*/),

  'asserts::should_succeed_assert_throw_with_correct_exception' =>
  PassedTest.new(),

  'asserts::should_succeed_on_assert_eq_with_fixture' =>
  PassedTest.new(),

  'asserts::should_succeed_on_assert_false_with_fixture' =>
  PassedTest.new(),

  'asserts::should_succeed_on_assert_ge_with_fixture' =>
  PassedTest.new(),

  'asserts::should_succeed_on_assert_gt_with_fixture' =>
  PassedTest.new(),

  'asserts::should_succeed_on_assert_le_with_fixture' =>
  PassedTest.new(),

  'asserts::should_succeed_on_assert_lt_with_fixture' =>
  PassedTest.new(),

  'asserts::should_succeed_on_assert_ne_with_fixture' =>
  PassedTest.new(),

  'asserts::should_succeed_on_assert_true_with_fixture' =>
  PassedTest.new(),

  'asserts::should_succeed_throw_any_with_int_exception' =>
  PassedTest.new(),

  'asserts::should_succeed_pointer_eq_0' =>
  PassedTest.new(),

  'asserts::should_succeed_0_eq_pointer' =>
  PassedTest.new(),

  'asserts::should_succeed_void_ptr_eq_ptr' =>
  PassedTest.new(),

  'asserts::should_succeed_ptr_eq_void_ptr' =>
  PassedTest.new(),

  'asserts::should_fail_pointer_eq_0' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_EQ\(pi, 0\)\n\s+where pi = (0x)?[[:xdigit:]]+$/me),

  'asserts::should_fail_0_eq_pointer' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_EQ\(0, pi\)\n\s+where pi = (0x)?[[:xdigit:]]+$/me),

  'asserts::should_fail_void_ptr_eq_ptr' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_EQ\(pv, pi\)\n\s+where pv = (0|\(nil\))\n\s+pi = (0x)?[[:xdigit:]]+$/me),

  'asserts::should_fail_ptr_eq_void_ptr' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_EQ\(pi, pv\)\n\s+where pi = (0x)?[[:xdigit:]]+\n\s+pv = (0|\(nil\))\s*$/me),

  'asserts::should_fail_eq_volatile' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_EQ\(n, m\)\n\s+where n = 3\n\s+m = 4\s*/me),

  'asserts::should_fail_false_volatile' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_FALSE\(n\)\n\s+is evaluated as:\n\s+3\s*/me),

  'asserts::should_fail_false_const_volatile' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_FALSE\(n\)\n\s+is evaluated as:\n\s+3\s*/me),

  'asserts::should_fail_true_volatile' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_TRUE\(n\)\n\s+is evaluated as:\n\s*0\s*/me),

  'asserts::should_fail_true_const_volatile' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_TRUE\(n\)\n\s+is evaluated as:\n\s*0\s*/me),

  'asserts::should_succeed_class_const_int_member' =>
  PassedTest.new(),

  'asserts::should_succeed_0_eq_pointer_to_member' =>
  PassedTest.new(),

  'asserts::should_fail_on_assert_true_with_small_unstreamable_param' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_TRUE\(i\)\n\s+is evaluated as:\n\s+.*<[0 ]*>\s*/me),

  'asserts::should_fail_on_assert_true_with_large_unstreamable_param' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_TRUE\(i\)\n\s+is evaluated as:\n\s+\?\s*/me),

  'asserts::expr::should_fail_on_assert_true_with_small_unstreamable_param' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_TRUE\(i - 4 < unstreamable<int>\(0\)\)\n\s+is evaluated as:\n\s+.*<[04 ]*> - 4 < .*<[0 ]*>\s*/me),

  'asserts::expr::should_fail_on_assert_true_with_large_unstreamable_param' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_TRUE\(i - 4 < unstreamable<long double>\(0\)\)\n\s+is evaluated as:\n\s+ \? - 4 < \?\s*/me),

  'asserts::expr::should_fail_on_assert_true_with_lt' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_TRUE\(n < num\)\n\s+is evaluated as:\n\s+4 < 3\s*/me),

  'asserts::expr::should_fail_on_assert_true_with_add_lt' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_TRUE\(n \+ num < 5\)\n\s+is evaluated as:\n\s+4 \+ 3 < 5\s*/me),

  'asserts::expr::should_fail_on_assert_true_with_sub_lt' =>
  FailedTest.new('running').
  log('violation',
      /#{A_H}ASSERT_TRUE\(n - num < 0\)\n\s+is evaluated as:\n\s+4 - 3 < 0\s*/me),

  'asserts::should_succeed_assert_throw_with_exact_string_match' =>
  PassedTest.new().
  tag('exception_content_match'),

  'asserts::should_fail_assert_throw_with_mismatching_string' =>
  FailedTest.new('running').
  tag('exception_content_match').
  log('violation',
      /#{A_H}ASSERT_THROW\(throw std::range_error\("apa"\), std::exception, "katt"\)\n\s*what\(\) == "apa" does not match string "katt"/me),

  'asserts::should_succeed_assert_throw_with_regexp_match' =>
  PassedTest.new().
  tag('exception_content_match'),

  'asserts::should_fail_assert_throw_with_mismatching_regexp' =>
  FailedTest.new('running').
  tag('exception_content_match').
  log('violation',
      /#{A_H}ASSERT_THROW\(.*\nregex\(\"\.\*x\.\*\"\) does not match what\(\) == \"en liten apa\"/me),

  'asserts::should_succeed_assert_throw_with_custom_matcher' =>
  PassedTest.new().
  tag('exception_content_match'),

  'asserts::should_fail_assert_throw_with_custom_matcher' =>
  FailedTest.new('running').
  tag('exception_content_match').
  log('violation',
      /#{A_H}ASSERT_THROW\(.*5 does not match the expected 3/me),

  'verify::should_succeed_verify_throw_with_correct_exception' =>
  PassedTest.new().
  log('info',
      /after/),

  'verify::should_fail_verify_exception_with_wrong_exception' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_THROW.*caught std::exception\s+what\(\)=/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_fail_verify_throw_with_no_exception' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_THROW.*Did not throw/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_fail_verify_throw_with_unexpected_c_string' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_THROW\([^\)]*\)\n\s+caught\s+\"apa\"/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_fail_verify_throw_with_translated_invalid_argument' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_THROW\(.*\"apa\"\).*\)\s+caught invalid_argument\s+what\(\)=apa/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_succeed_verify_no_throw' =>
  PassedTest.new().
  log('info', /after/),

  'verify::should_fail_verify_throw_any_with_no_exception' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_THROW.*Did not throw/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_succeed_throw_any_with_int_exception' =>
  PassedTest.new().
  log('info', /after/),

  'verify::should_fail_verify_no_throw_with_unknown_exception' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_NO_THROW.*caught\s+\.\.\./me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_fail_verify_no_throw_with_unexpected_c_string' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_NO_THROW.*caught \"apa\"/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_fail_verify_no_throw_with_translated_exception' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_NO_THROW\(.*\"apa\"\).*\).*caught invalid_argument\n\s+what\(\)=apa/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_fail_verify_no_throw_with_std_exception_string_apa' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_NO_THROW.*caught\s+#{S_E}=apa/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_succeed_on_verify_eq_with_fixture' =>
  PassedTest.new().
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_fail_on_verify_eq_with_fixture' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_EQ\(num, 3\)\s+where\s+num\s*=\s*4/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_succeed_on_verify_ne_with_fixture' =>
  PassedTest.new().
  log('info', /after/),

  'verify::should_fail_on_verify_ne_with_fixture' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_NE\(num, 3\)\s+where\s+num\s*=\s*3/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_succeed_on_verify_gt_with_fixture' =>
  PassedTest.new().
  log('info', /after/),

  'verify::should_fail_on_verify_gt_with_fixture' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_GT\(num, 3\)\s+where\s+num\s*=\s+3/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_succeed_on_verify_ge_with_fixture' =>
  PassedTest.new().
  log('info', /after/),

  'verify::should_fail_on_verify_ge_with_fixture' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_GE\(num, 3\)\s+where\s+num\s*=\s*2/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_succeed_on_verify_lt_with_fixture' =>
  PassedTest.new().
  log('info', /after/),

  'verify::should_fail_on_verify_lt_with_fixture' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_LT\(num, 3\)\s+where\s+num\s*=\s*3/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_succeed_on_verify_le_with_fixture' =>
  PassedTest.new().
  log('info', /after/),

  'verify::should_fail_on_verify_le_with_fixture' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_LE\(num, 3\)\s+where\s+num\s*=\s*4/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_succeed_on_verify_true_with_fixture' =>
  PassedTest.new().
  log('info', /after/),

  'verify::should_fail_on_verify_true_with_fixture' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_TRUE\(num\)\n\s+is evaluated as:\s+0/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_succeed_on_verify_false_with_fixture' =>
  PassedTest.new().
  log('info', /after/),

  'verify::should_fail_on_verify_false_with_fixture' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_FALSE\(num\)\n\s+is evaluated as:\n\s+3/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_fail_on_verify_gt_with_unstreamable_param_i' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_GT\(i, num\)\s+where\s+i\s*=\s*\d+-byte object <[03 ]+>\s+num\s*=\s*3/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_fail_on_verify_true_with_small_unstreamable_param' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_TRUE\(i\)\n\s+is evaluated as:\n\s+.*<[0 ]*>\s*/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_fail_on_verify_true_with_large_unstreamable_param' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_TRUE\(i\)\n\s+is evaluated as:\n\s+\?\s*/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_succeed_pointer_eq_0' =>
  PassedTest.new().
  log('info', /after/),

  'verify::should_succeed_0_eq_pointer' =>
  PassedTest.new().
  log('info', /after/),

  'verify::should_succeed_void_ptr_eq_ptr' =>
  PassedTest.new().
  log('info', /after/),

  'verify::should_succeed_ptr_eq_void_ptr' =>
  PassedTest.new().
  log('info', /after/),

  'verify::should_fail_pointer_eq_0' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_EQ\(pi, 0\)\n\s+where pi = (0x)?[[:xdigit:]]+$/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_fail_0_eq_pointer' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_EQ\(0, pi\)\n\s+where pi = (0x)?[[:xdigit:]]+$/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_fail_void_ptr_eq_ptr' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_EQ\(pv, pi\)\n\s+where pv = (0|\(nil\))\n\s+pi = (0x)?[[:xdigit:]]+$/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_fail_ptr_eq_void_ptr' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_EQ\(pi, pv\)\n\s+where pi = (0x)?[[:xdigit:]]+\n\s+pv = (0|\(nil\))\s*$/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_fail_eq_volatile' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_EQ\(n, m\)\n\s+where n = 3\n\s+m = 4\s*/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_fail_false_volatile' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_FALSE\(n\)\n\s+is evaluated as:\n\s+3\s*/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_fail_false_const_volatile' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_FALSE\(n\)\n\s+is evaluated as:\n\s+3\s*/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_fail_true_volatile' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_TRUE\(n\)\n\s+is evaluated as:\n\s*0\s*/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_fail_true_const_volatile' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_TRUE\(n\)\n\s+is evaluated as:\n\s*0\s*/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_succeed_class_const_int_member' =>
  PassedTest.new().
  log('info', /after/),

  'verify::should_succeed_0_eq_pointer_to_member' =>
  PassedTest.new().
  log('info', /after/),

  'verify::expr::should_fail_on_verify_true_with_small_unstreamable_param' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_TRUE\(i - 4 < unstreamable<int>\(0\)\)\n\s+is evaluated as:\n\s+.*<[04 ]*> - 4 < .*<[0 ]*>\s*/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::expr::should_fail_on_verify_true_with_large_unstreamable_param' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_TRUE\(i - 4 < unstreamable<long double>\(0\)\)\n\s+is evaluated as:\n\s+ \? - 4 < \?\s*/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::expr::should_fail_on_verify_true_with_lt' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_TRUE\(n < num\)\n\s+is evaluated as:\n\s+4 < 3\s*/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::expr::should_fail_on_verify_true_with_add_lt' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_TRUE\(n \+ num < 5\)\n\s+is evaluated as:\n\s+4 \+ 3 < 5\s*/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::expr::should_fail_on_verify_true_with_sub_lt' =>
  FailedTest.new('running').
  log('fail',
      /VERIFY_TRUE\(n - num < 0\)\n\s+is evaluated as:\n\s+4 - 3 < 0\s*/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_succeed_verify_throw_with_exact_string_match' =>
  PassedTest.new().
  tag('exception_content_match'),

  'verify::should_fail_verify_throw_with_mismatching_string' =>
  FailedTest.new('running').
  tag('exception_content_match').
  log('fail',
      /.*VERIFY_THROW\(throw std::range_error\("apa"\), std::exception, "katt"\)\n\s*what\(\) == "apa" does not match string "katt"/me).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_succeed_verify_throw_with_regexp_match' =>
  PassedTest.new().
  tag('exception_content_match'),

  'verify::should_fail_verify_throw_with_mismatching_regexp' =>
  FailedTest.new('running').
  tag('exception_content_match').
  log('fail',
      /.*VERIFY_THROW\(.*\n.*regex\(\"\.\*x\.\*\"\) does not match what\(\) == \"en liten apa\"/me).
  log('violation', /Earlier VERIFY failed/),

  'verify::should_succeed_verify_throw_with_custom_matcher' =>
  PassedTest.new().
  tag('exception_content_match'),

  'verify::should_fail_verify_throw_with_custom_matcher' =>
  FailedTest.new('running').
  tag('exception_content_match').
  log('fail',
      /.*VERIFY_THROW\(.*5 does not match the expected 3/me).
  log('violation', /Earlier VERIFY failed/),

  'death::by_exception::should_fail_any_exception' =>
  FailedTest.new('running').
  log('violation',
      /Unexpectedly did not throw/),

  'death::by_exception::should_fail_due_to_std_exception_with_string_apa' =>
  FailedTest.new('running').
  log('violation',
      /Unexpectedly caught #{S_E}=apa/me),

  'death::by_exception::should_fail_due_to_unknown_exception' =>
  FailedTest.new('running').
  log('violation',
      /Unexpectedly caught \.\.\./),

  'death::by_exception::should_fail_due_to_c_string_exception' =>
  FailedTest.new('running').
  log('violation',
      /Unexpectedly caught \"apa\"/),

  'death::by_exception::should_fail_with_no_exception' =>
  FailedTest.new('running').
  log('violation',
      /Unexpectedly did not throw/),

  'death::by_exception::should_fail_with_wrong_exception' =>
  FailedTest.new('running').
  log('violation',
      /Unexpectedly caught std::exception\n.*/),

  'death::by_exception::should_fail_with_c_string_exception' =>
  FailedTest.new('running').
  log('violation',
      /Unexpectedly caught \"apa\"/),

  'death::by_exception::should_fail_with_translated_exception' =>
  FailedTest.new('running').
  log('violation',
      /Unexpectedly caught invalid_argument\n\s+what\(\)=apa/),


  'death::by_exception::should_succed_with_any_exception' =>
  PassedTest.new(),

  'death::by_exception::should_succeed_with_range_error_thrown' =>
  PassedTest.new(),

  'death::by_exit::should_fail_with_exit_code_3' =>
  FailedTest.new('running').
  log('violation',
      /Exited with code 3\s+Expected normal exit/me),

  'death::by_exit::should_fail_with_no_exit' =>
  FailedTest.new('running').
  log('violation',
      /Unexpectedly survived\s+Expected exit with code 3/me),

  'death::by_exit::should_fail_with_wrong_exit_code' =>
  FailedTest.new('running').
  log('violation',
      /Exited with code 4\s+Expected exit with code 3/me),

  'death::by_exit::should_succeed_with_exit_code_3' =>
  PassedTest.new(),

  'death::by_exit::should_succeed_with_wiped_working_dir' =>
  PassedTest.new().
  tag('filesystem'),

  'death::by_exit::should_fail_wipe_with_left_behind_files_due_to_wrong_exit_code' =>
  FailedTest.new('running').
  tag('filesystem').
  file("katt/apa").
  file("katt").
  log('violation', /.*/me),

  'death::by_exit::should_fail_wipe_with_left_behind_files_due_to_signal_death' =>
  FailedTest.new('running').
  tag('filesystem').
  file("katt/apa").
  file("katt").
  log('violation', /.*/me),

  'death::by_signal::should_fail_with_left_behind_core_dump_due_to_death_on_signal_11' =>
  FailedTest.new('running').
  tag('filesystem').
  log('violation',
      /Died with core dump/).
  file('core'),

  'death::by_signal::should_fail_with_normal_exit' =>
  FailedTest.new('running').
  log('violation',
      /Unexpectedly survived\s+Expected signal 11/me),

  'death::by_signal::should_fail_with_wrong_signal' =>
  FailedTest.new('running').
  log('violation',
      /Died on signal 6\s+Expected signal 11/me),

  'death::by_signal::should_fail_without_core_dump_with_death_on_signal_11' =>
  FailedTest.new('running').
  log('violation',
      /Died on signal 11\s+Expected normal exit/me),

  'death::by_signal::should_succeed_with_death_on_signal_11' =>
  PassedTest.new(),

  'death::by_signal::should_succeed_with_wiped_working_dir' =>
  PassedTest.new().
  tag('filesystem'),

  'death::by_signal::should_fail_wipe_with_left_behind_files_due_to_wrong_signal' =>
  FailedTest.new('running').
  tag('filesystem').
  file("katt/apa").
  file("katt").
  log('violation', /.*/me),

  'death::by_signal::should_fail_wipe_with_left_behind_files_due_to_exit' =>
  FailedTest.new('running').
  tag('filesystem').
  file("katt/apa").
  file("katt").
  log('violation', /.*/me),

  'default_success' =>
  PassedTest.new(),

 'depends::should_not_run_due_to_one_failed_dependency_success_otherwise' =>
  PassedTest.new().
  tag('blocked'),

  'depends::should_succeed_after_success_dependencies' =>
  PassedTest.new(),

  'ext_parameters::should_succeed_expected_value' =>
  PassedTest.new().
  log('info',
      /katt/),

  'ext_parameters::should_succeed_no_value' =>
  PassedTest.new(),

  'ext_parameters::should_succeed_no_value_with_too_long_name' =>
  PassedTest.new(),

  'ext_parameters::should_succeed_value_interpret' =>
  PassedTest.new(),

  'ext_parameters::should_fail_value_interpret' =>
  FailedTest.new('running').
  log('violation',
      /Parameter apa with value "katt" cannot be interpreted/),

  'ext_parameters::should_fail_no_value_interpret' =>
  FailedTest.new('running').
  log('violation',
      /Parameter orm with no value cannot be interpreted/),

  'ext_parameters::should_fail_istream_nonexisting_parameter' =>
  FailedTest.new('running').
  tag('istream_ext_parameter').
  log('violation',
      /^No parameter named \"orm\"/),

  'ext_parameters::should_succeed_istream_string_value' =>
  PassedTest.new().
  tag('istream_ext_parameter'),

  'ext_parameters::should_fail_istream_parameter_with_wrong_type' =>
  FailedTest.new('running').
  tag('istream_ext_parameter').
  log('violation',
      /Extract value from stream failed/me),

  'ext_parameters::should_succeed_reading_multiple_values' =>
  PassedTest.new().
  tag('istream_ext_parameter'),

  'ext_parameters::should_fail_reading_too_many_values' =>
  FailedTest.new('running').
  tag('istream_ext_parameter').
  log('violation',
      /Extract value from stream failed/me),

  'ext_parameters::should_succeed_reading_stream_as_hex_value' =>
  PassedTest.new().
  tag('istream_ext_parameter'),

  'ext_parameters::should_succeed_reading_stream_as_octal_value' =>
  PassedTest.new().
  tag('istream_ext_parameter'),

  'ext_parameters::should_succeed_reading_stream_as_decimal_value' =>
  PassedTest.new().
  tag('istream_ext_parameter'),

  'ext_parameters::should_succeed_reading_stream_as_interpreted_base_value' =>
  PassedTest.new().
  tag('istream_ext_parameter'),

  'ext_parameters::should_fail_reading_relaxed_stream_value_of_wrong_type' =>
  FailedTest.new('running').
  tag('istream_ext_parameter').
  log('violation',
      /reading apa as int failed/),

  'ext_parameters::should_succeed_reading_relaxed_stream_value_of_right_type' =>
  PassedTest.new().
  tag('istream_ext_parameter'),

  'fp::abs::should_succeed_add_epsilon_float' =>
  PassedTest.new(),

  'fp::abs::should_succeed_sub_epsilon_float' =>
  PassedTest.new(),

  'fp::abs::should_fail_add_2epsilon_float' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(.*abs_diff.*param1 = \d+\.\d+.*Max allowed difference is.*Actual difference is.*/me),

  'fp::abs::should_fail_sub_2epsilon_float' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(.*abs_diff.*param1 = \d+\.\d+.*Max allowed difference is.*Actual difference is.*/me),

  'fp::abs::should_succeed_add_epsilon_double' =>
  PassedTest.new(),

  'fp::abs::should_succeed_sub_epsilon_double' =>
  PassedTest.new(),

  'fp::abs::should_fail_add_2epsilon_double' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(.*abs_diff.*param1 = \d+\.\d+.*Max allowed difference is.*Actual difference is.*/me),

  'fp::abs::should_fail_sub_2epsilon_double' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(.*abs_diff.*param1 = \d+\.\d+.*Max allowed difference is.*Actual difference is.*/me),


  'fp::abs::should_succeed_add_epsilon_long_double' =>
  PassedTest.new(),

  'fp::abs::should_succeed_sub_epsilon_long_double' =>
  PassedTest.new(),

  'fp::abs::should_fail_add_2epsilon_long_double' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(.*abs_diff.*param1 = \d+\.\d+.*Max allowed difference is.*Actual difference is/me),

  'fp::abs::should_fail_sub_2epsilon_long_double' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(.*abs_diff.*(\s+param[12] = \d+\.\d+){2}.*Max allowed difference is.*Actual difference is/me),

  'fp::relative::should_fail_relative_epsilon_float' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(.*relative_diff.*(\s+param[12] = \d+\.\d+){2}.*Max allowed relative difference is.*Actual relative difference is/me),

  'fp::relative::should_succeed_relative_epsilon_float' =>
  PassedTest.new(),

  'fp::relative::should_fail_relative_epsilon_double' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(.*relative_diff.*(\s+param[12] = \d+\.\d+){2}.*Max allowed relative difference is.*Actual relative difference is/me),

  'fp::relative::should_succeed_relative_epsilon_double' =>
  PassedTest.new(),

  'fp::relative::should_fail_relative_epsilon_long_double' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(.*relative_diff.*(\s+param[12] = \d+\.\d+){2}.*Max allowed relative difference is.*Actual relative difference is/me),

  'fp::relative::should_succeed_relative_epsilon_long_double' =>
  PassedTest.new(),

  'fp::ulps::using_float::should_succeed_equal_zeroes_0_ulps' =>
  PassedTest.new(),

  'fp::ulps::using_float::should_succeed_eps_diff_1_ulp' =>
  PassedTest.new(),

  'fp::ulps::using_float::should_fail_eps_diff_0_ulp' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(0U\), f1, f2\)\n\s+param1 = 1\n\s+param2 = 1/me),

  'fp::ulps::using_float::should_succeed_high_denorm_1_ulp' =>
  PassedTest.new(),

  'fp::ulps::using_float::should_fail_high_denorm_0_ulp' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(0U\), f1, f2\)\n\s+param1 = [0-9\.e+-]+\n\s+param2 = [0-9\.e+-]+/me),

  'fp::ulps::using_float::should_succeed_low_denorm_1_ulp' =>
  PassedTest.new(),

  'fp::ulps::using_float::should_fail_low_denorm_0_ulp' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(0U\), f1, f2\)\n\s+param1 = 0\n\s+param2 = [0-9\.e+-]+/me),

  'fp::ulps::using_float::should_succeed_pos_neg_denorm_min_2_ulps' =>
  PassedTest.new(),

  'fp::ulps::using_float::should_fail_pos_neg_denorm_min_1_ulp' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(1U\), f1, f2\)\n\s+param1 = [0-9\.e+-]+\n\s+param2 = -[0-9\.e+-]+/me),

  'fp::ulps::using_float::should_fail_nan' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(~unsigned\(\)\), f1, f2\)\n\s+param1 = -?[Nn][Aa][Nn]\n\s+param2 = 0/me),

  'fp::ulps::using_float::should_fail_max_inf_1_ulp' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(1U\), f1, f2\)\n\s+param1 = [Ii][Nn][Ff]\n\s+param2 = [0-9\.e+-]+/me),

  'fp::ulps::using_float::should_succeed_max_inf_1_ulp' =>
  PassedTest.new(),


  'fp::ulps::using_double::should_succeed_equal_zeroes_0_ulps' =>
  PassedTest.new(),

  'fp::ulps::using_double::should_succeed_eps_diff_1_ulp' =>
  PassedTest.new(),

  'fp::ulps::using_double::should_fail_eps_diff_0_ulp' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(0U\), f1, f2\)\n\s+param1 = 1\n\s+param2 = 1/me),

  'fp::ulps::using_double::should_succeed_high_denorm_1_ulp' =>
  PassedTest.new(),

  'fp::ulps::using_double::should_fail_high_denorm_0_ulp' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(0U\), f1, f2\)\n\s+param1 = [0-9\.e+-]+\n\s+param2 = [0-9\.e+-]+/me),

  'fp::ulps::using_double::should_succeed_low_denorm_1_ulp' =>
  PassedTest.new(),

  'fp::ulps::using_double::should_fail_low_denorm_0_ulp' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(0U\), f1, f2\)\n\s+param1 = 0\n\s+param2 = [0-9\.e+-]+/me),

  'fp::ulps::using_double::should_succeed_pos_neg_denorm_min_2_ulps' =>
  PassedTest.new(),

  'fp::ulps::using_double::should_fail_pos_neg_denorm_min_1_ulp' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(1U\), f1, f2\)\n\s+param1 = [0-9\.e+-]+\n\s+param2 = -[0-9\.e+-]+/me),

  'fp::ulps::using_double::should_fail_nan' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(~unsigned\(\)\), f1, f2\)\n\s+param1 = -?[Nn][Aa][Nn]\n\s+param2 = 0/me),

  'fp::ulps::using_double::should_fail_max_inf_1_ulp' =>
  FailedTest.new('running').
  log('violation',
      /#{FP_H}ASSERT_PRED\(crpcut::match<crpcut::ulps_diff>\(1U\), f1, f2\)\n\s+param1 = [Ii][Nn][Ff]\n\s+param2 = [0-9\.e+-]+/me),

  'fp::ulps::using_double::should_succeed_max_inf_1_ulp' =>
  PassedTest.new(),


  'parametrized::should_fail_assert_lt_char_array_string' =>
  FailedTest.new('running').
  log('violation',
      /#{P_H}ASSERT_LT\(p1, p2\)\s+where p1 = orm\s+p2 = katt/me),

  'parametrized::should_fail_assert_lt_int_char' =>
  FailedTest.new('running').
  log('violation',
      /#{P_H}ASSERT_LT\(p1, p2\)\s+where p1 = 800\s+p2 = A/me),

  'parametrized::should_fail_assert_lt_int_double' =>
  FailedTest.new('running').
  log('violation',
      /#{P_H}ASSERT_LT\(p1, p2\)\s+where p1 = 4\s+p2 = 3.14[12]\d*/me),

  'parametrized::should_succeed_assert_lt_char_array_string' =>
  PassedTest.new(),

  'parametrized::should_succeed_assert_lt_int_char' =>
  PassedTest.new(),

  'parametrized::should_succeed_assert_lt_int_double' =>
  PassedTest.new(),

  'predicates::should_succeed_simple_func' =>
  PassedTest.new(),

  'predicates::should_fail_simple_func' =>
  FailedTest.new('running').
  log('violation',
       /#{PR_H}ASSERT_PRED\(is_positive, v\)\s+param1 = -1/me),

  'predicates::should_succeed_simple_func_with_param_side_effect' =>
  PassedTest.new(),

  'predicates::should_fail_simple_func_with_param_side_effect' =>
  FailedTest.new('running').
  log('violation',
       /#{PR_H}ASSERT_PRED\(is_positive, --v\)\s+param1 = -1/me),

  'predicates::should_succeed_verify_simple_func' =>
  PassedTest.new().
  log('info', /after/),

  'predicates::should_fail_verify_simple_func' =>
  FailedTest.new('running').
  log('fail',
       /#{PR_H}VERIFY_PRED\(is_positive, v\)\s+param1 = -1/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'predicates::should_succeed_verify_simple_func_with_param_side_effect' =>
  PassedTest.new().
  log('info', /after/),

  'predicates::should_fail_verify_simple_func_with_param_side_effect' =>
  FailedTest.new('running').
  log('fail',
       /#{PR_H}VERIFY_PRED\(is_positive, --v\)\s+param1 = -1/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'predicates::should_succeed_func_wrap_class' =>
  PassedTest.new(),

  'predicates::should_fail_func_wrap_class' =>
  FailedTest.new('running').
  log('violation',
      /#{PR_H}ASSERT_PRED\(bifuncwrap.*less.*strcmp.*katt.*apa\"\)\s+param1 = katt\s+param2 = apa/me),

  'predicates::should_succeed_verify_func_wrap_class' =>
  PassedTest.new().
  log('info', /after/),

  'predicates::should_fail_verify_func_wrap_class' =>
  FailedTest.new('running').
  log('fail',
      /#{PR_H}VERIFY_PRED\(bifuncwrap.*less.*strcmp.*katt.*apa\"\)\s+param1 = katt\s+param2 = apa/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'predicates::should_succeed_streamable_pred' =>
  PassedTest.new(),

  'predicates::should_fail_streamable_pred' =>
  FailedTest.new('running').
  log('violation',
      /#{PR_H}ASSERT_PRED\(string_equal\(.*"katt"\)\s+param1 = katt\s+for string_equal.*\): compare.*equal to "apa"/me),

  'predicates::should_succeed_ptr_deref_eq' =>
  PassedTest.new(),

  'predicates::should_fail_ptr_deref_eq' =>
  FailedTest.new('running').
  log('violation',
      /#{PR_H}ASSERT_PRED.*pointing to:\s+4.*pointing to:\s+3/me),

  'predicates::should_succeed_verify_streamable_pred' =>
  PassedTest.new().
  log('info', /after/),

  'predicates::should_fail_verify_streamable_pred' =>
  FailedTest.new('running').
  log('fail',
      /#{PR_H}VERIFY_PRED\(string_equal\(.*"katt"\)\s+param1 = katt\s+for string_equal.*\): compare.*equal to "apa"/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'predicates::should_succeed_verify_ptr_deref_eq' =>
  PassedTest.new().
  log('info', /after/),

  'predicates::should_fail_verify_ptr_deref_eq' =>
  FailedTest.new('running').
  log('fail',
      /#{PR_H}VERIFY_PRED.*pointing to:\s+4.*pointing to:\s+3/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'predicates::match_operator::should_succeed_int_range_check' =>
  PassedTest.new(),
  
  'predicates::match_operator::should_fail_float_range_check' =>
  FailedTest.new('running').
  log('violation',
       /#{PR_H}ASSERT_TRUE\(.*\)\n\s+is evaluated as:\n\s+3.14\d* =~ in_range\(3.141\d+, 4\)/me),

  'collate::should_succeed_collation_string' =>
  PassedTest.new(),

  'collate::should_succeed_collation_char_array' =>
  PassedTest.new(),

  'collate::should_fail_collation_string' =>
  FailedTest.new('running').
  log('violation',
      /left hand value = \"app\"\n\s+.*right hand value = \"apa\"/me),

  'collate::should_fail_collation_char_array' =>
  FailedTest.new('running').
  log('violation',
      /left hand value = \"APP\"\n\s+.*right hand value = \"APA\"/me),

  'collate::should_succeed_equal_upcase' =>
  PassedTest.new(),

  'collate::should_fail_with_nonexisting_locale' =>
  FailedTest.new('running').
  log('violation',
      /ASSERT_.*caught\s+std::exception/me),

  'regex::should_succeed_simple_re' =>
  PassedTest.new(),

  'regex::should_fail_illegal_re' =>
  FailedTest.new('running').
  log('violation',
      /#{RE_H}ASSERT_PRED.*regex.*\)\s+param1 = apa.*\"\[a\"\):.*\n/me),

  'regex::should_fail_no_match' =>
  FailedTest.new('running').
  log('violation',
      /#{RE_H}ASSERT_PRED.*regex.*\)\n\s+param1 = katt.*\):.*regex\(\"apa\"\)/me),

  'regex::should_fail_case_mismatch' =>
  FailedTest.new('running').
  log('violation',
      /#{RE_H}ASSERT_PRED.*regex.*\)\n\s+param1 = APA.*\):.*regex\(\"apa\"\)/me),

  'regex::should_succeed_case_mismatch' =>
  PassedTest.new(),

  'regex::should_fail_ere_paren_on_non_e_re' =>
  FailedTest.new('running').
  log('violation',
      /#{RE_H}ASSERT_PRED.*\).*\)\s+param1 = apakattkattkatttupp.*regex\(\"apa\(katt\)\*tupp\"\)/me),

  'regex::should_succeed_ere_paren_on_e_re' =>
  PassedTest.new(),

  'regex::should_succeed_non_ere_paren_on_non_e_re' =>
  PassedTest.new(),

  'regex::should_fail_non_ere_paren_on_e_re' =>
  FailedTest.new('running').
  log('violation',
      /#{RE_H}ASSERT_PRED.*regex::e\), \".*\"\)\s+param1 = apakattkattkatttupp.*regex\(\"apa\\\(katt\\\)\*tupp\"\)/me),

  'regex::should_succeed_paren_litteral_e_re' =>
  PassedTest.new(),

  'regex::should_succeed_paren_litteral_non_e_re' =>
  PassedTest.new(),

  'regex::should_fail_ere_on_non_e_re' =>
  FailedTest.new('running').
  log('violation',
      /#{RE_H}ASSERT_PRED.*\"apa\+\"\), \"apaaa\"\)\n\s+param1 = apaaa.*regex\(\"apa\+\"\)/me),

  'regex::should_succeed_ere_on_e_re' =>
  PassedTest.new(),

  'should_fail_after_delay' =>
  FailedTest.new('running').
  tag('slow').
  log('violation',
      /Exited with code 1\s+Expected normal exit/me),

  'should_fail_due_to_left_behind_files' =>
  FailedTest.new('post_mortem').
  tag('slow').
  log('violation', /$/e).
  file("apa"),

  'should_succeed_reading_file_in_start_dir' =>
  PassedTest.new().
  tag('filesystem').
  log('info', /in.rdstate\(\)=\d-byte object <[ 0-9A-Fa-f]+>/),

 'should_not_run_due_to_failed_left_behind_files_success_otherwise' =>
  PassedTest.new().
  tag('blocked'),

  'output::should_fail_with_terminate' =>
  FailedTest.new('running').
  log('violation',
      /output.cpp:\d+\n\s*apa=(0[Xx])?1[fF]/),

  'output::should_succeed_with_info' =>
  PassedTest.new().
  log('info',
      /apa=3/),

  'output::should_fail_with_info' =>
  FailedTest.new('running').
  log('info',
      /apa=3/).
  log('violation',
      /Exited with code 1\s+Expected normal exit/me),

  'output::should_succeed_with_info_endl' =>
  PassedTest.new().
  log('info',/apa\nkatt/me),

  'output::should_fail_with_death_and_left_behind_core_dump' =>
  FailedTest.new('running').
  tag('slow').
  log('stderr',
      /output\.cpp:\d+.*[Aa]ssert/me).
  log('violation',
      /Died with core dump/).
  file('core'),

  'output::should_fail_with_death_due_to_assert_on_stderr' =>
  FailedTest.new('running').
  log('stderr',
      /output\.cpp:\d+.*[Aa]ssert/me).
  log('violation',
      /Died on signal \d+\s+Expected normal exit/me),

  'output::should_succeed_with_stderr' =>
  PassedTest.new().
  log('stderr', /hello/),

  'output::should_succeed_with_stdout' =>
  PassedTest.new().
  log('stdout', /hello/),

  'output::should_succeed_with_big_unstreamable_obj' =>
  PassedTest.new().
  log('info',
      /byte object <(\n[ a-fA-F0-9]+){2}\s*\n\s+>/me),

  'output::string_with_illegal_chars_should_succeed' =>
  PassedTest.new().
  log('info', /.*/me),
  
  'output::should_succeed_user_streamable_shown_as_defined' =>
  PassedTest.new().
  log('info', /user_streamable\(1\)/e),
  
  'output::should_succeed_multi_streamable_with_user_defined' =>
  PassedTest.new().
  log('info', /default operator => 1/).
  log('info', /user defined operator => multi_streamable\(1\)/e),

  'suite_deps::simple_all_ok::should_succeed' =>
  PassedTest.new(),

  'suite_deps::simple_all_ok::should_also_succeed' =>
  PassedTest.new(),

  'suite_deps::simple_all_fail::should_succeed' =>
  PassedTest.new(),

  'suite_deps::simple_all_fail::should_fail' =>
  FailedTest.new('running').
  log('violation',
      /ASSERT/),

  'suite_deps::should_succeed' =>
  PassedTest.new(),

  'suite_deps::should_not_run_success' =>
  PassedTest.new().
  tag('blocked'),

  'suite_deps::blocked_suite::cross_dep_violation_should_succeed' =>
  PassedTest.new(),

  'suite_deps::blocked_suite::should_not_run_success' =>
  PassedTest.new().
  tag('blocked'),

  'suite_deps::blocked_case::should_not_run_success' =>
  PassedTest.new().
  tag('blocked'),

  'suite_deps::blocked_case::nested_blocked::should_not_run_success' =>
  PassedTest.new().
  tag('blocked'),

  'suite_deps::should_run_remote_suite::should_succeed' =>
  PassedTest.new(),

  'suite_deps::blocked_remote_suite::should_not_run_success' =>
  PassedTest.new().
  tag('blocked'),

  'suite_deps::should_run_suite::should_succeed' =>
  PassedTest.new(),

  'suite_deps::should_run_case::should_succeed' =>
  PassedTest.new(),

  'suite_deps::should_run_case::nested_run::should_succeed' =>
  PassedTest.new(),

  'suite_deps::should_run_suite::should_also_succeed' =>
  PassedTest.new(),


  'timeouts::should_fail_slow_cputime_deadline' =>
  FailedTest.new('destroying').
  tag('slow').
  log('info', /.*/me).
  log('violation',
      /CPU-time timeout 100ms exceeded.\s+#{A_T} (([2-9]\d\d)|(1\d\d\d))ms/me),

  'timeouts::should_fail_slow_cputime_deadline_by_death' =>
  FailedTest.new('running').
  tag('slow').
  log('violation',
      /Died on signal \d+\s+Expected normal exit/me),

  'timeouts::should_fail_slow_realtime_deadline' =>
  FailedTest.new('destroying').
  tag('slow').
  log('violation',
      /Realtime timeout 100ms exceeded\.\s+#{A_T} [2-9]\d\dms/me),

  'timeouts::should_fail_slow_realtime_deadline_by_death' =>
  FailedTest.new('running').
  tag('slow').
  log('violation',
      /Timed out - killed/),

  'timeouts::should_succeed_slow_cputime_deadline' =>
  PassedTest.new(),

  'timeouts::should_succeed_slow_realtime_deadline' =>
  PassedTest.new(),

  'timeouts::should_fail_slow_save_from_stuck_constructor' =>
  FailedTest.new('creating').
  tag('slow').
  log('violation',
      /Timed out - killed/),

  'timeouts::should_fail_quick_save_from_stuck_constructor' =>
  FailedTest.new('creating').
  tag('slow').
  log('violation',
      /Timed out - killed/),

  'timeouts::should_fail_slow_save_from_stuck_destructor' =>
  FailedTest.new('destroying').
  tag('slow').
  log('violation',
      /Timed out - killed/),

  'timeouts::should_fail_quick_save_from_stuck_destructor' =>
  FailedTest.new('destroying').
  tag('slow').
  log('violation',
      /Timed out - killed/),

  'timeouts::expected::should_succeed_sleep' =>
  PassedTest.new().
  tag('slow'),

  'timeouts::expected::should_fail_early_return' =>
  FailedTest.new('running').
  log('violation',
      /Unexpectedly survived\nExpected 100ms realtime timeout/),

  'timeouts::expected::should_fail_cputime' =>
  FailedTest.new('running').
  tag('slow').
  log('violation',
      /Test consumed \d{2,3}ms CPU-time\nLimit was 3ms/),

  'timeouts::expected::should_succeed_cputime' =>
  PassedTest.new().
  tag('slow'),

  'timeouts::scoped::should_succeed_realtime_short_sleep' =>
  PassedTest.new(),

  'timeouts::scoped::should_fail_realtime_short_sleep' =>
  FailedTest.new('running').
  tag('slow').
  log('violation',
      /ASSERT_SCOPE_MAX_REALTIME_MS.*Actual time used was 2[0-5]ms/me),

  'timeouts::scoped::should_succeed_oversleep' =>
  PassedTest.new(),

  'timeouts::scoped::should_fail_cputime_long' =>
  FailedTest.new('running').
  tag('slow').
  log('violation',
      /ASSERT_SCOPE_MAX_CPUTIME_MS.*Actual time used was [1-5]\d\d\dms/me),


  'timeouts::scoped::should_succeed_verify_realtime_short_sleep' =>
  PassedTest.new().
  log('info', /after/),

  'timeouts::scoped::should_fail_verify_realtime_short_sleep' =>
  FailedTest.new('running').
  tag('slow').
  log('fail',
      /VERIFY_SCOPE_MAX_REALTIME_MS.*Actual time used was 2[0-5]ms/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'timeouts::scoped::should_succeed_verify_oversleep' =>
  PassedTest.new().
  log('info', /after/),

  'timeouts::scoped::should_fail_verify_cputime_long' =>
  FailedTest.new('running').
  tag('slow').
  log('fail',
      /VERIFY_SCOPE_MAX_CPUTIME_MS.*Actual time used was [1-5]\d\d\dms/me).
  log('info', /after/).
  log('violation', /Earlier VERIFY failed/),

  'very_slow_success' =>
  PassedTest.new().
  tag('slow'),

  'wrapped::should_succeed_in_range' =>
  PassedTest.new().
  log('info',
      /d=0.523\d+/),

  'wrapped::should_fail_assert_lt' =>
  FailedTest.new('running').
  log('violation',
      /wrapped\.cpp:\d+\s+ASSERT_LT\(d, 1\.\d*\)\s+where d = 1\.(1|0999).*/me),

  'heap::should_succeed_allocation_leak' =>
  PassedTest.new().
  log('info',
      /p1=.*/),

  'heap::should_fail_scope_leak_free' =>
  FailedTest.new('running').
  log('info',
      /p=.*/).
  log('violation',
      /1 object\n.*100 bytes at (0x)?[0-9A-Fa-f]+ allocated with malloc.*/me),

  'heap::should_succeed_scope_leak_free' =>
  PassedTest.new(),

  'heap::should_succeed_malloc_free_balance' =>
  PassedTest.new().
  log('info', /p1=.* p2=.*/),

  'heap::should_fail_verify_scope_leak_free' =>
  FailedTest.new('running').
  log('info', //).
  log('fail',
      /1 object\n.*100 bytes at (0x)?[0-9A-Fa-f]+ allocated with malloc/me).
  log('info', //).
  log('violation', /Earlier VERIFY failed/),

  'heap::should_succeed_verify_scope_leak_free' =>
  PassedTest.new().
  log('info', /after/),

  'heap::should_succeed_verify_malloc_free_balance' =>
  PassedTest.new().
  log('info', /p1=.* p2=.*/).
  log('info', /after/),

  'heap::should_succeed_empty_balance_fix' =>
  PassedTest.new(),

  'heap::should_succeed_malloc_balance_fix' =>
  PassedTest.new(),

  'heap::should_succeed_worlds_worst_strcpy' =>
  PassedTest.new(),

  'heap::should_succeed_malloc_blast_limit' =>
  PassedTest.new(),

  'heap::should_succeed_new_blast_limit' =>
  PassedTest.new(),

  'heap::should_succeed_new_array_blast_limit' =>
  PassedTest.new(),

  'heap::should_succeed_new_nothrow_blast_limit' =>
  PassedTest.new(),

  'heap::should_succeed_new_array_nothrow_blast_limit' =>
  PassedTest.new(),

  'heap::should_succeed_blast_limit_with_string' =>
  PassedTest.new(),

  'heap::should_fail_limit_too_low' =>
  FailedTest.new('running').
  log('violation',
      /heap::set_limit.*is below current/),

  'heap::should_fail_cross_malloc_delete' =>
  FailedTest.new('running').
  log('violation',
      /DEALLOC FAIL\ndelete.*using malloc/me),

  'heap::should_fail_cross_malloc_delete_array' =>
  FailedTest.new('running').
  log('violation',
      /DEALLOC FAIL\ndelete\[\].*using malloc/me),

  'heap::should_fail_cross_new_free' =>
  FailedTest.new('running').
  log('violation',
      /DEALLOC FAIL\nfree.*using new/me),

  'heap::should_fail_cross_new_delete_array' =>
  FailedTest.new('running').
  log('violation',
      /DEALLOC FAIL\ndelete\[\].*using new/me),

  'heap::should_fail_cross_new_array_free' =>
  FailedTest.new('running').
  log('violation',
      /DEALLOC FAIL\nfree.*using new\[\]/me),

  'heap::should_fail_cross_new_array_delete' =>
  FailedTest.new('running').
  log('violation',
      /DEALLOC FAIL\ndelete.*using new\[\]/me),

  'heap::should_succeed_new_handler' =>
  PassedTest.new(),

  'heap::should_succeed_new_handler_no_ballast' =>
  PassedTest.new(),

  'heap::should_succeed_nothrow_new_handler' =>
  PassedTest.new(),

  'heap::should_succeed_nothrow_new_handler_no_ballast' =>
  PassedTest.new(),

  'bad_forks::fork_and_let_child_hang_should_fail' =>
  FailedTest.new('running').
  tag('slow').
  log('violation',
      /Timed out - killed/),

  'bad_forks::fork_and_let_child_run_test_code_should_fail' =>
  FailedTest.new('child').
  log('violation',
      /I am child/)
}

GMOCK_TESTS = {
  'google_mock::basic_success' =>
  PassedTest.new(),

  'google_mock::should_fail_by_calling_with_wrong_value' =>
  FailedTest.new('running').
  log('violation',
      /mock function call.*call: func\(4\).*equal to 3\s+Actual: 4/me),

  'google_mock::should_fail_by_calling_too_often' =>
  FailedTest.new('running').
  log('violation',
      /more times than expected.*func\(3\)/me),

  'google_mock::should_fail_by_not_calling' =>
  FailedTest.new('destroying').
  log('violation',
      /call count doesn't match.*Actual: never called/me),

  'google_mock::sequence_success_1' =>
  PassedTest.new(),

  'google_mock::sequence_success_2' =>
  PassedTest.new(),

  'google_mock::sequence_should_fail_incomplete' =>
  FailedTest.new('destroying').
  log('violation',
      /call count doesn't match.*Actual: never called/me),

  'google_mock::sequence_should_fail_one_too_many' =>
  FailedTest.new('running').
  log('violation',
      /called more times than expected.*Actual: called twice/me),

  'google_mock::sequence_should_fail_one_wrong_value' =>
  FailedTest.new('running').
  log('violation',
      /Unexpected mock function call.*call: func\(4\).*none matched:/me),

  'google_mock::success_with_unstreamable_type' =>
  PassedTest.new(),

  'google_mock::should_fail_with_unstreamable_type_wrong_value' =>
  FailedTest.new('running').
  log('violation',
      /Unexpected mock.*Expected.*equal.*3.*Actual.*4.*>/me)
}


def check_run(command, tests)
  file = open("|#{command}")
  s = file.read
  file.close
  doc = REXML::Document.new s
  rc = $?.exitstatus
  unexpected = []
  wrong_result = []
  expected_failed = 0
  expected_passed = 0
  failed = 0
  passed = 0
  doc.elements.each('crpcut/test') do |e|
    name = e.attributes['name']
    result = e.attributes['result']
    failed += 1 if result == 'FAILED'
    passed += 1 if result == 'PASSED'
    t = tests[name]
    if !t then
      unexpected+= [name]
    else
      r = t.result_of(e)
      wrong_result += [ name, r ] if r != result
      expected_failed += 1 if r == 'FAILED'
      expected_passed += 1 if r == 'PASSED'
      tests.delete(name)
    end
  end
  stats = doc.elements['crpcut/statistics']
  regs = stats.elements['registered_test_cases'].text.to_i
  runs = stats.elements['run_test_cases'].text.to_i
  fails = stats.elements['failed_test_cases'].text.to_i
  dirname = nil
  begin
    dir = doc.elements['crpcut/remaining_files'].attributes['nonempty_dir']
    Dir::rmdir(dir)
  rescue SystemCallError
    report = true
    print "\n  working dir not empty"
  rescue
  end
  if !tests.empty?
  then
    print "\n  Expected tests did not run:"
    tests.each { | name, t | print "\n    #{name}" }
    report = true
  end
  if !unexpected.empty?
  then
    print "\n  Unexpected tests found:"
    unexpected.each { | name | print "\n    #{name}" }
    report = true
  end
  if failed != expected_failed
  then
    print "\n  Expected #{expected_failed} failed but found #{failed}"
    report = true
  end
  if expected_failed != rc
  then
    print "\n  Expected #{expected_failed} but returned #{rc}"
    report = true
  end
  if wrong_result.size
  then
    wrong_result.each { | n, r | print "\n#{n}#{r}\n" }
  end
  if expected_failed != fails
  then
    printf("\n  Expected %s fails, but report summary says %s",
           expected_failed,
           fails)
  end
  if expected_passed != passed
  then
    print "\n  Expected #{expected_passed} but found #{passed}"
    report = true
  end
  print "PASSED!" if !report
  puts
end

VERBOSE=[ [ '  ', /FAILED/], [ '-v', /.*/ ] ]
BLOCKING=[ [ '-n', [] ], [ '  ', [ 'blocked' ] ] ]
SLOW= [ [ '    ', ['slow', 'filesystem' ] ], [ '-c 8', [] ] ]
RUNS= [
  [ [ 'default_success' ],  VERBOSE[0], BLOCKING[1], SLOW[0], [ '--xml=yes' ] ],
  [ [ 'asserts' ],          VERBOSE[0], BLOCKING[1], SLOW[0], [ '--xml=yes' ] ],
  [ [ 'asserts' ],          VERBOSE[1], BLOCKING[1], SLOW[0], [ '--xml=yes' ] ],
  [ [ 'asserts' ],          VERBOSE[0], BLOCKING[1], SLOW[1], [ '--xml=yes' ] ],
  [ [ 'asserts' ],          VERBOSE[1], BLOCKING[1], SLOW[1], [ '--xml=yes' ] ],
  [ [ 'asserts' ],          VERBOSE[0], BLOCKING[0], SLOW[0], [ '--xml=yes' ] ],
  [ [ 'asserts' ],          VERBOSE[1], BLOCKING[0], SLOW[0], [ '--xml=yes' ] ],
  [ [ 'asserts' ],          VERBOSE[0], BLOCKING[0], SLOW[1], [ '--xml=yes' ] ],
  [ [ 'asserts' ],          VERBOSE[1], BLOCKING[1], SLOW[1], [ '--xml=yes' ] ],
  [ [ 'asserts', 'death' ], VERBOSE[0], BLOCKING[1], SLOW[0], [ '--xml=yes' ] ],
  [ [ 'asserts', 'death' ], VERBOSE[1], BLOCKING[1], SLOW[0], [ '--xml=yes' ] ],
  [ [ 'asserts', 'death' ], VERBOSE[0], BLOCKING[1], SLOW[1], [ '--xml=yes' ] ],
  [ [ 'asserts', 'death' ], VERBOSE[1], BLOCKING[1], SLOW[1], [ '--xml=yes' ] ],
  [ [ 'asserts', 'death' ], VERBOSE[0], BLOCKING[0], SLOW[0], [ '--xml=yes' ] ],
  [ [ 'asserts', 'death' ], VERBOSE[1], BLOCKING[0], SLOW[0], [ '--xml=yes' ] ],
  [ [ 'asserts', 'death' ], VERBOSE[0], BLOCKING[0], SLOW[1], [ '--xml=yes' ] ],
  [ [ 'asserts', 'death' ], VERBOSE[1], BLOCKING[0], SLOW[1], [ '--xml=yes' ] ],
  [ [],                     VERBOSE[0], BLOCKING[1], SLOW[0], [ '--xml=yes' ] ],
  [ [],                     VERBOSE[1], BLOCKING[1], SLOW[0], [ '--xml=yes' ] ],
  [ [],                     VERBOSE[0], BLOCKING[1], SLOW[1], [ '--xml=yes' ] ],
  [ [],                     VERBOSE[1], BLOCKING[1], SLOW[1], [ '--xml=yes' ] ],
  [ [],                     VERBOSE[0], BLOCKING[0], SLOW[0], [ '--xml=yes' ] ],
  [ [],                     VERBOSE[1], BLOCKING[1], SLOW[0], [ '--xml=yes' ] ],
  [ [],                     VERBOSE[0], BLOCKING[0], SLOW[1], [ '--xml=yes' ] ],
  [ [],                     VERBOSE[1], BLOCKING[0], SLOW[1], [ '--xml=yes' ] ],
  [ [],                     VERBOSE[0], BLOCKING[1], SLOW[0],
    [ '-o /tmp/crpcutst$$ -q',
       'v=$?; cat /tmp/crpcutst$$; rm /tmp/crpcutst$$; exit $v' ] ]
]

tests=TESTS;
tests.merge! GMOCK_TESTS if ARGV[0] == 'gmock'

File.open("apafil", 'w') { |f| f.write("apa\n") }
ulimit = open("|bash -c \"ulimit -c\"").read.chomp
if ulimit != "unlimited" && ulimit.to_i == 0  then
  puts "You must allow core dumps for the selt test to succeed."
  puts "Do that by issuing the command:"
  puts "> ulimit -c 100000"
  exit 1
end
puts "Self test takes a while - be patient"
RUNS.each do | names, verbosity, blocking, slowliness, specials |
  flag=''
  post=''
  if specials != [] then
    flag=specials[0]
    post="; #{specials[1]}" if specials.size > 1
  end
  tags=slowliness[1] + blocking[1]
  params="#{verbosity[0]} #{blocking[0]} #{slowliness[0]} #{flag} #{mk_tagflag(tags)} #{concat(names)} #{post}"
  printf "%-70s" % "#{params}"
  selection = tests.dup.delete_if {
    | name, test |
    !filter(names, tags, name, test) || !test.expected_result?(verbosity[1])
  }
  check_run("./test/testprog -p apa=katt --param=numeric=010 #{params}", selection)
end

dirname = "/tmp/crpcut_selftest_dir_#{$$}"
Dir.mkdir(dirname)
testname="should_fail_due_to_left_behind_files"
prog="-o /dev/null -q -d #{dirname} #{testname}"
print "%-70s" % prog
file = open("|./test/testprog #{prog}")
s = file.read
file.close
is_error=false
if !s.empty? then
  puts
  puts "Unexpected stdout for -q"
  is_error = true
end
file_found = nil
begin
  file_found = File.stat("#{dirname}/#{testname}/apa").file?
rescue
end
if !file_found then
  puts if !is_error
  puts "File was not created"
  is_error = true
end
begin
  File.unlink "#{dirname}/#{testname}/apa"
  Dir.rmdir "#{dirname}/#{testname}"
  Dir.rmdir dirname
rescue
  puts if !is_error
  puts "Couldn't remove created files"
  is_error = true
end
puts "PASSED!" if !is_error
File.unlink "./apafil"

begin
  File.unlink "./core"
rescue
  # do nothing, we don't care. Just as long as it's not there
end
testname="asserts::should_fail_void_ptr_eq_ptr"
prog="-s #{testname}"
print "%-70s" % prog
file = open("|./test/testprog #{prog}")
s = file.read
file.close
is_error=false
file_found = nil
begin
  file_found = File.stat("./core").file?
rescue
end
if !file_found then
  puts if !is_error
  puts "File was not created"
  is_error = true
end
begin
  File.unlink "./core"
rescue
  puts if !is_error
  puts "Couldn't remove created files"
  is_error = true
end
puts "PASSED!" if !is_error

