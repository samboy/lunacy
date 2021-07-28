Here is a list of CVEs which pop up when I perform a NVD search against
Lua 5.1.

# CVE-2014-5461

[CVE-2014-5461](https://nvd.nist.gov/vuln/detail/CVE-2014-5461)

Patch:

```
From: Enrico Tassi <gareuselesinge@debian.org>
Date: Tue, 26 Aug 2014 16:20:55 +0200
Subject: Fix stack overflow in vararg functions

---
 src/ldo.c | 2 +-
 1 file changed, 1 insertion(+), 1 deletion(-)

diff --git a/src/ldo.c b/src/ldo.c
index d1bf786..30333bf 100644
--- a/src/ldo.c
+++ b/src/ldo.c
@@ -274,7 +274,7 @@ int luaD_precall (lua_State *L, StkId func, int nresults) {
     CallInfo *ci;
     StkId st, base;
     Proto *p = cl->p;
-    luaD_checkstack(L, p->maxstacksize);
+    luaD_checkstack(L, p->maxstacksize + p->numparams);
     func = restorestack(L, funcr);
     if (!p->is_vararg) {  /* no varargs? */
       base = func + 1;
```

Patched in commit 4de84e044c1219b06744bfc0d80f6b8568e58e9a for 
release 2021-07-28

Note: The exploit code at http://www.lua.org/bugs.html#5.2.2-1 does
*not* crash Lunacy 2021-03-22.

Exploit code:

```
function f(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10,
           p11, p12, p13, p14, p15, p16, p17, p18, p19, p20,
           p21, p22, p23, p24, p25, p26, p27, p28, p29, p30,
           p31, p32, p33, p34, p35, p36, p37, p38, p39, p40,
           p41, p42, p43, p44, p45, p46, p48, p49, p50, ...)
  local a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14
end

f()   -- crashes on some machines
```

# CVE-2020-15888

[CVE-2020-15888](https://nvd.nist.gov/vuln/detail/CVE-2020-15888)

https://github.com/lua/lua/commit/6298903e35217ab69c279056f925fb72900ce0b7

https://github.com/lua/lua/commit/eb41999461b6f428186c55abd95f4ce1a76217d5

As per https://ubuntu.com/security/CVE-2020-15888 this does not look
to affect earlier versions of Lua.  The patches will not patch against
Lunacy (Lua 5.1 fork); the relevant code has been completely rewritten.

# CVE-2020-15945

[CVE-2020-15945](https://nvd.nist.gov/vuln/detail/CVE-2020-15945)

https://github.com/lua/lua/commit/a2195644d89812e5b157ce7bac35543e06db05e3

As per https://ubuntu.com/security/CVE-2020-15945 this does not look to
affect earlier versions of Lua.  The patches will not patch against
Lunacy (Lua 5.1 fork); the relevant code has been completely rewritten.


