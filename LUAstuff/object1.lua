-- Public domain example of Objects in Lua

-- Here, we have a function which returns an object.
-- Lua does not have objects.  It has tables.  But, with some
-- coding and with some metatables, we can have things which
-- act just like objects

function Add1Obj(start, isDerivedClass) 
  -- Default to "one" if start value is not set
  if start == nil then
    start = 1
  end
  local protected = {}
  protected.v = start
  function add1()
    protected.v = protected.v + 1
    return protected.v
  end
  if isDerivedClass then
    return {protected = protected, add1 = add1}
  else
    return {add1 = add1}
  end
end

foo = Add1Obj(1)
print(foo.add1())
print(foo.add1())

-- Now, let’s have a derived class
function Add2Obj(start, isDerivedClass)
  local out = Add1Obj(start, true)
  local protected = out.protected
  if not isDerivedClass then 
    out.protected = nil
  end
  function add2()
    protected.v = protected.v + 2
    return protected.v
  end
  out.add2 = add2
  return out
end

foo = Add2Obj(20)
print(foo.add1())
print(foo.add2())
