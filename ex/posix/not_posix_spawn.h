/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar at gmail com >
 */
#include <sched.h>
#include <signal.h>
#include <sys/types.h>

#if __STDC_VERSION__ < 199901L
#define restrict
#endif

typedef void *not_posix_spawnattr_t;

enum {
  POSIX_SPAWN_RESETIDS,
  POSIX_SPAWN_SETPGROUP,
  POSIX_SPAWN_SETSCHEDPARAM,
  POSIX_SPAWN_SETSCHEDULER,
  POSIX_SPAWN_SETSIGDEF,
  POSIX_SPAWN_SETSIGMASK,
};

int not_posix_spawnattr_init(not_posix_spawnattr_t *attrp);
int not_posix_spawnattr_getflags(
  const not_posix_spawnattr_t *restrict attrp,
  short *restrict flags);
int not_posix_spawnattr_setflags(
  not_posix_spawnattr_t *attrp,
  short flags);
int not_posix_spawnattr_getpgroup(
  const not_posix_spawnattr_t *restrict attrp,
  pid_t *restrict pgroup);
int not_posix_spawnattr_setpgroup(
  not_posix_spawnattr_t *attrp,
  pid_t pgroup);
int not_posix_spawnattr_getschedparam(
  const not_posix_spawnattr_t *restrict attrp,
  struct sched_param *restrict schedparam);
int not_posix_spawnattr_setschedparam(
  not_posix_spawnattr_t *restrict attrp,
  const struct sched_param *restrict schedparam);
int not_posix_spawnattr_getschedpolicy(
  const not_posix_spawnattr_t *restrict attrp,
  int *restrict schedpolicy);
int not_posix_spawnattr_setschedpolicy(
  not_posix_spawnattr_t *attrp,
  int schedpolicy);
int not_posix_spawnattr_getsigdefault(
  const not_posix_spawnattr_t *restrict attrp,
  sigset_t *restrict sigdefault);
int not_posix_spawnattr_setsigdefault(
  not_posix_spawnattr_t *restrict attrp,
  const sigset_t *restrict sigdefault);
int not_posix_spawnattr_getsigmask(
  const not_posix_spawnattr_t *restrict attrp,
  sigset_t *restrict sigmask);
int not_posix_spawnattr_setsigmask(
  not_posix_spawnattr_t *restrict attrp,
  const sigset_t *restrict sigmask);
int not_posix_spawnattr_destroy(not_posix_spawnattr_t *attrp);

typedef struct not_posix_spawn_file_actions not_posix_spawn_file_actions_t;
struct not_posix_spawn_file_actions {
  int dups[3];
};

int not_posix_spawn_file_actions_init(not_posix_spawn_file_actions_t *file_actions);
int not_posix_spawn_file_actions_adddup2(
  not_posix_spawn_file_actions_t *file_actions,
  int filedes,
  int newfiledes);
int not_posix_spawn_file_actions_addclose(
  not_posix_spawn_file_actions_t *file_actions,
  int filedes);
int not_posix_spawn_file_actions_addopen(
  not_posix_spawn_file_actions_t *restrict file_actions,
  int filedes,
  const char *restrict path,
  int oflag,
  mode_t mode);
int not_posix_spawn_file_actions_destroy(not_posix_spawn_file_actions_t *file_actions);

int not_posix_spawn(
  pid_t *restrict,
  const char *restrict,
  const not_posix_spawn_file_actions_t *,
  const not_posix_spawnattr_t *restrict,
  char *const argv[restrict],
  char *const envp[restrict]);
int not_posix_spawnp(
  pid_t *restrict,
  const char *restrict,
  const not_posix_spawn_file_actions_t *,
  const not_posix_spawnattr_t *restrict,
  char *const argv[restrict],
  char *const envp[restrict]);
