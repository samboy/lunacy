-- Public domain example of Objects in Lua
--
-- This version uses metatables instead of object function factories

Add1Obj = {}
Add1Obj.__index = Add1Obj

function Add1Obj:new(start) 
  if start == nil then
    start = 1
  end
  return setmetatable({v = start},self)
end

function Add1Obj:add1()
  self.v = self.v + 1
  return self.v
end

foo = Add1Obj:new(1)
print(foo:add1())
print(foo:add1())

Add2Obj = {}
Add2Obj.__index = Add2Obj
setmetatable(Add2Obj, Add1Obj)

function Add2Obj:add2()
  self.v = self.v + 2
  return self.v
end

foo = Add2Obj:new(20)
print(foo:add1())
print(foo:add2())
