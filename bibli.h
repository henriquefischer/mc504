#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <alloca.h>

#define _QUOTE(x) #x
#define QUOTE(x) _QUOTE(x)

/* defined to use exit instead of abort so that atexit handlers are called. */
#define assert(cond) {if(!(cond)){\
fprintf(stderr, "Assertion '%s' failed in file %s on line %d.\n", \
QUOTE(cond), \
__FILE__, \
__LINE__ \
); \
exit(EXIT_FAILURE); \
}}

/* Represents a set of UNIX semaphores. */
typedef struct {
    int id;
    int num_semaphores;
} sem_set_t;

/* Relates a single semaphore to the semaphore set that it belongs to */
typedef struct {
    sem_set_t *set;
    int num;
} sem_t;

/* operations on sets of semaphores */
void sem_fill_set(sem_set_t *set, const int num_semaphores);
void sem_empty_set(sem_set_t *set);
void sem_unpack_set(sem_set_t *set, sem_t *sem1, ...);
void sem_init_all(sem_set_t *set, const int value);

/* operations on individual semaphores */
void sem_init_index(sem_set_t *set, const int sem_index, const int value);
void sem_wait_index(sem_set_t *set, const int sem_index);
void sem_signal_index(sem_set_t *set,
                      const int sem_index,
                      const int num_signals);

#define sem_init(sem, val) sem_init_index((sem).set, (sem).num, (val))
#define sem_wait(sem) sem_wait_index((sem).set, (sem).num)
#define sem_signal(sem) sem_signal_index((sem).set, (sem).num, 1)
#define sem_signal_ntimes(sem, n) sem_signal_index((sem).set, (sem).num, (n))

#define CRITICAL(sem, context) {sem_wait(sem);{context}sem_signal(sem);}


typedef struct set *set_t;

set_t set_alloc(const int num_slots);
void set_exit_free(set_t set);
void set_free(set_t set);
void set_insert(set_t set, const int item);
int set_take(set_t set);
int set_cardinality(const set_t set);

