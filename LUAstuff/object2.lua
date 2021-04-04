-- Public domain example of Objects in Lua
--
-- This version uses metatables instead of object function factories

-- Let’s create a class
Add1Obj = {} -- Initialize a new class
Add1Obj.__index = Add1Obj -- Allow class to be a metatable

-- Method: Initialize class
function Add1Obj:new(start) 
  if start == nil then
    start = 1
  end
  return setmetatable({v = start},self)
end

-- Method: Add one to the class’s “v” element
function Add1Obj:add1()
  self.v = self.v + 1
  return self.v
end

-- Example of class usage
foo = Add1Obj:new(1)
print(foo:add1())
print(foo:add1())

-- Now, let’s create a derived class
Add2Obj = {} -- Initialize a new class
Add2Obj.__index = Add2Obj -- Allow this class to be a metatable
setmetatable(Add2Obj, Add1Obj) -- Inherit the Add1Obj class

-- Derived class has a new method
function Add2Obj:add2()
  self.v = self.v + 2
  return self.v
end

-- Example using derived class
foo = Add2Obj:new(20)
print(foo:add1())
print(foo:add2())
