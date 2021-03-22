Lunacy is a fork of Lua 5.1.5 (why 5.1?  Because it's about 20% smaller
than Lua 5.3 and because there’s a lot of code based on Lua 5.1: Roblox
Luau, LuaJIT, Gopher Lua, Adobe Lightroom Classic, etc.) designed to be
a tiny yet powerful stand alone scripting language.

This is designed to be compiled as a tiny Windows binary, but it also
compiles and runs in Linux (CentOS 7 64-bit).

# Lunacy changes from Lua 5.1

* Lunacy is compiled as a tiny (115,200 byte) Windows 32-bit binary
  which is (as of 2021) Windows XP and Windows 10 compatible.  This
  binary is in the `bin/` folder.
* **Security fix** Lunacy uses HalfSipHash-1-3 as its string hash
  compression function.  It is also possible to use SipHash-1-3 or
  SipHash-2-4 as the compression function; see the section SipHash
  below for details.
* To make sure we don't have issues come January 19, 2038, `os.clock()`,
  `os.date()`, and `os.difftime()` have all been removed (If you want 
  to play with dates, use a 64-bit compile of Lua so we don’t
  have headaches come 2038).  `os.time()` is here (and is Y2038 compliant,
  both as a 32-bit and 64-bit build: The Windows build uses the Y2038
  compliant “FileTime” API, and the 32-bit *NIX build gives negative
  timestamps post-Y2038 values), but only returns the current
  system time.
* `math.random()` uses RadioGatún[32] instead of `rand()` to get higher
  quality random numbers.  `math.random()` works as usual, but there is
  now `math.rand16()`, which generates 16-bit random numbers (i.e. a 
  random integer between 0 and 65,535) in a manner which allows one to 
  recreate RadioGatún[32]’s test vectors.  `math.randomseed()` takes a 
  NULL-terminated string (*not* number) as a random seed (the seed can 
  not have ASCII NULLs in it).  If a number is given to 
  `math.randomseed()`, Lua’s coercion converts it in to a string.
* The RadioGatun[32]-based `math.random()` routines can also be called
  with `rg32` aliases: `rg32.randomseed()`, `rg32.random()`, and 
  `rg32.rand16()`.  This allows one to use Lunacy’s random number
  generator via a third party library in stock Lua; Lunacy-compatible
  libraries for stock Lua are at https://github.com/samboy/LUAlibs
* This code does not support runtime loading of dynamic libraries.
* The luafilesystem suite is built in.  Not everything works, but basic
  directory and file traversal are present.
* `bit32` libs, based on the Lua 5.2 and Lua 5.3 API, are here for bit
  manipulation.
* A module by Steve Donovan called `spawner` is here so we can have a 
  version of Python’s old `popen2` in Luancy.  For users of stock Lua,
  this library for Lua is available at https://github.com/samboy/LUAlibs
* It is now possible to have Lunacy, when run in terminal mode,
  return the result of any expression which starts with a number (i.e.
  any character between `0` and `9`).  This gives Lunacy “desktop 
  calculator” support, allowing one to easily use it to perform numeric
  computations.

# Changelog

* `2021-03-21` When run in terminal mode, if the first character in an
  expression is a number, we return the result of the expression.  In 
  other words, Lunacy now is a “quick and simple” desktop calculator:
  Type in “lunacy”, then type in a numeric math expression one wants to 
  solve, e.g. `2 ^ 35`, and Lunacy will return the answer without needing
  to have the line begin with `=`.
* `2021-03-06` Fully document `lfs` (luafilesystem) in Lunacy manual.
  Remove spawner.c *NIX compile-time warnings.  Lunacy binary still at
  2021-02-22.
* `2021-02-27` Documentation update for Lunacy 2021-02-22.  `spawner` now
  has documentation; `rg32` random number routines (also with `math` 
  aliases) are now fully documented.  Basic documentation for `lfs` with
  pointer to full documentation added.  Lunacy code unchanged.
* `2021-02-22` `rg32.runmill` added to code; Lunacy date updated.
* `2021-02-21` Add `rg32.randomseed`, `rg32.random`, and `rg32.rand16`
  aliases for `math.random`, `math.randomseed`, and `math.rand16`.  This
  allows one to write code using the same random number generator in
  both Lunacy and stock Lua with the libs available at 
  https://github.com/samboy/LUAlibs
* `2021-02-15` Add `spawner` routines that allow Lunacy to have two-way
  pipes (both reading and writing) with child processes.
* `2020-12-06` Restore simple `os.time()` (numeric *NIX timestamp) routine
  which uses a Y2038-compatible Windows API (filetime), full Y2038 support 
  when `time_t` is 64-bit (i.e. most modern *NIX OSes), and support until 
  around 2100 if `time_t` is 32-bit (when time_t is negative, we add 
  2^32 seconds to the time).
* `2020-08-12` Lunacy now uses HalfSipHash-1-3 for hash compression.  
  While slightly slower than Lua’s default hash compressor, it protects us
  from hash flooding denial of service attacks.

# SipHash

Lunacy, by default, uses HalfSipHash-1-3 as its hash compression
algorithm.  This has reasonable security, while being very fast
when Lunacy is compiled as a 32-bit or 64-bit binary.

To instead compile Lunacy to use 64-bit SipHash-1-3, edit 
`src/Makefile` to add the flag `-DFullSipHash`, e.g.:

```
CFLAGS= -O3 -Wall $(MYCFLAGS) -DFullSipHash
```

To use 64 bit SipHash-2-4, likewise add `-DSIP24`:

```
CFLAGS= -O3 -Wall $(MYCFLAGS) -DFullSipHash -DSIP24
```

## Why HalfSipHash-1-3 is the default

I have run a number of benchmarks with Lunacy, my fork of Lua 5.1,
to see how much changing the SipHash variant used affects performance,
for both 32-bit (386) and 64-bit (x86_64) binaries.

Conclusion: I will use HalfSipHash31 as the hash compression algorithm,
for both 32-bit and 64-bit builds of Lunacy.

The binaries have been compiled using GCC 8.3.1, in CentOS 8, using an
older Core Duo T8100 chip from 2008.  The benchmark consisted of loading
and processing a bunch of COVID-19 data in to large tables taking up
550 (32-bit) or 750 megs (64-bit) of memory.  This real-world benchmark
(it is the exact same code I use to build an entire COVID-19 tracking
website) was done multiple times, to minimize speed fluctuations from
outside factors, against the following setups:

* “Lunacy32”, which is a 32-bit compile of Luancy
* “Lunacy64”, a 64-bit compile of same

And the following string hash compression functions:

* “noSipHash”: Lua’s default hash compressor
* “SipHash24”: 64-bit SipHash with 2 rounds during input processing,
  followed by 4 rounds after input ends.
* “SipHash13”: 64-bit SipHash with 1 round during input processing,
  followed by 3 rounds after input ends.
* “SipHalf13”: 32-bit HalfSipHash with 1 round during input
  processing, followed by 3 rounds after input ends.

Here are the results, where lower numbers are better (less time needed
to run the benchmark):

```
lunacy64-noSipHash 197.801
lunacy64-sipHash13 203.457
lunacy64-SipHalf13 203.507
lunacy64-sipHash24 210.043
lunacy32-noSipHash 240.898
lunacy32-SipHalf13 246.995
lunacy32-sipHash13 265.916
lunacy32-sipHash24 270.226
```

HalfSipHash-1-3 is as fast as full SipHash-1-3 on 64-bit CPUs, while
being quite a bit faster for 32-bit binaries compared to 64-bit sipHash.

HalfSipHash-1-3 is only 2.5% slower on 32-bit machines (compared to
Lua’s  “stock” hash); it is only 2.9% slower on 64-bit machines.

In Lunacy’s use case, HalfSipHash should provide an adequate security margin;
as per [what its designer has to 
say](http://lkml.iu.edu/hypermail/linux/kernel/1612.2/01666.html):

>HalfSipHash takes its core function from Chaskey and uses the same
>construction as SipHash, so it *should* be secure. Nonetheless it hasn't
>received the same amount of attention as 64-bit SipHash did. So I'm less
>confident about its security than about SipHash's, but it obviously inspires
>a lot more confidence than non-crypto hashes.
>
>Too, HalfSipHash only has a 64-bit key, not a 128-bit key like SipHash, so
>only use this as a mitigation for hash-flooding attacks, where the output of
>the hash function is never directly shown to the caller.

# See also

Some other languages based on Lua:

* https://github.com/lua/lua Official Lua Github mirror
* https://github.com/paly2/ALua One Lua variant
* https://github.com/mingodad/ljs Lua variant with C/Javascript syntax
  (also available: "ljs" variants of Lua 5.1, LuaJIT)
* https://github.com/esle/Bracket-ed-Lua Lua with {} loop brackets
* https://github.com/leafo/moonscript Lua with Python-like syntax
  (compiles to Lua)
* https://github.com/yuin/gopher-lua Lua in Go
* https://roblox.github.io/luau/ Lua for Roblox (not open source)
* https://github.com/NLua/NLua Lua for .net
* https://github.com/bakpakin/Fennel Lisp variant that compiles to Lua
* https://github.com/fengari-lua/fengari Javascript implementation of Lua
* https://github.com/moonsharp-devs/moonsharp C sharp Lua implementation
* https://github.com/SquidDev/urn Another Lisp variant that compiles to Lua
* https://github.com/teal-language/tl/ Strongly typed Lua variant 
* https://github.com/LiXizhi/NPLRuntime Neural Parallel Language
* https://github.com/hengestone/lua-languages Another list of Lua-like 
  languages
* https://github.com/presidentbeef/brat Semi-esoteric language which compiles
  to Lua

# LuaJIT

LuaJIT is a high performance implementation of Lua 5.1.

* https://github.com/LuaJIT/LuaJIT

Since development for LuaJIT has slowed down,
there are some forks of it:

* https://github.com/moonjit/moonjit/
* https://github.com/raptorjit/raptorjit
 
