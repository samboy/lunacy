#!/usr/bin/env lua

-- Some helper functions
function wrap(f,x) local a = f(x) print(a) return a end
function log10(x) return math.log(x) / math.log(10) end
function deg2rad(x) return math.pi * x / 180 end
-- Some convienent ways to call log and trig functions
function ln(x) return wrap(math.log,x) end
function lc(x) return wrap(log10,x) end -- lc: common log
function sin(x) return wrap(math.sin,deg2rad(x)) end
function cos(x) return wrap(math.cos,deg2rad(x)) end
function tan(x) return wrap(math.tan,deg2rad(x)) end

-- Simple calculator for Lua 5.1
while true do
  io.write("] ")
  local l = io.read()
  local z, y = pcall(loadstring(l))
  if z then 
    if y ~= nil then
      print(y)
    end
  else
    local a, b = pcall(loadstring("return " .. l))
    if a then print(b) end
  end
end
