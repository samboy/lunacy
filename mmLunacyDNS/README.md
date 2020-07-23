# mmLunacyDNS

mmLunacyDNS is a simple DNS server configured with Lunacy, my fork of
Lua 5.1 (the syntax is the same)

It uses, as a Lua configuration file, `mmLunacyDNS.lua` by default.
It is possible to use a different config file by either:

* Renaming the `mmLunacyDNS` binary to another name.
* By specifying a config file with the `-f` option:

```
	mmLunacyDNS -f /etc/mmLunacyDNS.lua
```

Note that the configuration file *must* have the suffix `.lua`.  

The configuration file is tells mmLunacyDNS the IP to bind to, and
has a Lua function which is called every time a DNS query is received.

The script `compile.mmLunacyDNS.sh` compiles mmLunacyDNS and links it
to Lunacy (Lua).

mmLunacyDNS currently has no daemonization support.

# Configuration file format

mmLunacyDNS uses a Lua script as a configuration file.

From that file, it gets the string bindIp, which is the IP 
mmLunacyDNS binds to.

Once it binds to the IP, every time mmLunacyDNS gets a query, it
runs the lua function processQuery, which takes as its input a
table with two members:

* `mmQuery`: This is the DNS name requested, in the form of a string
  like `caulixtla.com.` or `samiam.org.` (observe the dot at the end of 
  the mmQuery string).  If the string has anything besides an ASCII 
  letter, an ASCII number, the `-` character (dash), or the `_` 
  character (underline), the character will be a two-digit hexadecimal 
  number in brackets.  If we get the raw UTF-8 query `ñ.samiam.org` 
  (where the first character is a n with a tilde), mmQuery will look 
  like `{c3}{b1}.samiam.org.`.
* `mmQtype`: The is the numeric DNS query type requested.  This is a number
  between 0 and 65535, and corresponds to the DNS query type made.  A
  list of DNS query type numbers is available at
  https://www.iana.org/assignments/dns-parameters/dns-parameters.xhtml
  1 is "A", i.e. a request for an IPv4 IP address.

The processQuery function returns as its output a table with two
parameters:

* `mm1Type`: This right now has to be the string `A`, or the DNS query is
  ignored
* `mm1Data`: This is an IPv4 IP in dotted decimal format, e.g. `10.1.2.3`

# Examples

```lua
-- We bind to the IP 127.0.0.1 and always return "10.1.2.3" for every
-- DNS query we get
bindIp = "127.0.0.1"
function processQuery(mmAll)
  return {mm1Type = "A", mm1Data="10.1.2.3"}
end
```

Here is a more complex example, where *.com gets 10.1.1.1 and anything else
gets 10.1.2.3, non-A queries are ignored, and we log A queries:

```lua
bindIp = "127.0.0.1" -- We bind the server to the IP 127.0.0.1
function processQuery(mmAll) -- Called for every DNS query received
  if mmAll.mmQtype ~= 1 then -- If it is not an A (ipv4) query
    return {mm1Type = "ignoreMe"} -- Ignore the query
  end
  mmDNS.log("Got IPv4 query for " .. mmAll.mmQuery) -- Log the query
  if string.match(mmAll.mmQuery,'.com.$') then
    return {mm1Type = "A", mm1Data = "10.1.1.1"} -- Answer for anything.com
  end
  return {mm1Type = "A", mm1Data = "10.1.2.3"} -- Answer for anything not .com
end
```

# Sandboxing

mmLunacyDNS provides some level of protection from untrusted `.lua` 
configuration files.  It is run in sandbox mode: All top-level
functions (such as `require`) are not present.  In addition, libraries
which can affect other files (such as `io` or `lfs`) are not here.  
What *is* present are three libraries for manipulating data: `string`,
`math`, and a mostly complete implementation of `bit32`.

To make up for there not being a `print` method, `mmDNS.log` can
be called instead; note that `mmDNS.log` only takes a single string
argument.
