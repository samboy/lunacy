/* Combined form of old Lua Extension Proposal.  See COPYING.ex for MIT
 * license */
#ifdef MINGW
/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar at gmail com >
 */
#include <windows.h>

typedef struct DIR_tag DIR;
DIR *opendir(const char *name);
const WIN32_FIND_DATA *readdir(DIR *pi);
void closedir(DIR *pi);
/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar at gmail com >
 */
#include <windows.h>
#include <stdlib.h>
#include "dirent.h"

struct DIR_tag {
  int first;
  HANDLE hf;
  WIN32_FIND_DATA fd;
};

static char *mkpattern(const char *name)
{
  static const char suffix[] = "\\*";
  size_t len = strlen(name);
  char *pattern = malloc(len + sizeof suffix);
  if (pattern)
    strcpy(memcpy(pattern, name, len) + len, suffix);
  return pattern;
}

DIR *opendir(const char *name)
{
  DIR *pi = malloc(sizeof *pi);
  char *pattern = mkpattern(name);
  if (!pi || !pattern
      || INVALID_HANDLE_VALUE == (pi->hf = FindFirstFile(pattern, &pi->fd))) {
    free(pi);
    pi = 0;
  }
  else {
    pi->first = 1;
  }
  free(pattern);
  return pi;
}

static int isdotfile(const char *name)
{
  return name[0] == '.' && (name[1] == 0
    || (name[1] == '.' && name[2] == 0));
}

const WIN32_FIND_DATA *readdir(DIR *pi)
{
  switch (pi->first) do {
  default:
    if (!FindNextFile(pi->hf, &pi->fd)) {
      FindClose(pi->hf);
      pi->hf = INVALID_HANDLE_VALUE;
      return 0;
    }
  case 1: pi->first = 0;
  } while (isdotfile(pi->fd.cFileName));
  return &pi->fd;
}

void closedir(DIR *pi)
{
  if (pi->hf != INVALID_HANDLE_VALUE) {
      FindClose(pi->hf);
      pi->hf = INVALID_HANDLE_VALUE;
  }
  free(pi);
}
/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar at gmail com >
 */
#ifndef pusherror_h
#define pusherror_h

#include <windows.h>
#include "lua.h"

int windows_pusherror(lua_State *L, DWORD error, int nresults);
#define windows_pushlasterror(L) windows_pusherror(L, GetLastError(), -2)

#endif/*pusherror_h*/
/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar at gmail com >
 */
#include <stdio.h> /* sprintf() */
#include <ctype.h>
#define WIN32_LEAN_AND_MEAN 1
#define NOGDI 1
#include <windows.h>

#include "pusherror.h"

/* Push nil, followed by the Windows error message corresponding to
 * the error number, or a string giving the error value in decimal if
 * no error message is found.  If nresults is -2, always push nil and
 * the error message and return 2 even if error is NO_ERROR.  If
 * nresults is -1 and error is NO_ERROR, then push true and return 1.
 * Otherwise, if error is NO_ERROR, return nresults.
 */
int windows_pusherror(lua_State *L, DWORD error, int nresults)
{
  if (error != NO_ERROR || nresults == -2) {
    char buffer[1024];
    size_t len = sprintf(buffer, "%lu (0x%lX): ", error, error);
    size_t res = FormatMessage(
      FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
      0, error, 0, buffer + len, sizeof buffer - len, 0);
    if (res) {
      len += res;
      while (len > 0 && isspace(buffer[len - 1]))
        len--;
    }
    else {
      len += sprintf(buffer + len, "<error string not available>");
    }
    lua_pushnil(L);
    lua_pushlstring(L, buffer, len);
    nresults = 2;
  }
  else if (nresults < 0) {
    lua_pushboolean(L, 1);
    nresults = 1;
  }
  return nresults;
}
/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar at gmail com >
 */
#ifndef SPAWN_H
#define SPAWN_H

#include <windows.h>
#include "lua.h"

#define PROCESS_HANDLE "process"
struct process;
struct spawn_params;

struct spawn_params *spawn_param_init(lua_State *L);
void spawn_param_filename(struct spawn_params *p);
void spawn_param_args(struct spawn_params *p);
void spawn_param_env(struct spawn_params *p);
void spawn_param_redirect(
  struct spawn_params *p,
  const char *stdname,
  HANDLE h);
int spawn_param_execute(struct spawn_params *p);

int process_wait(lua_State *L);
int process_tostring(lua_State *L);

#endif/*SPAWN_H*/
/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar at gmail com >
 */
#include <stdlib.h>
#include <windows.h>
#include <io.h>

#include "lua.h"
#include "lauxlib.h"

#include "spawn.h"
#include "pusherror.h"

#define debug(...) /* fprintf(stderr, __VA_ARGS__) */
#define debug_stack(L) /* #include "../lds.c" */

#define file_handle(fp) (HANDLE)_get_osfhandle(fileno(fp))

struct spawn_params {
  lua_State *L;
  const char *cmdline;
  const char *environment;
  STARTUPINFO si;
};

/* quotes and adds argument string to b */
static int add_argument(luaL_Buffer *b, const char *s) {
  int oddbs = 0;
  luaL_addchar(b, '"');
  for (; *s; s++) {
    switch (*s) {
    case '\\':
      luaL_addchar(b, '\\');
      oddbs = !oddbs;
      break;
    case '"':
      luaL_addchar(b, '\\');
      oddbs = 0;
      break;
    default:
      oddbs = 0;
      break;
    }
    luaL_addchar(b, *s);
  }
  luaL_addchar(b, '"');
  return oddbs;
}

struct spawn_params *spawn_param_init(lua_State *L)
{
  static const STARTUPINFO si = {sizeof si};
  struct spawn_params *p = lua_newuserdata(L, sizeof *p);
  p->L = L;
  p->cmdline = p->environment = 0;
  p->si = si;
  return p;
}

/* cmd ... -- cmd ... */
void spawn_param_filename(struct spawn_params *p)
{
  lua_State *L = p->L;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  if (add_argument(&b, lua_tostring(L, 1))) {
    luaL_error(L, "argument ends in odd number of backslashes");
    return;
  }
  luaL_pushresult(&b);
  lua_replace(L, 1);
  p->cmdline = lua_tostring(L, 1);
}

/* cmd ... argtab -- cmdline ... */
void spawn_param_args(struct spawn_params *p)
{
  lua_State *L = p->L;
  int argtab = lua_gettop(L);
  size_t i, n = lua_objlen(L, argtab);
  luaL_Buffer b;
  debug("spawn_param_args:"); debug_stack(L);
  lua_pushnil(L);                 /* cmd opts ... argtab nil */
  luaL_buffinit(L, &b);           /* cmd opts ... argtab nil b... */
  lua_pushvalue(L, 1);            /* cmd opts ... argtab nil b... cmd */
  luaL_addvalue(&b);              /* cmd opts ... argtab nil b... */
  /* concatenate the arg array to a string */
  for (i = 1; i <= n; i++) {
    const char *s;
    lua_rawgeti(L, argtab, i);    /* cmd opts ... argtab nil b... arg */
    lua_replace(L, argtab + 1);   /* cmd opts ... argtab arg b... */
    luaL_addchar(&b, ' ');
    /* XXX checkstring is confusing here */
    s = lua_tostring(L, argtab + 1);
    if (!s) {
      luaL_error(L, "expected string for argument %d, got %s",
                 i, lua_typename(L, lua_type(L, argtab + 1)));
      return;
    }
    add_argument(&b, luaL_checkstring(L, argtab + 1));
  }
  luaL_pushresult(&b);            /* cmd opts ... argtab arg cmdline */
  lua_replace(L, 1);              /* cmdline opts ... argtab arg */
  lua_pop(L, 2);                  /* cmdline opts ... */
  p->cmdline = lua_tostring(L, 1);
}

/* ... tab nil nil [...] -- ... tab envstr */
static char *add_env(lua_State *L, int tab, size_t where) {
  char *t;
  lua_checkstack(L, 2);
  lua_pushvalue(L, -2);
  if (lua_next(L, tab)) {
    size_t klen, vlen;
    const char *key = lua_tolstring(L, -2, &klen);
    const char *val = lua_tolstring(L, -1, &vlen);
    t = add_env(L, tab, where + klen + vlen + 2);
    memcpy(&t[where], key, klen);
    t[where += klen] = '=';
    memcpy(&t[where + 1], val, vlen + 1);
  }
  else {
    t = lua_newuserdata(L, where + 1);
    t[where] = '\0';
    lua_replace(L, tab + 1);
  }
  return t;
}

/* ... envtab -- ... envtab envstr */
void spawn_param_env(struct spawn_params *p)
{
  lua_State *L = p->L;
  int envtab = lua_gettop(L);
  lua_pushnil(L);
  lua_pushnil(L);
  p->environment = add_env(L, envtab, 0);
  lua_settop(L, envtab + 1);
}

void spawn_param_redirect(struct spawn_params *p, const char *stdname, HANDLE h)
{
  SetHandleInformation(h, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
  if (!(p->si.dwFlags & STARTF_USESTDHANDLES)) {
    p->si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
    p->si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    p->si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
    p->si.dwFlags |= STARTF_USESTDHANDLES;
  }
  switch (stdname[3]) {
  case 'i': p->si.hStdInput = h; break;
  case 'o': p->si.hStdOutput = h; break;
  case 'e': p->si.hStdError = h; break;
  }
}

struct process {
  int status;
  HANDLE hProcess;
  DWORD dwProcessId;
};

int spawn_param_execute(struct spawn_params *p)
{
  lua_State *L = p->L;
  char *c, *e;
  PROCESS_INFORMATION pi;
  BOOL ret;
  struct process *proc = lua_newuserdata(L, sizeof *proc);
  luaL_getmetatable(L, PROCESS_HANDLE);
  lua_setmetatable(L, -2);
  proc->status = -1;
  c = strdup(p->cmdline);
  e = (char *)p->environment; /* strdup(p->environment); */
  /* XXX does CreateProcess modify its environment argument? */
  ret = CreateProcess(0, c, 0, 0, TRUE, 0, e, 0, &p->si, &pi);
  /* if (e) free(e); */
  free(c);
  if (!ret)
    return windows_pushlasterror(L);
  proc->hProcess = pi.hProcess;
  proc->dwProcessId = pi.dwProcessId;
  return 1;
}

/* proc -- exitcode/nil error */
int process_wait(lua_State *L)
{
  struct process *p = luaL_checkudata(L, 1, PROCESS_HANDLE);
  if (p->status == -1) {
    DWORD exitcode;
    if (WAIT_FAILED == WaitForSingleObject(p->hProcess, INFINITE)
        || !GetExitCodeProcess(p->hProcess, &exitcode))
      return windows_pushlasterror(L);
    p->status = exitcode;
  }
  lua_pushnumber(L, p->status);
  return 1;
}

/* proc -- string */
int process_tostring(lua_State *L)
{
  struct process *p = luaL_checkudata(L, 1, PROCESS_HANDLE);
  char buf[40];
  lua_pushlstring(L, buf,
    sprintf(buf, "process (%lu, %s)", (unsigned long)p->dwProcessId,
      p->status==-1 ? "running" : "terminated"));
  return 1;
}
/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar at gmail com >
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <sys/locking.h>
#include "dirent.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define absindex(L,i) ((i)>0?(i):lua_gettop(L)+(i)+1)

#include "spawn.h"

#include "pusherror.h"
#define push_error(L) windows_pushlasterror(L)


/* name -- value/nil */
static int ex_getenv(lua_State *L)
{
  const char *nam = luaL_checkstring(L, 1);
  char sval[256], *val;
  size_t len = GetEnvironmentVariable(nam, val = sval, sizeof sval);
  if (sizeof sval < len)
    len = GetEnvironmentVariable(nam, val = lua_newuserdata(L, len), len);
  if (len == 0)
    return push_error(L);
  lua_pushlstring(L, val, len);
  return 1;
}

/* name value -- true/nil error
 * name nil -- true/nil error */
static int ex_setenv(lua_State *L)
{
  const char *nam = luaL_checkstring(L, 1);
  const char *val = lua_tostring(L, 2);
  if (!SetEnvironmentVariable(nam, val))
    return push_error(L);
  lua_pushboolean(L, 1);
  return 1;
}

/* -- environment-table */
static int ex_environ(lua_State *L)
{
  const char *nam, *val, *end;
  const char *envs = GetEnvironmentStrings();
  if (!envs) return push_error(L);
  lua_newtable(L);
  for (nam = envs; *nam; nam = end + 1) {
    end = strchr(val = strchr(nam, '=') + 1, '\0');
    lua_pushlstring(L, nam, val - nam - 1);
    lua_pushlstring(L, val, end - val);
    lua_settable(L, -3);
  }
  return 1;
}


/* -- pathname/nil error */
static int ex_currentdir(lua_State *L)
{
  char pathname[MAX_PATH + 1];
  size_t len = GetCurrentDirectory(sizeof pathname, pathname);
  if (len == 0) return push_error(L);
  lua_pushlstring(L, pathname, len);
  return 1;
}

/* pathname -- true/nil error */
static int ex_chdir(lua_State *L)
{
  const char *pathname = luaL_checkstring(L, 1);
  if (!SetCurrentDirectory(pathname))
    return push_error(L);
  lua_pushboolean(L, 1);
  return 1;
}

/* pathname -- true/nil error */
static int ex_mkdir(lua_State *L)
{
  const char *pathname = luaL_checkstring(L, 1);
  if (!CreateDirectory(pathname, 0))
    return push_error(L);
  lua_pushboolean(L, 1);
  return 1;
}

/* pathname -- true/nil error */
static int ex_remove(lua_State *L)
{
  const char *pathname = luaL_checkstring(L, 1);
  DWORD attr = GetFileAttributes(pathname);
  if (attr == (DWORD)-1
      || (attr & FILE_ATTRIBUTE_DIRECTORY
          ? !RemoveDirectory(pathname)
          : !DeleteFile(pathname)))
    return push_error(L);
  lua_pushboolean(L, 1);
  return 1;
}


static FILE *check_file(lua_State *L, int idx, const char *argname)
{
  FILE **pf;
  if (idx > 0) pf = luaL_checkudata(L, idx, LUA_FILEHANDLE);
  else {
    idx = absindex(L, idx);
    pf = lua_touserdata(L, idx);
    luaL_getmetatable(L, LUA_FILEHANDLE);
    if (!pf || !lua_getmetatable(L, idx) || !lua_rawequal(L, -1, -2))
      luaL_error(L, "bad %s option (%s expected, got %s)",
                 argname, LUA_FILEHANDLE, luaL_typename(L, idx));
    lua_pop(L, 2);
  }
  if (!*pf) return luaL_error(L, "attempt to use a closed file"), NULL;
  return *pf;
}

static FILE **new_file(lua_State *L, HANDLE h, int dmode, const char *mode)
{
  FILE **pf = lua_newuserdata(L, sizeof *pf);
  *pf = 0;
  luaL_getmetatable(L, LUA_FILEHANDLE);
  lua_setmetatable(L, -2);
  *pf = _fdopen(_open_osfhandle((long)h, dmode), mode);
  return pf;
}

#define file_handle(fp) (HANDLE)_get_osfhandle(_fileno(fp))

static lua_Number qword_to_number(DWORD hi, DWORD lo)
{
  /* lua_Number must be floating-point or as large or larger than
   * two DWORDs in order to be considered adequate for representing
   * large file sizes */
  lua_assert(hi == 0
         || (lua_Number)0.5 > 0
         || sizeof(lua_Number) > 2 * sizeof(DWORD)
         || !"lua_Number cannot adequately represent large file sizes" );
  return hi * (1.0 + (DWORD)-1) + lo;
}

static lua_Number get_file_size(const char *name)
{
  HANDLE h = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, 0,
                        OPEN_EXISTING, 0, 0);
  DWORD lo, hi;
  lua_Number size;
  if (h == INVALID_HANDLE_VALUE)
    size = 0;
  else {
    lo = GetFileSize(h, &hi);
    if (lo == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
      size = 0;
    else
      size = qword_to_number(hi, lo);
    CloseHandle(h);
  }
  return size;
}

#define new_dirent(L) lua_newtable(L)

/* pathname/file [entry] -- entry */
static int ex_dirent(lua_State *L)
{
  int isdir;
  lua_Number size;
  DWORD attr;
  switch (lua_type(L, 1)) {
  default: return luaL_typerror(L, 1, "file or pathname");
  case LUA_TSTRING: {
    const char *name = lua_tostring(L, 1);
    attr = GetFileAttributes(name);
    if (attr == (DWORD)-1)
      return push_error(L);
    isdir = attr & FILE_ATTRIBUTE_DIRECTORY;
    if (isdir)
      size = 0;
    else
      size = get_file_size(name);
    } break;
  case LUA_TUSERDATA: {
    FILE *f = check_file(L, 1, NULL);
    BY_HANDLE_FILE_INFORMATION info;
    if (!GetFileInformationByHandle(file_handle(f), &info))
      return push_error(L);
    attr = info.dwFileAttributes;
    isdir = attr & FILE_ATTRIBUTE_DIRECTORY;
    size = qword_to_number(info.nFileSizeHigh, info.nFileSizeLow);
    } break;
  }
  if (lua_type(L, 2) != LUA_TTABLE) {
    lua_settop(L, 1);
    new_dirent(L);
  }
  else {
    lua_settop(L, 2);
  }
  if (isdir)
    lua_pushliteral(L, "directory");
  else
    lua_pushliteral(L, "file");
  lua_setfield(L, 2, "type");
  lua_pushnumber(L, size);
  lua_setfield(L, 2, "size");
  return 1;
}

#define DIR_HANDLE "DIR*"

/* ...diriter... -- ...diriter... pathname */
static int diriter_getpathname(lua_State *L, int index)
{
  lua_pushvalue(L, index);
  lua_gettable(L, LUA_REGISTRYINDEX);
  return 1;
}

/* ...diriter... pathname -- ...diriter... */
static int diriter_setpathname(lua_State *L, int index)
{
  size_t len;
  const char *path = lua_tolstring(L, -1, &len);
  if (path && path[len - 1] != *LUA_DIRSEP) {
    lua_pushliteral(L, LUA_DIRSEP);
    lua_concat(L, 2);
  }
  lua_pushvalue(L, index);              /* ... pathname diriter */
  lua_insert(L, -2);                    /* ... diriter pathname */
  lua_settable(L, LUA_REGISTRYINDEX);   /* ... */
  return 0;
}

/* diriter -- diriter */
static int diriter_close(lua_State *L)
{
  DIR **pd = lua_touserdata(L, 1);
  if (*pd) {
    closedir(*pd);
    *pd = 0;
  }
  lua_pushnil(L);
  diriter_setpathname(L, 1);
  return 0;
}

static int isdotfile(const char *name)
{
  return name[0] == '.' && (name[1] == '\0'
         || (name[1] == '.' && name[2] == '\0'));
}

/* pathname -- iter state nil */
/* diriter ... -- entry */
static int ex_dir(lua_State *L)
{
  const char *pathname;
  DIR **pd;
  const WIN32_FIND_DATA *d;
  switch (lua_type(L, 1)) {
  default: return luaL_typerror(L, 1, "pathname");
  case LUA_TNONE:
	lua_pushliteral(L, ".");
  case LUA_TSTRING:
    pathname = lua_tostring(L, 1);
    lua_pushcfunction(L, ex_dir);       /* pathname ... iter */
    pd = lua_newuserdata(L, sizeof *pd);/* pathname ... iter state */
    *pd = opendir(pathname);
    if (!*pd) return push_error(L);
    luaL_getmetatable(L, DIR_HANDLE);   /* pathname ... iter state M */
    lua_setmetatable(L, -2);            /* pathname ... iter state */
    lua_pushvalue(L, 1);                /* pathname ... iter state pathname */
    diriter_setpathname(L, -2);         /* pathname ... iter state */
    return 2;
  case LUA_TUSERDATA:
    pd = luaL_checkudata(L, 1, DIR_HANDLE);
    do d = readdir(*pd);
    while (d && isdotfile(d->cFileName));
    if (!d) return push_error(L);
    new_dirent(L);                      /* diriter ... entry */
    diriter_getpathname(L, 1);          /* diriter ... entry dir */
    lua_pushstring(L, d->cFileName);    /* diriter ... entry dir name */
    lua_pushvalue(L, -1);               /* diriter ... entry dir name name */
    lua_setfield(L, -4, "name");        /* diriter ... entry dir name */
    lua_concat(L, 2);                   /* diriter ... entry fullpath */
    lua_replace(L, 1);                  /* fullpath ... entry */
    lua_replace(L, 2);                  /* fullpath entry ... */
    return ex_dirent(L);
  }
  /*NOTREACHED*/
}


static int file_lock(lua_State *L,
                     FILE *f, const char *mode, long offset, long length)
{
  static const ULARGE_INTEGER zero_len;
  static const OVERLAPPED zero_ov;
  HANDLE h = file_handle(f);
  ULARGE_INTEGER len = zero_len;
  OVERLAPPED ov = zero_ov;
  DWORD flags = 0;
  BOOL ret;
  if (length) len.LowPart = length;
  else len.LowPart = len.HighPart = -1;
  ov.Offset = offset;
  switch (*mode) {
    case 'w':
      flags = LOCKFILE_EXCLUSIVE_LOCK;
      /*FALLTHRU*/
    case 'r':
      flags |= LOCKFILE_FAIL_IMMEDIATELY;
      ret = LockFileEx(h, flags, 0, len.LowPart, len.HighPart, &ov);
      break;
    case 'u':
      ret = UnlockFileEx(h, 0, len.LowPart, len.HighPart, &ov);
      break;
    default:
      return luaL_error(L, "invalid mode");
  }
  if (!ret)
    return push_error(L);
  /* return the file */
  lua_settop(L, 1);
  return 1;
}

static const char *opt_mode(lua_State *L, int *pidx)
{
  if (lua_type(L, *pidx) != LUA_TSTRING)
    return "u";
  return lua_tostring(L, (*pidx)++);
}

/* file [mode] [offset [length]] -- file/nil error */
static int ex_lock(lua_State *L)
{
  FILE *f = check_file(L, 1, NULL);
  int argi = 2;
  const char *mode = opt_mode(L, &argi);
  long offset = luaL_optnumber(L, argi, 0);
  long length = luaL_optnumber(L, argi + 1, 0);
  return file_lock(L, f, mode, offset, length);
}

/* -- in out/nil error */
static int ex_pipe(lua_State *L)
{
  HANDLE ph[2];
  if (!CreatePipe(ph + 0, ph + 1, 0, 0))
    return push_error(L);
  SetHandleInformation(ph[0], HANDLE_FLAG_INHERIT, 0);
  SetHandleInformation(ph[1], HANDLE_FLAG_INHERIT, 0);
  new_file(L, ph[0], _O_RDONLY, "r");
  new_file(L, ph[1], _O_WRONLY, "w");
  return 2;
}


/* seconds --
 * interval units -- */
static int ex_sleep(lua_State *L)
{
  lua_Number interval = luaL_checknumber(L, 1);
  lua_Number units = luaL_optnumber(L, 2, 1);
  Sleep(1e3 * interval / units);
  return 0;
}


static void get_redirect(lua_State *L,
                         int idx, const char *stdname, struct spawn_params *p)
{
  lua_getfield(L, idx, stdname);
  if (!lua_isnil(L, -1))
    spawn_param_redirect(p, stdname, file_handle(check_file(L, -1, stdname)));
  lua_pop(L, 1);
}

/* filename [args-opts] -- proc/nil error */
/* args-opts -- proc/nil error */
static int ex_spawn(lua_State *L)
{
  struct spawn_params *params;
  int have_options;
  switch (lua_type(L, 1)) {
  default: return luaL_typerror(L, 1, "string or table");
  case LUA_TSTRING:
    switch (lua_type(L, 2)) {
    default: return luaL_typerror(L, 2, "table");
    case LUA_TNONE: have_options = 0; break;
    case LUA_TTABLE: have_options = 1; break;
    }
    break;
  case LUA_TTABLE:
    have_options = 1;
    lua_getfield(L, 1, "command");      /* opts ... cmd */
    if (!lua_isnil(L, -1)) {
      /* convert {command=command,arg1,...} to command {arg1,...} */
      lua_insert(L, 1);                 /* cmd opts ... */
    }
    else {
      /* convert {arg0,arg1,...} to arg0 {arg1,...} */
      size_t i, n = lua_objlen(L, 1);
      lua_rawgeti(L, 1, 1);             /* opts ... nil cmd */
      lua_insert(L, 1);                 /* cmd opts ... nil */
      for (i = 2; i <= n; i++) {
        lua_rawgeti(L, 2, i);           /* cmd opts ... nil argi */
        lua_rawseti(L, 2, i - 1);       /* cmd opts ... nil */
      }
      lua_rawseti(L, 2, n);             /* cmd opts ... */
    }
    if (lua_type(L, 1) != LUA_TSTRING)
      return luaL_error(L, "bad command option (string expected, got %s)",
                        luaL_typename(L, 1));
    break;
  }
  params = spawn_param_init(L);
  /* get filename to execute */
  spawn_param_filename(params);
  /* get arguments, environment, and redirections */
  if (have_options) {
    lua_getfield(L, 2, "args");         /* cmd opts ... argtab */
    switch (lua_type(L, -1)) {
    default:
      return luaL_error(L, "bad args option (table expected, got %s)",
                        luaL_typename(L, -1));
    case LUA_TNIL:
      lua_pop(L, 1);                    /* cmd opts ... */
      lua_pushvalue(L, 2);              /* cmd opts ... opts */
      if (0) /*FALLTHRU*/
    case LUA_TTABLE:
      if (lua_objlen(L, 2) > 0)
        return
          luaL_error(L, "cannot specify both the args option and array values");
      spawn_param_args(params);         /* cmd opts ... */
      break;
    }
    lua_getfield(L, 2, "env");          /* cmd opts ... envtab */
    switch (lua_type(L, -1)) {
    default:
      return luaL_error(L, "bad env option (table expected, got %s)",
                        luaL_typename(L, -1));
    case LUA_TNIL:
      break;
    case LUA_TTABLE:
      spawn_param_env(params);          /* cmd opts ... */
      break;
    }
    get_redirect(L, 2, "stdin", params);    /* cmd opts ... */
    get_redirect(L, 2, "stdout", params);   /* cmd opts ... */
    get_redirect(L, 2, "stderr", params);   /* cmd opts ... */
  }
  return spawn_param_execute(params);   /* proc/nil error */
}


/* register functions from 'lib' in table 'to' by copying existing
 * closures from table 'from' or by creating new closures */
static void copyfields(lua_State *L, const luaL_reg *l, int from, int to)
{
  for (to = absindex(L, to); l->name; l++) {
    lua_getfield(L, from, l->name);
    if (lua_isnil(L, -1)) {
      lua_pop(L, 1);
      lua_pushcfunction(L, l->func);
    }
    lua_setfield(L, to, l->name);
  }
}

int luaopen_ex(lua_State *L)
{
  const char *name = lua_tostring(L, 1);
  int ex;
  const luaL_reg ex_iolib[] = {
    {"pipe",       ex_pipe},
#define ex_iofile_methods (ex_iolib + 1)
    {"lock",       ex_lock},
    {"unlock",     ex_lock},
    {0,0} };
  const luaL_reg ex_oslib[] = {
    /* environment */
    {"getenv",     ex_getenv},
    {"setenv",     ex_setenv},
    {"environ",    ex_environ},
    /* file system */
    {"currentdir", ex_currentdir},
    {"chdir",      ex_chdir},
    {"mkdir",      ex_mkdir},
    {"remove",     ex_remove},
    {"dir",        ex_dir},
    {"dirent",     ex_dirent},
    /* process control */
    {"sleep",      ex_sleep},
    {"spawn",      ex_spawn},
    {0,0} };
  const luaL_reg ex_diriter_methods[] = {
    {"__gc",       diriter_close},
    {0,0} };
  const luaL_reg ex_process_methods[] = {
    {"__tostring", process_tostring},
#define ex_process_functions (ex_process_methods + 1)
    {"wait",       process_wait},
    {0,0} };
  /* diriter metatable */
  luaL_newmetatable(L, DIR_HANDLE);           /* . D */
  luaL_register(L, 0, ex_diriter_methods);    /* . D */
  /* proc metatable */
  luaL_newmetatable(L, PROCESS_HANDLE);       /* . P */
  luaL_register(L, 0, ex_process_methods);    /* . P */
  lua_pushvalue(L, -1);                       /* . P P */
  lua_setfield(L, -2, "__index");             /* . P */
  /* make all functions available via ex. namespace */
  luaL_register(L, name, ex_oslib);           /* . P ex */
  luaL_register(L, 0, ex_iolib);
  copyfields(L, ex_process_functions, -2, -1);
  return 1;
}
#else // MINGW
/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2009 Mark Edgar < medgar123 at gmail com >
 */

#ifdef __APPLE__

#include <crt_externs.h>
#define environ (*_NSGetEnviron())

#else

extern char **environ;

#endif
/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar at gmail com >
 */
#ifndef SPAWN_H
#define SPAWN_H

#include <stdio.h>
#include "lua.h"

#define PROCESS_HANDLE "process"
struct process;
struct spawn_params;

struct spawn_params *spawn_param_init(lua_State *L);
void spawn_param_filename(struct spawn_params *p, const char *filename);
void spawn_param_args(struct spawn_params *p);
void spawn_param_env(struct spawn_params *p);
void spawn_param_redirect(struct spawn_params *p, const char *stdname, int fd);
int spawn_param_execute(struct spawn_params *p);

int process_wait(lua_State *L);
int process_tostring(lua_State *L);

#endif/*SPAWN_H*/
/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar at gmail com >
 */
#include <unistd.h>
#include <sys/wait.h>
#if MISSING_POSIX_SPAWN
#include "posix_spawn.h"
#else
#include <spawn.h>
#endif

#include "lua.h"
#include "lauxlib.h"


struct spawn_params {
  lua_State *L;
  const char *command, **argv, **envp;
  posix_spawn_file_actions_t redirect;
};

extern int push_error(lua_State *L);

struct spawn_params *spawn_param_init(lua_State *L)
{
  struct spawn_params *p = lua_newuserdata(L, sizeof *p);
  p->L = L;
  p->command = 0;
  p->argv = p->envp = 0;
  posix_spawn_file_actions_init(&p->redirect);
  return p;
}

void spawn_param_filename(struct spawn_params *p, const char *filename)
{
  p->command = filename;
}

/* Converts a Lua array of strings to a null-terminated array of char pointers.
 * Pops a (0-based) Lua array and replaces it with a userdatum which is the
 * null-terminated C array of char pointers.  The elements of this array point
 * to the strings in the Lua array.  These strings should be associated with
 * this userdatum via a weak table for GC purposes, but they are not here.
 * Therefore, any function which calls this must make sure that these strings
 * remain available until the userdatum is thrown away.
 */
/* ... array -- ... vector */
static const char **make_vector(lua_State *L)
{
  size_t i, n = lua_objlen(L, -1);
  const char **vec = lua_newuserdata(L, (n + 2) * sizeof *vec);
                                        /* ... arr vec */
  for (i = 0; i <= n; i++) {
    lua_rawgeti(L, -2, i);              /* ... arr vec elem */
    vec[i] = lua_tostring(L, -1);
    if (!vec[i] && i > 0) {
      luaL_error(L, "expected string for argument %d, got %s",
                 i, lua_typename(L, lua_type(L, -1)));
      return 0;
    }
    lua_pop(L, 1);                      /* ... arr vec */
  }
  vec[n + 1] = 0;
  lua_replace(L, -2);                   /* ... vector */
  return vec;
}

/* ... argtab -- ... argtab vector */
void spawn_param_args(struct spawn_params *p)
{
  const char **argv = make_vector(p->L);
  if (!argv[0]) argv[0] = p->command;
  p->argv = argv;
}

/* ... envtab -- ... envtab vector */
void spawn_param_env(struct spawn_params *p)
{
  lua_State *L = p->L;
  size_t i = 0;
  lua_newtable(L);                      /* ... envtab arr */
  lua_pushliteral(L, "=");              /* ... envtab arr "=" */
  lua_pushnil(L);                       /* ... envtab arr "=" nil */
  for (i = 0; lua_next(L, -4); i++) {   /* ... envtab arr "=" k v */
    if (!lua_tostring(L, -2)) {
      luaL_error(L, "expected string for environment variable name, got %s",
                 lua_typename(L, lua_type(L, -2)));
      return;
    }
    if (!lua_tostring(L, -1)) {
      luaL_error(L, "expected string for environment variable value, got %s",
                 lua_typename(L, lua_type(L, -1)));
      return;
    }
    lua_pushvalue(L, -2);               /* ... envtab arr "=" k v k */
    lua_pushvalue(L, -4);               /* ... envtab arr "=" k v k "=" */
    lua_pushvalue(L, -3);               /* ... envtab arr "=" k v k "=" v */
    lua_concat(L, 3);                   /* ... envtab arr "=" k v "k=v" */
    lua_rawseti(L, -5, i);              /* ... envtab arr "=" k v */
    lua_pop(L, 1);                      /* ... envtab arr "=" k */
  }                                     /* ... envtab arr "=" */
  lua_pop(L, 1);                        /* ... envtab arr */
  p->envp = make_vector(L);             /* ... envtab arr vector */
}

void spawn_param_redirect(struct spawn_params *p, const char *stdname, int fd)
{
  int d;
  switch (stdname[3]) {
  case 'i': d = STDIN_FILENO; break;
  case 'o': d = STDOUT_FILENO; break;
  case 'e': d = STDERR_FILENO; break;
  }
  posix_spawn_file_actions_adddup2(&p->redirect, fd, d);
}

struct process {
  int status;
  pid_t pid;
};

int spawn_param_execute(struct spawn_params *p)
{
  lua_State *L = p->L;
  int ret;
  struct process *proc;
  if (!p->argv) {
    p->argv = lua_newuserdata(L, 2 * sizeof *p->argv);
    p->argv[0] = p->command;
    p->argv[1] = 0;
  }
  if (!p->envp)
    p->envp = (const char **)environ;
  proc = lua_newuserdata(L, sizeof *proc);
  luaL_getmetatable(L, PROCESS_HANDLE);
  lua_setmetatable(L, -2);
  proc->status = -1;
  ret = posix_spawnp(&proc->pid, p->command, &p->redirect, 0,
                     (char *const *)p->argv, (char *const *)p->envp);
  posix_spawn_file_actions_destroy(&p->redirect);
  return ret != 0 ? push_error(L) : 1;
}

/* proc -- exitcode/nil error */
int process_wait(lua_State *L)
{
  struct process *p = luaL_checkudata(L, 1, PROCESS_HANDLE);
  if (p->status == -1) {
    int status;
    if (-1 == waitpid(p->pid, &status, 0))
      return push_error(L);
    p->status = WEXITSTATUS(status);
  }
  lua_pushnumber(L, p->status);
  return 1;
}

/* proc -- string */
int process_tostring(lua_State *L)
{
  struct process *p = luaL_checkudata(L, 1, PROCESS_HANDLE);
  char buf[40];
  lua_pushlstring(L, buf,
    sprintf(buf, "process (%lu, %s)", (unsigned long)p->pid,
      p->status==-1 ? "running" : "terminated"));
  return 1;
}
/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar123 at gmail com >
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define absindex(L,i) ((i)>0?(i):lua_gettop(L)+(i)+1)

/* -- nil error */
extern int push_error(lua_State *L)
{
  lua_pushnil(L);
  lua_pushstring(L, strerror(errno));
  return 2;
}


/* name -- value/nil */
static int ex_getenv(lua_State *L)
{
  const char *nam = luaL_checkstring(L, 1);
  char *val = getenv(nam);
  if (!val)
    return push_error(L);
  lua_pushstring(L, val);
  return 1;
}

/* name value -- true/nil error
 * name nil -- true/nil error */
static int ex_setenv(lua_State *L)
{
  const char *nam = luaL_checkstring(L, 1);
  const char *val = lua_tostring(L, 2);
  int err = val ? setenv(nam, val, 1) : unsetenv(nam);
  if (err == -1) return push_error(L);
  lua_pushboolean(L, 1);
  return 1;
}

/* -- environment-table */
static int ex_environ(lua_State *L)
{
  const char *nam, *val, *end;
  const char **env;
  lua_newtable(L);
  for (env = (const char **)environ; (nam = *env); env++) {
    end = strchr(val = strchr(nam, '=') + 1, '\0');
    lua_pushlstring(L, nam, val - nam - 1);
    lua_pushlstring(L, val, end - val);
    lua_settable(L, -3);
  }
  return 1;
}


/* -- pathname/nil error */
static int ex_currentdir(lua_State *L)
{
  char pathname[PATH_MAX + 1];
  if (!getcwd(pathname, sizeof pathname))
    return push_error(L);
  lua_pushstring(L, pathname);
  return 1;
}

/* pathname -- true/nil error */
static int ex_chdir(lua_State *L)
{
  const char *pathname = luaL_checkstring(L, 1);
  if (-1 == chdir(pathname))
    return push_error(L);
  lua_pushboolean(L, 1);
  return 1;
}

/* pathname -- true/nil error */
static int ex_mkdir(lua_State *L)
{
  const char *pathname = luaL_checkstring(L, 1);
  if (-1 == mkdir(pathname, 0777))
    return push_error(L);
  lua_pushboolean(L, 1);
  return 1;
}

/* Lua os.remove provides the correct semantics on POSIX systems */


static FILE *check_file(lua_State *L, int idx, const char *argname)
{
  FILE **pf;
  if (idx > 0) pf = luaL_checkudata(L, idx, LUA_FILEHANDLE);
  else {
    idx = absindex(L, idx);
    pf = lua_touserdata(L, idx);
    luaL_getmetatable(L, LUA_FILEHANDLE);
    if (!pf || !lua_getmetatable(L, idx) || !lua_rawequal(L, -1, -2))
      luaL_error(L, "bad %s option (%s expected, got %s)",
                 argname, LUA_FILEHANDLE, luaL_typename(L, idx));
    lua_pop(L, 2);
  }
  if (!*pf) return luaL_error(L, "attempt to use a closed file"), NULL;
  return *pf;
}

static FILE **new_file(lua_State *L, int fd, const char *mode)
{
  FILE **pf = lua_newuserdata(L, sizeof *pf);
  *pf = 0;
  luaL_getmetatable(L, LUA_FILEHANDLE);
  lua_setmetatable(L, -2);
  *pf = fdopen(fd, mode);
  return pf;
}


#define new_dirent(L) lua_newtable(L)

/* pathname/file [entry] -- entry */
static int ex_dirent(lua_State *L)
{
  struct stat st;
  switch (lua_type(L, 1)) {
  default: return luaL_typerror(L, 1, "file or pathname");
  case LUA_TSTRING: {
    const char *name = lua_tostring(L, 1);
    if (-1 == stat(name, &st))
      return push_error(L);
    } break;
  case LUA_TUSERDATA: {
    FILE *f = check_file(L, 1, NULL);
    if (-1 == fstat(fileno(f), &st))
      return push_error(L);
    } break;
  }
  if (lua_type(L, 2) != LUA_TTABLE) {
    lua_settop(L, 1);
    new_dirent(L);
  }
  else {
    lua_settop(L, 2);
  }
  if (S_ISDIR(st.st_mode))
    lua_pushliteral(L, "directory");
  else
    lua_pushliteral(L, "file");
  lua_setfield(L, 2, "type");
  lua_pushnumber(L, st.st_size);
  lua_setfield(L, 2, "size");
  return 1;
}

#define DIR_HANDLE "DIR*"

/* ...diriter... -- ...diriter... pathname */
static int diriter_getpathname(lua_State *L, int index)
{
  lua_pushvalue(L, index);
  lua_gettable(L, LUA_REGISTRYINDEX);
  return 1;
}

/* ...diriter... pathname -- ...diriter... */
static int diriter_setpathname(lua_State *L, int index)
{
  size_t len;
  const char *path = lua_tolstring(L, -1, &len);
  if (path && path[len - 1] != *LUA_DIRSEP) {
    lua_pushliteral(L, LUA_DIRSEP);
    lua_concat(L, 2);
  }
  lua_pushvalue(L, index);              /* ... pathname diriter */
  lua_insert(L, -2);                    /* ... diriter pathname */
  lua_settable(L, LUA_REGISTRYINDEX);   /* ... */
  return 0;
}

/* diriter -- diriter */
static int diriter_close(lua_State *L)
{
  DIR **pd = lua_touserdata(L, 1);
  if (*pd) {
    closedir(*pd);
    *pd = 0;
  }
  lua_pushnil(L);
  diriter_setpathname(L, 1);
  return 0;
}

static int isdotfile(const char *name)
{
  return name[0] == '.' && (name[1] == '\0'
         || (name[1] == '.' && name[2] == '\0'));
}

/* pathname -- iter state nil */
/* diriter ... -- entry */
static int ex_dir(lua_State *L)
{
  const char *pathname;
  DIR **pd;
  struct dirent *d;
  switch (lua_type(L, 1)) {
  default: return luaL_typerror(L, 1, "pathname");
  case LUA_TSTRING:
    pathname = lua_tostring(L, 1);
    lua_pushcfunction(L, ex_dir);       /* pathname ... iter */
    pd = lua_newuserdata(L, sizeof *pd);/* pathname ... iter state */
    *pd = opendir(pathname);
    if (!*pd) return push_error(L);
    luaL_getmetatable(L, DIR_HANDLE);   /* pathname ... iter state M */
    lua_setmetatable(L, -2);            /* pathname ... iter state */
    lua_pushvalue(L, 1);                /* pathname ... iter state pathname */
    diriter_setpathname(L, -2);         /* pathname ... iter state */
    return 2;
  case LUA_TUSERDATA:
    pd = luaL_checkudata(L, 1, DIR_HANDLE);
    do d = readdir(*pd);
    while (d && isdotfile(d->d_name));
    if (!d) { diriter_close(L); return push_error(L); }
    new_dirent(L);                      /* diriter ... entry */
    diriter_getpathname(L, 1);          /* diriter ... entry dir */
    lua_pushstring(L, d->d_name);       /* diriter ... entry dir name */
    lua_pushvalue(L, -1);               /* diriter ... entry dir name name */
    lua_setfield(L, -4, "name");        /* diriter ... entry dir name */
    lua_concat(L, 2);                   /* diriter ... entry fullpath */
    lua_replace(L, 1);                  /* fullpath ... entry */
    lua_replace(L, 2);                  /* fullpath entry ... */
    return ex_dirent(L);
  }
  /*NOTREACHED*/
}


static int file_lock(lua_State *L,
                     FILE *f, const char *mode, long offset, long length)
{
  struct flock k;
  switch (*mode) {
    case 'w': k.l_type = F_WRLCK; break;
    case 'r': k.l_type = F_RDLCK; break;
    case 'u': k.l_type = F_UNLCK; break;
    default: return luaL_error(L, "invalid mode");
  }
  k.l_whence = SEEK_SET;
  k.l_start = offset;
  k.l_len = length;
  if (-1 == fcntl(fileno(f), F_SETLK, &k))
    return push_error(L);
  /* return the file */
  lua_settop(L, 1);
  return 1;
}

static const char *opt_mode(lua_State *L, int *pidx)
{
  if (lua_type(L, *pidx) != LUA_TSTRING)
    return "u";
  return lua_tostring(L, (*pidx)++);
}

/* file [mode] [offset [length]] -- file/nil error */
static int ex_lock(lua_State *L)
{
  FILE *f = check_file(L, 1, NULL);
  int argi = 2;
  const char *mode = opt_mode(L, &argi);
  long offset = luaL_optnumber(L, argi, 0);
  long length = luaL_optnumber(L, argi + 1, 0);
  return file_lock(L, f, mode, offset, length);
}


static int closeonexec(int d)
{
  int fl = fcntl(d, F_GETFD);
  if (fl != -1)
    fl = fcntl(d, F_SETFD, fl | FD_CLOEXEC);
  return fl;
}

/* -- in out/nil error */
static int ex_pipe(lua_State *L)
{
  int fd[2];
  if (-1 == pipe(fd))
    return push_error(L);
  closeonexec(fd[0]);
  closeonexec(fd[1]);
  new_file(L, fd[0], "r");
  new_file(L, fd[1], "w");
  return 2;
}


/* seconds --
 * interval units -- */
static int ex_sleep(lua_State *L)
{
  lua_Number interval = luaL_checknumber(L, 1);
  lua_Number units = luaL_optnumber(L, 2, 1);
  usleep(1e6 * interval / units);
  return 0;
}


static void get_redirect(lua_State *L,
                         int idx, const char *stdname, struct spawn_params *p)
{
  lua_getfield(L, idx, stdname);
  if (!lua_isnil(L, -1))
    spawn_param_redirect(p, stdname, fileno(check_file(L, -1, stdname)));
  lua_pop(L, 1);
}

/* filename [args-opts] -- proc/nil error */
/* args-opts -- proc/nil error */
static int ex_spawn(lua_State *L)
{
  struct spawn_params *params;
  int have_options;
  switch (lua_type(L, 1)) {
  default: return luaL_typerror(L, 1, "string or table");
  case LUA_TSTRING:
    switch (lua_type(L, 2)) {
    default: return luaL_typerror(L, 2, "table");
    case LUA_TNONE: have_options = 0; break;
    case LUA_TTABLE: have_options = 1; break;
    }
    break;
  case LUA_TTABLE:
    have_options = 1;
    lua_getfield(L, 1, "command");      /* opts ... cmd */
    if (!lua_isnil(L, -1)) {
      /* convert {command=command,arg1,...} to command {arg1,...} */
      lua_insert(L, 1);                 /* cmd opts ... */
    }
    else {
      /* convert {arg0,arg1,...} to arg0 {arg1,...} */
      size_t i, n = lua_objlen(L, 1);
      lua_rawgeti(L, 1, 1);             /* opts ... nil cmd */
      lua_insert(L, 1);                 /* cmd opts ... nil */
      for (i = 2; i <= n; i++) {
        lua_rawgeti(L, 2, i);           /* cmd opts ... nil argi */
        lua_rawseti(L, 2, i - 1);       /* cmd opts ... nil */
      }
      lua_rawseti(L, 2, n);             /* cmd opts ... */
    }
    if (lua_type(L, 1) != LUA_TSTRING)
      return luaL_error(L, "bad command option (string expected, got %s)",
                        luaL_typename(L, 1));
    break;
  }
  params = spawn_param_init(L);
  /* get filename to execute */
  spawn_param_filename(params, lua_tostring(L, 1));
  /* get arguments, environment, and redirections */
  if (have_options) {
    lua_getfield(L, 2, "args");         /* cmd opts ... argtab */
    switch (lua_type(L, -1)) {
    default:
      return luaL_error(L, "bad args option (table expected, got %s)",
                        luaL_typename(L, -1));
    case LUA_TNIL:
      lua_pop(L, 1);                    /* cmd opts ... */
      lua_pushvalue(L, 2);              /* cmd opts ... opts */
      if (0) /*FALLTHRU*/
    case LUA_TTABLE:
      if (lua_objlen(L, 2) > 0)
        return
          luaL_error(L, "cannot specify both the args option and array values");
      spawn_param_args(params);         /* cmd opts ... */
      break;
    }
    lua_getfield(L, 2, "env");          /* cmd opts ... envtab */
    switch (lua_type(L, -1)) {
    default:
      return luaL_error(L, "bad env option (table expected, got %s)",
                        luaL_typename(L, -1));
    case LUA_TNIL:
      break;
    case LUA_TTABLE:
      spawn_param_env(params);          /* cmd opts ... */
      break;
    }
    get_redirect(L, 2, "stdin", params);    /* cmd opts ... */
    get_redirect(L, 2, "stdout", params);   /* cmd opts ... */
    get_redirect(L, 2, "stderr", params);   /* cmd opts ... */
  }
  return spawn_param_execute(params);   /* proc/nil error */
}


/* register functions from 'lib' in table 'to' by copying existing
 * closures from table 'from' or by creating new closures */
static void copyfields(lua_State *L, const luaL_reg *l, int from, int to)
{
  for (to = absindex(L, to); l->name; l++) {
    lua_getfield(L, from, l->name);
    if (lua_isnil(L, -1)) {
      lua_pop(L, 1);
      lua_pushcfunction(L, l->func);
    }
    lua_setfield(L, to, l->name);
  }
}

int luaopen_ex(lua_State *L)
{
  const char *name = lua_tostring(L, 1);
  int ex;
  const luaL_reg ex_iolib[] = {
    {"pipe",       ex_pipe},
#define ex_iofile_methods (ex_iolib + 1)
    {"lock",       ex_lock},
    {"unlock",     ex_lock},
    {0,0} };
  const luaL_reg ex_oslib[] = {
    /* environment */
    {"getenv",     ex_getenv},
    {"setenv",     ex_setenv},
    {"environ",    ex_environ},
    /* file system */
    {"currentdir", ex_currentdir},
    {"chdir",      ex_chdir},
    {"mkdir",      ex_mkdir},
    {"dir",        ex_dir},
    {"dirent",     ex_dirent},
    /* process control */
    {"sleep",      ex_sleep},
    {"spawn",      ex_spawn},
    {0,0} };
  const luaL_reg ex_diriter_methods[] = {
    {"__gc",       diriter_close},
    {0,0} };
  const luaL_reg ex_process_methods[] = {
    {"__tostring", process_tostring},
#define ex_process_functions (ex_process_methods + 1)
    {"wait",       process_wait},
    {0,0} };
  /* diriter metatable */
  luaL_newmetatable(L, DIR_HANDLE);           /* . D */
  luaL_register(L, 0, ex_diriter_methods);    /* . D */
  /* proc metatable */
  luaL_newmetatable(L, PROCESS_HANDLE);       /* . P */
  luaL_register(L, 0, ex_process_methods);    /* . P */
  lua_pushvalue(L, -1);                       /* . P P */
  lua_setfield(L, -2, "__index");             /* . P */
  /* make all functions available via ex. namespace */
  luaL_register(L, name, ex_oslib);           /* . P ex */
  luaL_register(L, 0, ex_iolib);
  copyfields(L, ex_process_functions, -2, -1);
  return 1;
}
#endif // MINGW
