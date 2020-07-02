#!/usr/bin/env lua

-- File operations in Lua.  "lfs" (version 1) needed.

-- Some distros come with a "lua-lfs" or similar package, if not
-- fetch it from https://github.com/keplerproject/luafilesystem or
-- via Lua Rocks.

if not lfs then lfs = require "lfs" end
if not lfs then print "lfs library needed" os.exit(1) end

-------------------------- fileCopy() --------------------------
-- copy a file at location "source" to "destination"
-- If "b" is already present then fail
-- Fail if read or write do not work
-- Return true or false; if return false, also return error string
function fileCopy(source, destination)
  if lfs.attributes(destination) then
    return false, "filecopy will not overwrite " .. destination
  end
  local i = io.open(source, "rb")
  if not i then return false, "could not open " .. source end
  local o = io.open(destination, "wb")
  if not o then
    i:close()
    return false, "could not write to " .. destination 
  end
  local buffer = i:read(32768)
  while buffer do
    o:write(buffer)
    buffer = i:read(32768)
  end
  i:close()
  o:close()
  return true, "wrote to " .. destination
end

