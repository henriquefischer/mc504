#include "bibli.h"

typedef union {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
} my_semun_t;

typedef struct sembuf my_sembuf_t;

struct set {
    sem_t write_lock;
    sem_set_t semaphore_set;
    char *slots;
    
    int num_slots;
    int cardinality;
};

set_t set_alloc(const int num_slots) {
    
    set_t set = NULL;
    size_t obj_size = sizeof(struct set);
    size_t buff_size = sizeof(char) * num_slots;
    size_t inv_pad;
    
    assert(0 < num_slots);
    
    /* make sure that the set's slots are 8-byte aligned */
    obj_size += (inv_pad = obj_size % 8) > 0 ? 8 - inv_pad : 0;
    set = (set_t) malloc(obj_size + buff_size);
    if(NULL == set) {
        return NULL;
    }
    
    set->slots = ((char *) set) + obj_size;
    set->cardinality = 0;
    set->num_slots = num_slots;
    
    
    memset(&(set->slots[0]), 0, buff_size);
    
    /* get the write lock */
    sem_fill_set(&(set->semaphore_set), 1);
    sem_unpack_set(&(set->semaphore_set), &(set->write_lock));
    sem_init(set->write_lock, 1);
    
    return set;
}

void set_exit_free(set_t set) {
    assert(NULL != set);
    sem_empty_set(&(set->semaphore_set));
}

void set_free(set_t set) {
    assert(NULL != set);
    set_exit_free(set);
    free(set);
}

void set_insert(set_t set, const int item) {
    assert(NULL != set);
    assert(item >= 0 && item < set->num_slots);
    
    CRITICAL(set->write_lock, {
        /* add the item into the set */
        set->slots[item] = 1;
        if(set->cardinality < set->num_slots) {
            ++(set->cardinality);
        }
    });
}

int set_take(set_t set) {
    int item;
    assert(NULL != set);
    
    CRITICAL(set->write_lock, {
        /* find a random item in the set */
        for(item = rand() % set->num_slots;
            !set->slots[item];
            item = rand() % set->num_slots);
        
        set->slots[item] = 0;
        --(set->cardinality);
    });
    
    return item;
}

int set_cardinality(const set_t set) {
    return set->cardinality;
}
/**
 * Fill a semaphore set. Prints an error if
 *
 * Params: - Pointer to the semaphore set to fill
 *         - The number of semaphores to fill the set with.
 *
 * Side-Effects: If this function fails then the program will be exited.
 */
void sem_fill_set(sem_set_t *set, const int num_semaphores) {
    
    assert(NULL != set);
    assert(0 < num_semaphores);
    
    set->num_semaphores = num_semaphores;
    set->id = semget(
                     IPC_PRIVATE,
                     num_semaphores,
                     IPC_CREAT | IPC_EXCL | IPC_NOWAIT | 0666
                     );
    
    if(-1 == set->id) {
        perror("sem_fill_set[semget]");
        exit(EXIT_FAILURE);
    }
}

/**
 * Empty a set of semaphores.
 *
 * Params: - Pointer to a filled semaphore set.
 *
 * Note: emptying a set doesn't change the state of any previously unpacked
 *       semaphores from the set. Using these semaphores after the set has
 *       been emptied results in undefined behavior.
 *
 * Side-Effects: If this function fails then the program will be exited.
 */
void sem_empty_set(sem_set_t *set) {
    my_semun_t _;
    assert(NULL != set);
    
    if(-1 == semctl(set->id, 0, IPC_RMID, _)) {
        perror("sem_empty_set[semctl]");
        exit(EXIT_FAILURE);
    }
    
    set->id = -1;
    set->num_semaphores = -1;
}

/**
 * Unpack a semaphore set into individual semaphores that can then be
 * initialized.
 *
 * Params: - Pointer to the semaphore set to unpack
 *         - Variable number of semaphores to unpack from the set; the number
 *           of arguments expected is the same as the number of semaphores in
 *           the set.
 *
 * Side-Effects: If this function fails then the program will be exited.
 */
void sem_unpack_set(sem_set_t *set, sem_t *sem1, ...) {
    va_list sems;
    sem_t *curr_sem;
    int i;
    
    va_start(sems, sem1);
    
    assert(NULL != set);
    assert(NULL != sem1);
    
    for(i = 0, curr_sem = sem1;
        i < set->num_semaphores;
        ++i, curr_sem = va_arg(sems, sem_t *)) {
        
        curr_sem->num = i;
        curr_sem->set = set;
    }
    
    va_end(sems);
}

/**
 * Initialize a given semaphore to some value.
 *
 * Params: - Pointer to semaphore set
 *         - Index into the semaphore set of the semaphore we're interested in
 *         - Value to initialize the semaphore
 *
 * Side-Effects: If this function fails then the program will be exited.
 */
void sem_init_index(sem_set_t *set, const int sem_index, const int value) {
    my_semun_t arg;
    
    assert(NULL != set);
    assert(0 <= sem_index && sem_index < set->num_semaphores);
    
    arg.val = value;
    if(-1 == semctl(set->id, sem_index, SETVAL, arg)) {
        perror("sem_init_index[semctl]");
        exit(EXIT_FAILURE);
    }
}

/**
 * Initialize all semaphores within a set.
 *
 * Params: - Pointer to semaphore set.
 *         - Value which will be assigned to all semaphores in the above set.
 *
 * Side-Effects: If this function fails then the program will be exited.
 */
void sem_init_all(sem_set_t *set, const int value) {
    int i;
    unsigned short *sem_ids;
    my_semun_t arg;
    struct semid_ds sem_buf;
    assert(NULL != set);
    
    arg.buf = &sem_buf;
    
    /* get the info */
    if(-1 == semctl(set->id, 0, IPC_STAT, arg))  {
        perror("sem_init_all[semctl:IPC_STAT]");
        exit(EXIT_FAILURE);
    }
    
    /* fill the ids */
    sem_ids = alloca(sizeof(unsigned short) * set->num_semaphores);
    if(NULL == sem_ids) {
        perror("sem_init_all[alloca]");
        exit(EXIT_FAILURE);
    }
    
    arg.array = &(sem_ids[0]);
    for(i = 0; i < set->num_semaphores; ++i) {
        sem_ids[i] = value;
    }
    
    /* initialize the values */
    if(-1 == semctl(set->id, 0, SETALL, arg)) {
        perror("sem_init_all[semctl:SETALL]");
        exit(EXIT_FAILURE);
    }
}

/**
 * Wait until a given semaphore has cleared.
 *
 * Params: - Pointer to semaphore set to which the indexed semaphore belongs.
 *         - Index of semaphore to wait on.
 *
 * Side-Effects: If this function fails then the program will be exited.
 */
void sem_wait_index(sem_set_t *set, const int sem_index) {
    my_sembuf_t op;
    
    assert(NULL != set);
    assert(0 <= sem_index && sem_index < set->num_semaphores);
    
    op.sem_num = sem_index;
    op.sem_flg = 0;
    op.sem_op = -1;
    
    if(-1 == semop(set->id, &op, 1)) {
        perror("sem_wait_index[semop]");
        exit(EXIT_FAILURE);
    }
}

/**
 * Signal a semaphore num_signals times.
 *
 * Params: - Pointer to semaphore set
 *         - Index of semaphore within that set
 *         - Number of times to signal that semaphore. This must be a positive
 *           integer.
 *  
 * Side-Effects: If this function fails then the program will be exited.
 */
void sem_signal_index(sem_set_t *set,
                      const int sem_index,
                      const int num_signals) {
    my_sembuf_t op;
    
    assert(NULL != set);
    assert(num_signals > 0);
    assert(0 <= sem_index && sem_index < set->num_semaphores);
    
    op.sem_num = sem_index;
    op.sem_flg = 0;
    op.sem_op = num_signals;
    
    if(-1 == semop(set->id, &op, 1)) {
        perror("sem_init[semop]");
        exit(EXIT_FAILURE);
    }
}