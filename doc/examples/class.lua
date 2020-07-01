#!/usr/bin/env lunacy

-- Classes in Lua.  Lua is not Java; it uses prototype inheritance.

-- Let's make our class, which, typical of Lua, is a table
Example = {}
-- This is a magic value which means which tells us to look in the
-- table "Example" for our value if it's not in our current table
ExampleMeta = {__index = Example}
-- Constructor
Example.new = function(val) 
  local o = {}
  setmetatable(o,ExampleMeta) -- If no element in o, look in Example for it
  o.val = val
  return o
end
-- Example method
Example.increment = function(self) 
  self.val = self.val + 1 
end

a = Example.new(1)
b = Example.new(10)
print(a.val)
a:increment() -- Same as a.increment(a)
print(a.val)
print(b.val)
b:increment()
print(b.val)

-- We didn't bother protecting the members (there are ways to do protection
-- in Lua, but why bother?)
a.val = a.val + 1
print(a.val)

-- Classes can be changed on the fly in Lua.  Here we give every class
-- derived from "Example" (i.e. any table which uses Example as its index
-- metatable, i.e. any table set up so, when it has an undefined value, we
-- look for the value in the "Example" table) a new member:
Example.foo = 1 -- Every instance of class now has the "foo" member
print(a.foo)
a.foo = a.foo + 1 -- Copy from example.foo then add 1.  Now that a.foo has
                  -- a value, we do not look for foo in the Example table
print(a.foo)
print(b.foo) -- This is getting foo from Example since b.foo is nil
