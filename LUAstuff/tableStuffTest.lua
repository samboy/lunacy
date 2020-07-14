#!/usr/bin/env lua
require("tableStuff")

-- Test sPairs
print("TEST #1: sPairs")
someTable = {foo = "bar", bar = "hello" , aaa = "zzz", aab = "xyz" }
for key, value in sPairs(someTable) do print(key, value) end
print("")

-- Test tablePrint
print("TEST #2: tablePrint")
table1 = {foo = "bar", bar = "hello" }
table2 = {foo = "t2", t1 = table1 }
table1.t2 = table2 -- Make sure this doesn't make an infinite loop
tablePrint(table1)
print("")

-- Test tableCopyD and tableCopyR
print("TEST #3: tableCopyD and tableCopyR")
foo = {a = 1, b = 2, c = {d = 3, f = 4}}
foo.c.z = foo
bar = tableCopyD(foo)
baz = tableCopyR(foo)
tablePrint(foo) -- foo.c.z should be circular loop warning
tablePrint(bar) -- Should not have bar.c.z
bar.c.d = 5
print(foo.c.d) -- 3
print(bar.c.d) -- 5
print(foo,foo.c.z) -- Should be same address twice
print(bar,bar.c.z) -- Should be address then nil
print(baz,baz.c.z) -- Should be same address twice
print(foo,baz) -- Should be different addresses
