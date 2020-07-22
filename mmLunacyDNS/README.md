# mmLunacyDNS

mmLunacyDNS is a wrapper which uses Lua to set up a simple DNS server.

It uses, as a Lua configuration file, `mmLunacyDNS.lua` by default.
It is possible to use a different config file by either:

* Renaming the `mmLunacyDNS` binary to another name.
* By specifying a config file with the `-f` option:

```
	mmLunacyDNS -f /etc/mmLunacyDNS.lua
```

Note that the configuration file *must* have the suffix `.lua`.  

The configuration file is read to get the IP to bind to and the
IP to return.

The script `compile.mmLunacyDNS.sh` compiles mmLunacyDNS and links it
to Lunacy (Lua).

mmLunacyDNS currently has no daemonization support.

The Lua configuration file is an unstable API and may change.

