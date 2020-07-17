----------------------- rCharSplit() -----------------------
-- Input: A string we will split, a character (class) we split on
-- Output: An array (numeric indexed table) with the split string
-- Should the character we split on not be in the string to split, 
-- we will have a one-element array with the input string
--
-- Note: Should the splitter regex be a single character, things will 
-- split as expected, but 0-length strings are converted in to a 
-- one-character long space (" ") string.
--
-- Example usage:
-- a = "1,2,3" t = rSplit(a,",") for k,v in pairs(t) do print(k,v) end
function rCharSplit(i, c)
  local out = {}
  local n = 1
  local q

  -- For one-character separators, like ",", we allow empty fields
  if string.len(tostring(c)) == 1 then
    i = string.gsub(i, tostring(c) .. tostring(c),
                    tostring(c) .. " " .. tostring(c))
  end

  for q in string.gmatch(i, "[^" .. tostring(c) .. "]+") do
    out[n] = q
    n = n + 1
  end
  return out
end 

----------------------- rStrSplit() -----------------------
-- This does a simple split for a given string, useful for simple CSV
-- Input: string (single CSV line), split character 
-- Output: An array with each field in the CSV line
function rStrSplit(s, splitOn)
  if not splitOn then splitOn = "," end
  local place = 1
  local out = {} 
  local last = 1
  while place do
    place = string.find(s, splitOn, place, true) 
    if place then
      table.insert(out,string.sub(s, last, place - 1))
      place = place + 1
      last = place
    end
  end 
  table.insert(out,string.sub(s, last, -1))
  return out
end

----------------------- qCsvSplit() -----------------------
-- This handles the pesky issue of quoted CSV like this:
-- '"foo, bar",2,"field 3, you know",This is field 4'
-- s: String to split (a single CSV line)
-- splitOn: Character to split on
-- leftQuote: The character which starts a quote
-- rightQuote: The character which ends a quote
-- Output: An array with each field in the CSV line
-- Note that leftQuote and rightQuote do not included in the
-- final output array, since the usual intention with quoted CSV is to
-- not have the quotes in the actual fields
function qCsvSplit(s, splitOn, leftQuote, rightQuote)
  if not splitOn then splitOn = "," end
  if not leftQuote then leftQuote = '"' end
  if not rightQuote then rightQuote = leftQuote end
  local out = {}
  local place = 1
  local lastSplitter = 1
  local state = 1 -- Finite state machine state
  local toAdd = ""
  while place <= string.len(s) do
    local look = string.sub(s,place,place)
    if look == splitOn and state == 1 then
      table.insert(out,toAdd) 
      toAdd = ""
    elseif look == leftQuote and state == 1 then
      state = 2
    elseif look == rightQuote and state == 2 then
      state = 1
    else
      toAdd = toAdd .. look
    end
    place = place + 1
  end
  table.insert(out,toAdd)
  return out
end

-- Make a large number look like a nice number with commas
-- Input: The number, separator (make "." for European numbers)
-- Output: A string with the number looking nice
-- Example usage:
-- foo = 1234567890 print(humanNumber(foo,",","%d"))
function humanNumber(n, separator, fmt)
  if not fmt then fmt = "%.2f" end
  if not separator then separator = "," end -- I am in the US
  local out = ""
  if n < 0 then 
    out = "-" 
    n = -n 
  end
  local parts = {}
  if n < 1000 then return out .. string.format(fmt, n) end
  low = n % 1000
  n = math.floor(n / 1000)
  while n > 0 do
    if n > 0 then table.insert(parts, n % 1000) end
    n = math.floor(n / 1000)
  end
  for i = #parts, 1, -1 do
    if i == #parts then out = out .. tostring(parts[i])
    else out = out .. separator .. string.format("%03d",parts[i]) end
  end
  out = out .. separator
  if(low < 100) then out = out .. "0" end
  if(low < 10) then out = out .. "0" end
  out = out .. string.format(fmt, low)
  return out
end
