Lunacy is a fork of Lua 5.1.5 (why 5.1?  Because it's about 20% smaller than
Lua 5.3 and because there's a lot of places stuck at Lua 5.1 -- can you 
say Roblox? -- this is a good "baseline" version of Lua) designed to
be a tiny yet powerful stand alone scripting language.

This is designed to be compiled as a tiny Windows binary, but it also
compiles and runs in Linux (CentOS 7 64-bit).

# Lunacy changes from Lua 5.1

* Lunacy is compiled as a tiny (117,760 byte) Windows 32-bit binary
  which is (as of 2020) Windows XP and Windows 10 compatible.  This
  binary is in the `bin/` folder.
* To make sure we don't have issues come January 19, 2038, `os.clock()`,
  `os.time()`, `os.date()`, and `os.difftime()` have all been removed
  (If you want to play with dates, use a 64-bit compile of Lua so we don’t
   have headaches come 2038).
* `math.random()` uses RadioGatún[32] instead of `rand()` to get higher
  quality random numbers.  `math.random()` works as usual, but there is
  now `math.rand16()`, which generates 16-bit random numbers (i.e. a 
  random integer between 0 and 65,535).  `math.randomseed()` takes a 
  NULL-terminated string (*not* number) as a random seed (the seed can 
  not have ASCII NULLs in it).  If a number is given to 
  `math.randomseed()`, Lua’s coercion converts it in to a string.
* This code does not support runtime loading of dynamic libraries.
* The luafilesystem suite is built in.  Not everything works, but basic
  directory and file traversal are present.
* `bit32` libs, based on the Lua 5.2 and Lua 5.3 API, are here for bit
  manipulation.

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
