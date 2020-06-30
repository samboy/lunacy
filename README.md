Lunacy is a fork of Lua 5.1.5 (why 5.1?  Because it's about 20% smaller than
Lua 5.3 and because there's a lot of places stuck at Lua 5.1 -- can you 
say Roblox? -- this is a good "baseline" version of Lua) designed to
be a tiny yet powerful stand alone scripting language.

This is designed to be compiled as a tiny Windows binary, but it also
compiles and runs in Linux (CentOS 7 64-bit).

# Lunacy changes from Lua 5.1

* Lunacy is compiled as a tiny (107,520 byte) Windows 32-bit binary
  which is (as of 2020) Windows XP and Windows 10 compatible.  This
  binary is in the `bin/` folder.
* To make sure we don't have issues come January 19, 2038, `os.clock()`,
  `os.time()`, `os.date()`, and `os.difftime()` have all been removed
  (If you want to play with dates, use a 64-bit compile of Lua so we don’t
   have headaches come 2038).
* `math.random()` uses RadioGatún[32] instead of `rand()` to get higher
  quality random numbers.  `math.random()` works as usual, but there is
  now `math.rand16()`, which generates 16-bit random
  numbers (i.e. a random integer between 0 and 65,535). 
  `math.randomseed()` is here, accepts any floating point number,
  but may generate different sequences on 
  systems with different byte orders or floating point representations.
  We have also added `math.randomstrseed()` 
  which takes a NULL-terminated string (*not* number) as a random seed 
  (the seed can not have ASCII NULLs in it).  `math.randomstrseed()` should
  generate the same sequence of numbers for a given seed, regardless of
  the architecture, floating point representation, or byte order used.

