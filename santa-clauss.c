/*
 *   André Vitor Terron                                 119098
 *   Henrique Fischer de Paula Lopes                    117205
 *   Gustavo Rodrigues Basso                            105046
 *
 * -------------------------------------------------------------------
 * Santa Clauss Problem
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include "bibli.h"
#include "draw_chars.h"

#define NUM_REINDEER 9
#define NUM_ELVES 9
#define NUM_ELVES_PER_GROUP 3

/* should "waits" take up time? */
#define OBSERVABLE_DELAYS 1

/* max wait time (in approx. cycles) if OBSERVABLE_DELAYS is set */
#define MAX_WAIT_TIME (INT_MAX >> 4)

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static sem_set_t elf_line_set;
static sem_set_t sem_set;
static sem_t santa_busy_mutex;
static sem_t santa_sleep_mutex;
static sem_t reindeer_counting_sem;
static sem_t reindeer_counter_lock;
static int num_reindeer_waiting = 0;
static set_t elves_waiting;
static sem_t elf_counting_sem;
static sem_t elf_mutex;
static sem_t elf_counter_lock;
static sem_t draw;

static int num_elves_being_helped = 0;
static int santa_status = 0;

static void random_wait_elves(const char *message, const int format_var) {
    unsigned int i = rand() % (MAX_WAIT_TIME / 8);
//    fprintf(stdout, message, format_var);
    if(OBSERVABLE_DELAYS) {
        for(; --i; ) /* ho ho ho! */;
    }
}


static void random_wait(const char *message, const int format_var) {
    unsigned int i = (rand() * 16) % MAX_WAIT_TIME;
//    fprintf(stdout, message, format_var);
    if(OBSERVABLE_DELAYS) {
        for(; --i; ) /* ho ho ho! */;
    }
}

static void help_elves(void) {
    int i;
    int elf;
        
    sem_wait(santa_busy_mutex);
    
   

    
    /* help the elves */
     CRITICAL(elf_counter_lock, {   //CRITICAL(elf_mutex, {
	num_elves_being_helped = NUM_ELVES_PER_GROUP;
	
        santa_status = 0;
        for(i = 0; i < NUM_ELVES_PER_GROUP; ++i) {
            elf = set_take(elves_waiting);
            sem_signal_index(&elf_line_set, elf, 1);
        }
	CRITICAL(draw,{
        	print_all(set_cardinality(elves_waiting),num_reindeer_waiting, 3, santa_status);
	//num_elves_being_helped = 0;
        //santa_status = 1;
        //print_all(set_cardinality(elves_waiting),num_reindeer_waiting, 0, santa_status);
    })})                ;
}


static void prepare_sleigh(void) {
    sem_wait(santa_busy_mutex);
//    fprintf(stdout, "Santa: preparing the sleigh. \n");
    CRITICAL(draw, {
    	sem_signal_ntimes(reindeer_counting_sem, NUM_REINDEER);
    });
}


static void *santa(void *_) {
    static int num_launched = 0;
    
    assert(1 == ++num_launched);
    
    while(1) {
        CRITICAL(santa_busy_mutex, { CRITICAL(draw,{
            print_all(set_cardinality(elves_waiting),num_reindeer_waiting, 0, 1);
            santa_status = 1;
        })});
        sem_wait(santa_sleep_mutex);
        
        CRITICAL(santa_busy_mutex, { CRITICAL(draw,{
            print_all(set_cardinality(elves_waiting),num_reindeer_waiting, 0, 0);
            santa_status = 0;
        })});
        if(NUM_REINDEER <= num_reindeer_waiting) {
            num_reindeer_waiting = NUM_REINDEER;
            prepare_sleigh();
            
            sem_wait(santa_busy_mutex);
            sem_wait(santa_sleep_mutex);
            
        } else if(3 <= set_cardinality(elves_waiting)) {
            help_elves();
        }
    }
    return NULL;
}

static void get_help(const int id) {
//    fprintf(stdout, "Elf %d got santa's help! \n", id);
	random_wait_elves("", id);
    CRITICAL(elf_counter_lock,{ CRITICAL(draw, {
        --num_elves_being_helped;
        
        if(!num_elves_being_helped) {
            sem_signal(santa_busy_mutex);
            sem_signal_ntimes(elf_counting_sem, NUM_ELVES_PER_GROUP);
	    print_all(set_cardinality(elves_waiting),num_reindeer_waiting, num_elves_being_helped, santa_status);
        } else {
		print_all(set_cardinality(elves_waiting),num_reindeer_waiting, num_elves_being_helped, santa_status);
	}
    })});
}

static void *elf(void *elf_id) {
    const int id = *((int *) elf_id);
    
    while(1) {
        random_wait_elves("Elf %d is working... \n", id);
//        fprintf(stdout, "Elf %d needs Santa's help. \n", id);

        //sem_wait(elf_counting_sem);
        CRITICAL(elf_mutex, { 
            set_insert(elves_waiting, id);
	    CRITICAL(draw,{
            	print_all(set_cardinality(elves_waiting),num_reindeer_waiting, num_elves_being_helped, santa_status);
            });
            if(NUM_ELVES_PER_GROUP <= set_cardinality(elves_waiting)) {
 //               fprintf(stdout, "Elves: waking up santa! \n");
                sem_signal(santa_sleep_mutex);
            }
        });
        
        sem_wait_index(&elf_line_set, id);
        get_help(id);
    }
    
    return NULL;
}


static void *reindeer(void *reindeer_id) {
    const int id = *((int *) reindeer_id);
    
    random_wait("Reindeer %d is off to the Tropics! \n", id);
    
    CRITICAL(reindeer_counter_lock, {
        ++num_reindeer_waiting;
        CRITICAL(draw,  {
            print_all(set_cardinality(elves_waiting),num_reindeer_waiting, num_elves_being_helped, santa_status);
        });
    });
    
    
    if(NUM_REINDEER <= num_reindeer_waiting) {
//        fprintf(stdout, "Reindeer %d: I'm the last one; I'll get santa!\n", id);
        sem_signal(santa_sleep_mutex);
    }
    
    sem_wait(reindeer_counting_sem);
    
    CRITICAL(reindeer_counter_lock, {
        --(num_reindeer_waiting);
        
        if(0 == num_reindeer_waiting) {
	    CRITICAL(draw, {
            	fprintf(stdout, "Santa: Ho ho ho! Off to deliver presents! \n");
            	exit(EXIT_SUCCESS);
	    });
        }
    });
    
    return NULL;
}


static void sequence_pthreads(int num_threads,
                              pthread_t *thread_ids,
                              void *(*func)(void *),
                              int *args) {
    int i;
    for(i = 0; num_threads--; ++i) {
        pthread_create(&(thread_ids[i]), NULL, func, (void *) &(args[i]));
    }
}


static void free_resources(void) {
    static int resources_freed = 0;
    if(!resources_freed) {
        resources_freed = 1;
//        fprintf(stdout,"\n.. And that year was a Merry Christmas indeed!\n\n");
        sem_empty_set(&sem_set);
        sem_empty_set(&elf_line_set);
        set_exit_free(elves_waiting);
    }
}

static void sigint_handler(int _) {
    exit(EXIT_SUCCESS);
}


static void launch_threads(void) {
    
    pthread_t thread_ids[1 + NUM_ELVES + NUM_REINDEER+1];
    
    int ids[MAX(NUM_ELVES, NUM_REINDEER)];
    int i;
    
    /* fill up the ids */
    for(i = 0; i < MAX(NUM_ELVES, NUM_REINDEER); ++i) {
        ids[i] = i;
    }
    
    pthread_create(&(thread_ids[0]), NULL, &santa, NULL);
    sequence_pthreads(NUM_ELVES, &(thread_ids[1]), &elf, &(ids[0]));
    sequence_pthreads(NUM_REINDEER, thread_ids + 1 + NUM_ELVES+1, &reindeer, ids);

    for(i = 0; i < (1 + NUM_ELVES + NUM_REINDEER +1); ++i) {
        pthread_join(thread_ids[i], NULL);
    }
}

int main(void) {
    
    sem_fill_set(&sem_set, 8);
    sem_fill_set(&elf_line_set, NUM_ELVES);
    
    elves_waiting = set_alloc(NUM_ELVES);
    
    if(!atexit(&free_resources)) {
        signal(SIGINT, &sigint_handler);
        
        
        sem_unpack_set(&sem_set,
                       &reindeer_counter_lock,
                       &elf_counter_lock,
                       &draw,
                       &santa_busy_mutex,
                       &santa_sleep_mutex,
                       &reindeer_counting_sem,
                       &elf_counting_sem,
                       &elf_mutex
                       );
        
        sem_init(elf_mutex, 1);
        sem_init(draw,1);
        sem_init(reindeer_counter_lock, 1);
        sem_init(elf_counter_lock, 1);
        sem_init(santa_busy_mutex, 1);
        sem_init(santa_sleep_mutex, 0); /* starts as locked! */
        sem_init(reindeer_counting_sem, 0);
        sem_init(elf_counting_sem, NUM_ELVES_PER_GROUP);
        sem_init_all(&elf_line_set, 0);

        srand((unsigned int) time(NULL));
        
        launch_threads();
        
    } else {
        free_resources();
    }
    
    set_free(elves_waiting);
    
    return 0;
}
