----------------------- sortedTableKeys() -----------------------
--  Input: A table
--  Output: An array (i.e. table with only numeric keys, starting at 1
--          and counting upwards without any gaps in the numeric sequence)
--          with the input table's keys lexically (alphabetically) sorted
--
--  I created this routine when I wanted to make a version of pairs()
--  guaranteed to traverse a table in a (mostly) deterministic fashion.

function sortedTableKeys(t, sortBy)
  local a = {}
  local b = 1
  if not sortBy then
    sortBy = function(y,z) return tostring(y) < tostring(z) end
  end
  for k,_ in pairs(t) do -- pairs() use OK; will sort
    a[b] = k
    b = b + 1
  end
  table.sort(a, sortBy)
  return a
end

-- This works like sortedTableKeys(), but sorts based on the values
-- in the table (which are assumed to be numbers); highest numbers come
-- first.
-- For example, if we have this
-- a = {bar = 2, baz = 3, foo = 1}
-- Then run sortedByRevValue on a, we will get
-- {"baz", "bar", "foo"}, since "baz" has the highest value, "bar" has
-- the second highest value, and "foo" has the lowest value.
function sortedByRevValue(t)
  local a = {}
  local b = 1
  local c = t
  for k,_ in pairs(t) do
    a[b] = k
    b = b + 1
  end
  local function s(x, y)
    return tonumber(c[x]) > tonumber(c[y])
  end
  table.sort(a, s)
  return a
end

----------------------- tablePrint() -----------------------
-- Print out a table on standard output.  Traverse sub-tables, avoid
-- circular traversals.  The code here has three arguments, but
-- we only need one argument when looking at a table; the "prefix"
-- and "seen" arguments are only used when tablePrint recursively looks
-- at a sub-table.
-- Usage
-- t = {foo = "bar", baz = "b"} t2 = {l1 = "hi", t1 = t} t.z = t2 tablePrint(t)
function tablePrint(t, prefix, seen)
  if not seen then 
    seen = {}
    seen[tostring(t)] = true
  end
  for k,v in sPairs(t) do
    if type(v) == "table" then
      if not seen[tostring(v)] then
        seen[tostring(v)] = true
        local prefixR
        if prefix then
          prefixR = prefix .. ">" .. tostring(k) .. ":"
        else
          prefixR = tostring(k) .. ":"
        end
        tablePrint(t[k],prefixR,seen)
      else
        if prefix then
          print(prefix, k, v, "Already seen, not traversing")
        else
          print(k, v, "Already seen, not traversing")
        end
      end
    else
      if prefix then
        print(prefix, k, v)
      else
        print(k, v)
      end
    end
  end
end

----------------------- tableCopyD(t) -----------------------
-- Input: Table with no duplicate references (sub-tables are OK, but
-- there must be no "hard link" tables; if there are, we only copy
-- one of the hard link tables)
-- The second argument is a table, where the keys are table pointers,
-- which lists tables we have already seen or copied
-- Output: Copy of table
-- example usage:
-- foo = {a = 1, b = 2, c = {d = 3, f = 4}}
-- print(foo.c.d) -- 3
-- bar = tableCopyD(foo)
-- print(bar.c.d) -- also 3
-- bar.c.d = 5
-- print(foo.c.d) -- still 3
-- print(bar.c.d) -- now 5
function tableCopyD(t, seen)
  if not seen then seen = {} end
  local out = {}
  seen[t] = true
  for k, v in pairs(t) do
    if type(v) == "table" then
      if not seen[v] then
        out[k] = tableCopyD(v, seen)
      end
    else
      out[k] = v
    end
  end
  return out
end

----------------------- sPairs(t) -----------------------
-- Input: Table
-- Ouput: Iterator used by "for" which will go through the keys in
-- a table in a sorted order, e.g. 
-- someTable = {foo = "bar", bar = "hello" , aaa = "zzz", aab = "xyz" }
-- for key, value in sPairs(someTable) do print(key, value) end
-- The sorter, if present, is a pointer to a function.  This function
-- takes the table t as an input, and returns a 1-indexed array (i.e.
-- table with only ordered numeric keys) where the values are table keys
-- (presumably sorted).
-- For example, sPairs(t,sortedByRevValue) iterates through the table
-- as per the value of each table element, reverse numeric sorted
function sPairs(t, sorter)
  if not sorter then sorter = sortedTableKeys end
  local function _tableIter(t, _)
    local k = t.s[t.i]
    local v
    if k then
      v = t.t[k]
    else
      return nil
    end
    t.i = t.i + 1
    if v then
      return k, v
    end
  end
  local tt = {}
  tt.s = sorter(t)
  tt.t = t
  tt.i = 1
  return _tableIter, tt, nil
end
