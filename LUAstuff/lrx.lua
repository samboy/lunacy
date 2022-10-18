#!/usr/bin/env lunacy64
-- This script does regular expression parsing similar to perl -pe, but
-- using Lua5.1/Lunacy regexes instead of Perl regex.  

-- This script processes input from standard input and outputs to standard
-- output.  The script takes a variable number of arguments, but needs
-- to have at least one argument.

-- Unless otherwise specified, the output is the same as the input

-- If the script is given one argument, then the given regular expression
-- is removed from the output

-- If the script is given two arguments, then the regular expression in the
-- first argument is replaced with the string in the secon argument

-- If the script is given an even number of arguments, then each odd 
-- numbered argument (1, 3, 5, 7, etc.) is a regular expression which is
-- replaced by a string in the subsequent argument

-- For example, echo foo | lua5.1 lrx.lua 'f' 'g' 'o' 'a' 
-- is the same as echo foo | perl -pe 's/f/g/g;s/o/a/g;'
-- or for that matter echo foo | sed 's/f/g/g;s/o/a/g;'
-- The output is gaa

-- regular expression replacement strings can be blank:
-- echo foo | lua5.1 lrx.lua 'f' '' 'o' 'a' # Output is aa

-- If lrx is given an odd number of arguments, then the final argument
-- is a regular expression which will be removed in the input

-- echo foo | lua5.1 lrx.lua 'o' 'a' 'f' # Output is aa

-- Regular expressionare are Lua regular expressions.  The main difference
-- is that they are not as full features as Perl compatible regular 
-- expressions, and that % instead of \ is used for meta characters
-- See section 5.4.1 of the Lunacy or Lua 5.1 manual for details:
-- https://www.lua.org/manual/5.1/manual.html#5.4.1
-- https://maradns.samiam.org/lunacy/lunacy-manual.html#5.4.1

-- This script has been donated to the public domain in 2022 by Sam Trenholme
-- If, for some reason, a public domain declation is not acceptable, it
-- may be licensed under the following terms:

-- Copyright 2022 Sam Trenholme
-- Permission to use, copy, modify, and/or distribute this software for 
-- any purpose with or without fee is hereby granted.
-- THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
-- WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
-- OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
-- ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
-- WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
-- ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
-- OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

before = {}
after = {}
index = 1
for a=1,#arg,2 do
  before[index] = arg[a]
  if arg[a + 1] then 
    after[index] = arg[a + 1]
  else
    after[index] = ""
  end
  index = index + 1
end

-- This is code which reads and processes lines from 
-- standard input
l = io.read()
while l do
  for a=1,#before do
    l = string.gsub(l,before[a],after[a])
  end
  print(l)
  l = io.read()
end
