-- This is a configuration file for mmLunacyDNS
-- This file is a Lua script
-- The API is unstable and subject to change

bindIp = "127.0.0.2"

-- Please be aware that this API is **unstable** and **WILL** change
-- in the next version of mmLunacyDNS!
function processQuery(query)
  return "10.1.2.3"
end
