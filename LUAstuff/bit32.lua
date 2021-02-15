-- **WARNING** This lib is *only* for Lua 5.4+, which has dropped bit32
-- this is a lib which allows Lua 5.4 to run scripts written for LuaJIT,
-- luau, and other implmentations based on some variant of Lua 5.1 + bit32
--
-- This file should be loaded as follows:
--
-- if not bit32 and tonumber((string.gsub(_VERSION,'^[^ ]+',''))) > 5.3 then
--   require("bit32")
-- end
-- 
-- Pure Lua implementation of bit32 for Lua 5.4+
-- This is only for Lua 5.4+; Lua 5.3 and Lua 5.2 have bit32, and
-- there are various workarounds for Lua 5.1 (LuaJIT uses bit, which
-- is similar; Roblox luau uses bit32; my own lunacy uses a subset of 
-- bit32; etc.)
bit32 = {}
bit32.band = function(a,...) 
  for _,v in pairs({...}) do a = a & v end
  return a & 0xffffffff
end
bit32.bor = function(a,...) 
  for _,v in pairs({...}) do a = a | v end
  return a & 0xffffffff
end
bit32.bxor = function(a,...) 
  for _,v in pairs({...}) do a = a ~ v end
  return a & 0xffffffff
end
bit32.bnot = function(a) return ~a & 0xffffffff end
bit32.lshift = function(a,r) return (a << r) & 0xffffffff end
bit32.rshift = function(a,r) return (a & 0xffffffff) >> r end
bit32.rrotate = function(a,r)
  r = r % 32
  a = a & 0xffffffff
  if r == 0 then return a end
  return ((a >> r) | (a << (32 - r))) & 0xffffffff
end
bit32.arshift = function(a,r)
  if (a & 0x80000000) == 0 then return (a & 0xffffffff) >> r end
  a = a & 0xffffffff
  for i=1,r do
     a = a >> 1
     a = a | 0x80000000
  end
  return a
end
