#!/usr/bin/env lua
require("regexStuff")
require("tableStuff")
a="a,2,c,life,,12345"
tablePrint(rCharSplit(a,","))
print("")

b="This is a big  test!"
tablePrint(rCharSplit(b,"%s"))
