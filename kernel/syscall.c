#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

typedef struct {
  uint64 (*func)(void);
  const char *name;
  int args_count;
} syscall_info_t;

extern uint64 sys_fork(void);
extern uint64 sys_exit(void);
extern uint64 sys_wait(void);
extern uint64 sys_pipe(void);
extern uint64 sys_read(void);
extern uint64 sys_kill(void);
extern uint64 sys_exec(void);
extern uint64 sys_fstat(void);
extern uint64 sys_chdir(void);
extern uint64 sys_dup(void);
extern uint64 sys_getpid(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_sleep(void);
extern uint64 sys_uptime(void);
extern uint64 sys_open(void);
extern uint64 sys_write(void);
extern uint64 sys_mknod(void);
extern uint64 sys_unlink(void);
extern uint64 sys_link(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_close(void);
extern uint64 sys_trace(void);
extern uint64 sys_sysinfo(void);

static const syscall_info_t syscall_table[] = {
  [SYS_fork]    = { sys_fork,    "fork",    0 },
  [SYS_exit]    = { sys_exit,    "exit",    1 },
  [SYS_wait]    = { sys_wait,    "wait",    1 },
  [SYS_pipe]    = { sys_pipe,    "pipe",    1 },
  [SYS_read]    = { sys_read,    "read",    3 },
  [SYS_kill]    = { sys_kill,    "kill",    1 },
  [SYS_exec]    = { sys_exec,    "exec",    2 },
  [SYS_fstat]   = { sys_fstat,   "fstat",   2 },
  [SYS_chdir]   = { sys_chdir,   "chdir",   1 },
  [SYS_dup]     = { sys_dup,     "dup",     1 },
  [SYS_getpid]  = { sys_getpid,  "getpid",  0 },
  [SYS_sbrk]    = { sys_sbrk,    "sbrk",    1 },
  [SYS_sleep]   = { sys_sleep,   "sleep",   1 },
  [SYS_uptime]  = { sys_uptime,  "uptime",  0 },
  [SYS_open]    = { sys_open,    "open",    2 },
  [SYS_write]   = { sys_write,   "write",   3 },
  [SYS_mknod]   = { sys_mknod,   "mknod",   3 },
  [SYS_unlink]  = { sys_unlink,  "unlink",  1 },
  [SYS_link]    = { sys_link,    "link",    2 },
  [SYS_mkdir]   = { sys_mkdir,   "mkdir",   1 },
  [SYS_close]   = { sys_close,   "close",   1 },
  [SYS_trace]   = { sys_trace,   "trace",   1 },
  [SYS_sysinfo] = { sys_sysinfo, "sysinfo", 1 },
};

int
fetchaddr(uint64 addr, uint64 *ip)
{
  struct proc *p = myproc();
  if(addr >= p->sz || addr + sizeof(uint64) > p->sz || copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0)
    return -1;
  return 0;
}

int
fetchstr(uint64 addr, char *buf, int max)
{
  struct proc *p = myproc();
  if(copyinstr(p->pagetable, buf, addr, max) < 0)
    return -1;
  return strlen(buf);
}

static uint64
argraw(int n)
{
  struct proc *p = myproc();
  switch (n) {
    case 0: return p->trapframe->a0;
    case 1: return p->trapframe->a1;
    case 2: return p->trapframe->a2;
    case 3: return p->trapframe->a3;
    case 4: return p->trapframe->a4;
    case 5: return p->trapframe->a5;
  }
  panic("argraw");
  return -1;
}

void
argint(int n, int *ip)
{
  *ip = argraw(n);
}

int
argaddr(int n, uint64 *ip)
{
  *ip = argraw(n);
  return 0;
}

int
argstr(int n, char *buf, int max)
{
  uint64 addr;
  argaddr(n, &addr);
  return fetchstr(addr, buf, max);
}

static void
trace_syscall(struct proc *p, int num, uint64 first_arg_backup)
{
  printf("%d: syscall %s(", p->pid, syscall_table[num].name);
  int count = syscall_table[num].args_count;
  
  for (int i = 0; i < count; i++) {
    uint64 val;
    if (i == 0) val = first_arg_backup;
    else val = argraw(i);
    
    printf("%d%s", (int)val, (i < count - 1) ? ", " : "");
  }
  printf(") -> %d\n", (int)p->trapframe->a0);
}

void
syscall(void)
{
  struct proc *p = myproc();
  int num = p->trapframe->a7;
  uint64 orig_a0 = p->trapframe->a0;

  if(num > 0 && num < NELEM(syscall_table) && syscall_table[num].func) {
    p->trapframe->a0 = syscall_table[num].func();
    
    if ((1 << num) & p->tracemask) {
      trace_syscall(p, num, orig_a0);
    }
  } else {
    printf("%d %s: unknown sys call %d\n", p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}