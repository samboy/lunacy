#!/usr/bin/env lua

-- Mailing lists
-- Key is either email (has @, string value) -or- list name (no @, list value)
emails={}
e=emails
-- Myself
e['joe@example.com'] = 'Joe Smith'
e['some.random.dude@gmail.com'] = 'Mr. Random'
-- A mailing list
e["Hangout"] = {'joe@example.com', 'some.random.dude@gmail.com' }

function showEmail(list) 
  who = emails[list]
  if type(who) == 'string' then print(list, who) return 0 end
  if type(who) == 'table' then
    for a=1,#who do
      if type(who[a]) == 'string' then print(who[a], emails[who[a]]) end
    end
  end
end

function usage()
  print("Usage: emailLists.lua email@address.example.com")
end

-- Process args
action = {usage, 0}
if #arg == 1 then 
  if emails[arg[1]] then action = {showEmail, arg[1]} end
end

-- List members can be functions in Lua
action[1](action[2])

