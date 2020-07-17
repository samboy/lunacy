#!/usr/bin/env lua
require("regexStuff")
require("tableStuff")
a="a,2,c,life,,12345"
tablePrint(rCharSplit(a,","))
print("")

b="This is a big  test!"
tablePrint(rCharSplit(b,"%s"))

--- rStrSplit
b="1,,2,3,45,Hello there!,6,789"
print("rStrSplit test #1 ---------------------------")
tablePrint(rStrSplit(a))
print("rStrSplit test #2 ---------------------------")
tablePrint(rStrSplit(b))
print("rStrSplit test #3 ---------------------------")
for i,v in ipairs(rStrSplit(a)) do
  print(v,string.len(v))
end
-- In comparison, this will not have a 0 len string
print("rStrSplit test #4 ---------------------------")
for i,v in ipairs(rCharSplit(a,",")) do
  print(v,string.len(v))
end
c='"foo, bar",2,"field 3, you know",This is field 4'
-- This is where rStrSplit breaks down
print("rStrSplit test #5 ---------------------------")
for i,v in ipairs(rStrSplit(c)) do
  print(v,string.len(v))
end
-- So we need something which can handle quoted CSV stuff
print("qCsvSplit test ------------------------------")
for i,v in ipairs(qCsvSplit(c)) do
  print(v,string.len(v))
end
