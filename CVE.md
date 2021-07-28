There following CVEs (possible security holes)
may need to be patched against:

# CVE-2014-5461

[CVE-2014-5461](https://nvd.nist.gov/vuln/detail/CVE-2014-5461)

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

# CVE-2020-15888

[CVE-2020-15888](https://nvd.nist.gov/vuln/detail/CVE-2020-15888)

https://github.com/lua/lua/commit/6298903e35217ab69c279056f925fb72900ce0b7

https://github.com/lua/lua/commit/eb41999461b6f428186c55abd95f4ce1a76217d5

# CVE-2020-15945

[CVE-2020-15945](https://nvd.nist.gov/vuln/detail/CVE-2020-15945)

https://github.com/lua/lua/commit/a2195644d89812e5b157ce7bac35543e06db05e3
