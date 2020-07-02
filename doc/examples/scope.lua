#!/usr/bin/env lunacy

-- Scope is this
-- * Function args are local in the function
-- * Loop values are local to the block the loop encloses
-- That in mind

-- Global "a" is not affected
function f1(a) 
  a = a + 1
  print(a)
end

-- Global "a" is affected
function f2()
  a = a + 1
  print(a)
end

-- Numeric for loop and scope
function f3(a)
  -- This for loop does not affect the "a" given to the function
  for a = 1, 10 do 
    print(a)
  end
  print("")
  print(a)
end

-- Functional for loop and scope
function f4(a, b)
  c = {x = 1, y = 2, z = 3}
  -- This does not affect the "a" and "b" given to the function either
  for a, b in pairs(c) do
    print(a, b)
  end
  print("")
  print(a, b)
end

-- Affecting global values directly
function f5(a)
  a = a + 1 -- This does not affect the global a
  _G.a = _G.a - 1 -- We can specify we want to affect the global a
  print(a)
  print(_G.a)
end

print("f1")
a = 1
f1(a) -- 2
print(a) -- 1
print("") print("f2")
f2()
print(a) -- 2
print("") print("f3")
f3(a) -- 1 to 10, then 2
b = 1
print("") print("f4")
f4(a, b) -- table elements in random order, then "2 1"
print("") print("f5")
-- Remember, global a is 2 right now
f5(a) -- 3 then 1
print("")
print(a) -- 1 again
