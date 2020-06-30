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

