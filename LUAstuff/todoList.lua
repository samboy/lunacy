#!/usr/bin/env lua

-- This is a to do list.  This to do list can handle appointments on a given
-- day, weekly recurring items, and monthly recurring items
------------------------------------
function init()
  local out = {}
  -- This trick lets us have a quick form of adding items to the to do list
  out["i"] = table.insert
  return out
end

onday = {} -- On a specific day
weekly = {} -- Weekly recurring items
monthly = {} -- Monthly recurring items

-- Put to do items here
-- Sunday
weekly[0] = init()
weekly[0]:i("Go to church")

-- Monday
weekly[1] = init()
weekly[1]:i("Monday night football")
weekly[1]:i("Get chips for football game.")

-- Tuesday
weekly[2] = init()
weekly[2]:i("Tuesday stuff")

-- Wednesday
weekly[3] = init()

-- Thursday
weekly[4] = init()

-- Friday
weekly[5] = init()

-- Saturday
weekly[6] = init()


for day = 1,28 do
  monthly[day]=init()
end
-- Pay the credit card on the 21st of every month
monthly[21]:i("Pay credit card")

-- On a specific day
onday["2022-09-16"] = init()
onday["2022-09-16"]:i("Job interview with Frobozz inc.")

-- END SCHEDULED APPOINTMENTS
--------------------------------------
print()
print("==TODO FOR TODAY==")
-- A tiny todo engine
if lunacy then
  year, mon, day, wday = lunacy.today()
else
  local a = os.date("*t") year=a.year mon=a.month day=a.day wday=a.wday - 1
end

function isLeapYear(year) 
  if year % 4 == 0 then
    if year % 100 == 0 and year % 400 ~= 0 then
      return false
    end
    return true
  end
  return false
end

-- Calculate the day of the week
-- Input: year, month, day (e.g. 2022,9,16)
-- Output: day of week (0 = Sunday, 6 = Saturday)
function dayOfWeek(year, month, day)
  -- Tomohiko Sakamoto algorithm
  local monthX = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4}
  if month < 3 then year = year - 1 end
  local yearX = (year + math.floor(year / 4) - math.floor(year / 100) +
                 math.floor(year / 400))
  local out = yearX + monthX[month] + day
  out = out % 7
  return out
end

function daysInMonth(month)
  local mdays = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
  if isLeapYear(year) then mdays[2] = 29 end
  if month < 1 then month = month + 12 end
  if month > 12 then month = month - 12 end
  return mdays[month]
end

function tomorrow(year, mon, day, wday)
  day = day + 1
  if day > daysInMonth(mon) then 
    day = 1
    mon = mon + 1
  end
  if mon > 12 then
    mon = 1
    year = year + 1
  end
  wday = wday + 1
  if wday > 6 then
    wday = 0
  end
  return year, mon, day, wday
end

function show(year, mon, day, wday)
  local date = string.format("%d-%02d-%02d",year,mon,day)
  local seen = false

  -- Weekly items
  if weekly[wday] then 
    for a=1,#weekly[wday] do print(weekly[wday][a]) seen = true end 
  end

  -- Monthly items

  -- Make sure to have stuff for each 29th/30th/31st show up early the
  -- next month.  Be sure to not put stuff like bills with a hard
  -- due date on the 29/30/31 because, to avoid having too many items
  -- on February 28th, we move stuff after the end of the month to the
  -- next month
  if day > daysInMonth(mon - 1) then day = day - daysInMonth(mon - 1) end

  if monthly[day] then 
    for a=1,#monthly[day] do print(monthly[day][a]) seen = true end 
  end

  -- Items on a specific date
  if onday[date] then 
    for a=1,#onday[date] do print(onday[date][a]) seen = true end 
  end

  if not seen then print("Nothing on todo list this day") end

end

show(year,mon,day,wday)
print()
print("==TODO FOR TOMORROW==")
show(tomorrow(year,mon,day,wday))

