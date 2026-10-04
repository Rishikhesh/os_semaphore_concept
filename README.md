# OS package

Three classic synchronization problems with pthreads + semaphores:

| Program | Problem |
| --- | --- |
| `producer_consumer` | Bounded buffer: 5 producers, 5 consumers, 5 slots |
| `teaching_assistant [students]` | Sleeping TA (sleeping barber): 1 TA, 3 waiting chairs |
| `dining_philosopher <n> [meals]` | Dining philosophers without deadlock |

`make` builds all three, `make test` runs them; each asserts its invariants and exits non-zero if one breaks.
`sem.h` wraps semaphores so it works on macOS too (macOS doesn't implement `sem_init`).
