-- This is a configuration file for mmLunacyDNS
-- This file is a Lua script
-- The API is unstable and subject to change

-- This is the IP the server will have
bindIp = "127.0.0.1"

-- Here is the API:
-- print (and all top-level methods) is disabled.  However, all
-- "string", "math", and a subset of "bit32" methods are present and 
-- work.  To make up for the lack of "print", we have "mmDNS.log()" which
-- takes a single string as an argument and logs the string in question.
--
-- Every time mmLunacyDNS receives a DNS query, processQuery is called
-- Its input is a table with two members:
-- mmQtype: A numeric field with the DNS query type (1 is "A", i.e. IPv4
-- IP address)
-- mmQuery: A string with the actual name being queried for, with a dot
-- at the end, such as "samiam.org." or "caulixtla.com."
--
-- Its output is a table with two members:
-- mm1Type: The type of data we return over DNS.  Right now, this must be 
-- "A" or the response is ignored
-- mm1Data: The IP, as a string in dotted decimal form
function processQuery(mmAll)
  local query = mmAll["mmQuery"]
  if not query then
    mmDNS.log("No query received")
    return nil
  end
  if mmAll["mmQtype"] ~= 1 then 
    mmDNS.log("Ignoring non-A query for " .. query)
    return {mm1Type = "ignoreMe"}
  end 
  mmDNS.log("Query: " .. query)
  out = {mm1Type = "A"}
  if(string.match(query,'.com.$')) then
    out.mm1Data = "10.1.1.1"
  else
    out.mm1Data = "10.2.2.2"
  end
  return out
end
