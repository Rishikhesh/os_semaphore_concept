/*
Portable counting semaphore.

macOS declares sem_init() but doesn't implement it (it returns -1 / ENOSYS),
so unnamed POSIX semaphores silently do nothing there. On Apple we use
Grand Central Dispatch semaphores instead; everywhere else plain POSIX.
*/
#ifndef SEM_H
#define SEM_H

#ifdef __APPLE__
#include <dispatch/dispatch.h>
typedef dispatch_semaphore_t sem;
static inline void sem_make(sem *s, int value) { *s = dispatch_semaphore_create(value); }
static inline void sem_down(sem *s) { dispatch_semaphore_wait(*s, DISPATCH_TIME_FOREVER); }
static inline void sem_up(sem *s) { dispatch_semaphore_signal(*s); }
static inline void sem_free(sem *s) { dispatch_release(*s); }
#else
#include <semaphore.h>
typedef sem_t sem;
static inline void sem_make(sem *s, int value) { sem_init(s, 0, value); }
static inline void sem_down(sem *s) { while (sem_wait(s) != 0) {} } /* retry on EINTR */
static inline void sem_up(sem *s) { sem_post(s); }
static inline void sem_free(sem *s) { sem_destroy(s); }
#endif

#endif
