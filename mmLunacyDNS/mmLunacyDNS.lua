-- This is a configuration file for mmLunacyDNS
-- This file is a Lua script
-- The API is unstable and subject to change

bindIp = "127.0.0.2"

-- Please be aware that this API is **unstable** and **WILL** change
-- in the next version of mmLunacyDNS!
function processQuery(query)
  mmDNS.log("Query: " .. query) -- Log all queries
  out = {mm1Type = "A", mm1Data = "127.0.0.1"}
  if(string.match(query,'.com.$')) then
    out.mm1Data = "10.1.1.1"
  else
    out.mm1Data = "10.2.2.2"
  end
  return out
end
