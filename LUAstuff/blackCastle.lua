-- Donated to the public domain 2021 Sam Trenholme
-- I could not find a CC0 licensed native Lua JSON parser, so I
-- have written my own (there are plenty of MIT licensed ones)

-- This is called "Black Castle" because Douglas Crockford's (same guy
-- who invented JSON) old game "Galahad and the Holy Grail" had a very 
-- difficult puzzle where a secret passage was hidden in the black castle; 
-- the passage was so difficult to find, I had to run the program through 
-- a 6502 monitor to find it.  

-- A 1984 review of "Galahad and the Holy Grail" had the same complaint:
-- In "The book of Atari Software 1984", the reviewer could not find this
-- secret passage until Atari sent them a map and hint sheet.

-- To Douglas Crockford's credit, the 1985 Antic version of the game has
-- an arrow in the black castle pointing to the secret passage.

-- This only parses JSON input; this can not handle backslashes in strings;
-- this parser assumes that strings are binary blobs where " only appears
-- right after the \ character.
-- 
-- This does not generate JSON; this only reads JSON from a file; this uses
-- tonumber for numeric generation; this allows comments (with #) and bare
-- words for object keys (both useful non-JSON extensions); this parser 
-- doesn't care where or one puts (or doesn't put) a "," or ":"; this parser 
-- makes null in JSON the string "--NULL--" by default (can be changed to
-- other values); this parser is a quick and dirty hack.

-- Usage:

--   require("blackCastle")
--   foo = blackCastle("file.json")

-- blackCastle input: File name with JSON we wish to read
--             optional input: If the JSON has a null entry, what value
--                             we should give null.  Defaults to the 
--                             string "--NULL--"

function blackCastle(filename, nullvalue)
  local globalError = nil
  local lineNumber = 1
  if not nullvalue then nullvalue = "--NULL--" end

  -- While comments are not part of standard JSON, we support # comments
  function processComment(jsonF) 
    char = true
    while char and char ~= "\n" do
      char = jsonF:read(1)
    end
    if char == "\n" then lineNumber = lineNumber + 1 end
    return char
  end

  -- Is a given character white space?
  function isWhitespace(char)
    if char == "\n" then lineNumber = lineNumber + 1 end
    return char == " " or char == "\t" or char == "\r" or char == "\n" 
  end

  -- Grab a string from JSON input
  function getString(jsonF)
    local out = ""
    local char = true
    while char do
      char = jsonF:read(1)
      if not char then 
        globalError = "Premature end of file"
        return nil 
      end
      if char == '"' then return out end
      -- To correctly add support for \ in JSON strings, this library
      -- would need to have an entire Unicode subsystem: Convert Unicode
      -- codepoints to UTF-8; convert UTF-8 sequences in to Unicode 
      -- code points (or, at least, make sure we don’t have invalid UTF-8
      -- in our strings); convert surrogate pairs in to single code points;
      -- etc.  It would make this quick and dirty JSON library about four
      -- times larger.
      -- So, instead, we just pass the string the JSON gives us as-is as
      -- a "binary blob" to Lua.  The UTF-8 infinity symbol will become
      -- a literal UTF-8 infinity symbol (Hex: E2 88 9E); the sequence
      -- '\u221e' in a JSON string (which represents the infinity symbol)
      -- will remain the ASCII string "\u221e"
      out = out .. char
      if char == '\\' then 
	out = out .. jsonF:read(1)
      end
    end
    return out
  end

  -- Grab a number from JSON input
  -- We have to give this function the first digit of the number we see
  function getNumber(jsonF, char)
    local out = ""
    while char do
      if not char then 
        globalError = "Premature end of file"
        return nil 
      end
      if isWhitespace(char) or char == "," or char == "]" or char == "}" then 
        return tonumber(out), char
      elseif char:find("[0-9%.Ee%+%-]") then
        out = out .. char
      else
        globalError = "Invalid char in number"
        return nil
      end
      char = jsonF:read(1)
    end
    return tonumber(out)
  end

  -- Grab a word from JSON input
  -- We have to give this function the first letter of the word we see
  function getWord(jsonF, char)
    local out = ""
    while char do
      if not char then 
        globalError = "Premature end of file"
        return nil 
      end
      if isWhitespace(char) or char == "," or char == ":" then 
        return out
      elseif char:find("[A-Za-z0-9%_]") then
        out = out .. char
      elseif char == "}" or char == "]" then
        return out, char
      else
        globalError = "Invalid char in word"
        return nil
      end
      char = jsonF:read(1)
    end
    return out
  end

  -- Input: jsonF: File we are reading JSON from
  --        mode: one char string: "{" if object, "[" if array
  function toJsonRecurse(jsonF, mode,depth)
    if depth > 100 then return nil end
    if not jsonF then return {_ERROR = "Invalid jsonF for toJsonRecurse"} end
    if mode ~= "[" and mode ~= "{" then return {_ERROR = "Invalid mode"} end
    local out = {}
    local char = true
    local name = nil
    local value = nil
    if mode == "[" then name = 1 end
    while char do
      char = jsonF:read(1)
      if not char then return out end
      if mode == "{" and char == '"' and not name then 
        name = getString(jsonF)
        if not name then return nil end
      -- Not JSON compliant, but allow bare words for object keys
      elseif mode == "{" and char:find("[A-Za-z]") and not name then
        name = getWord(jsonF, char)
        if not name then return nil end
      elseif char == '"' then 
        value = getString(jsonF)
        if not value then return nil end
        out[name] = value
        if mode == "{" then
          name = nil
        else
          name = name + 1
        end
      elseif char == "t" or char == "f" or char == "n" then
        value, next = getWord(jsonF, char)
	-- You do not want to make JSON "null" Lua nil
	if value == "null" then value = nullvalue
	elseif value == "false" then value = false
	elseif value == "true" then value = true
	else
	  globalError = "Error parsing word"
	  return nil
	end
        out[name] = value
        if mode == "{" then
          name = nil
        else
          name = name + 1
        end
        if next == "}" or next == "]" then return out end
      elseif char:find("[0-9%-]") and name then
        value, next = getNumber(jsonF, char)
        if not value then
          globalError = "Error parsing number"
	  return nil
        end
        out[name] = value
        if mode == "{" then
          name = nil
        else
          name = name + 1
        end
        if next == "}" or next == "]" then return out end
      elseif char == "[" or char == "{" then
        value = toJsonRecurse(jsonF, char,depth + 1)
        if not value then
          if not globalError then 
	    globalError = "Error parsing sub-object"
	  end
	  return nil
        end
        out[name] = value
        if mode == "{" then
          name = nil
        else
          name = name + 1
        end
      elseif char == "#" then 
        char = processComment(jsonF) 
      elseif char == "," or isWhitespace(char) then 
        char = true 
      -- It actually makes the parser simpler to treat ':' as whitespace
      -- We already know if we're processing a key or value
      elseif char == ":" then 
        char = true 
      elseif char == "}" or char == "]" then
        return out
      else
        globalError = "JSON syntax error"
        return nil
      end
    end
    return out
  end
  
  -- Input: Name of file with JSON to open
  -- Output: Table with parsed JSON
  function toJson(filename)
    if type(filename) ~= "string" then
      return {_ERROR = "Invalid filename", line = -1}
    end
    jsonF = io.open(filename)
    if jsonF == nil then
      return {_ERROR = "Error opening file", line = -1}
    end
    char = true
    while char do
      char = jsonF:read(1)
      if not char then break end
      if char == "[" or char == "{" then 
        out = toJsonRecurse(jsonF, char, 1) 
        jsonF:close()
        if not out then return {_ERROR = globalError, line = lineNumber} end
        return out
      elseif char == "#" then -- Comments, non-JSON extension
        char = processComment(jsonF)
        if not char then break end
      elseif isWhitespace(char) then
        char = true 
      -- It's actually simpler for this parser to treat ',' like whitespace
      elseif char == "," then
        char = true
      else
        jsonF:close()
        return {_ERROR = "JSON syntax error", line = lineNumber}
      end 
    end
    jsonF:close()
    return {}
  end

  return toJson(filename)
end
