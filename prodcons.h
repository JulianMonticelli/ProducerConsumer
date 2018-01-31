#include <asm/errno.h>
#include <asm/unistd.h>
#include <asm/mman.h>
#include <asm/semaphore.h>

#include <linux/spinlock_types.h>

#define MAP_SIZE 0x00000FFF

// cs1550_sem data structure
struct cs1550_sem {
        // The value for the semaphore
        int value;
        struct cs1550_process_list * list; // The head node of the process list
        spinlock_t * sem_lock; // The necessary spinlock to prevent semaphore r$
};


// Syscall prototypes
void cs1550_sem_init(struct cs1550_sem * sem, int initValue);
void cs1550_up(struct cs1550_sem * sem);
void cs1550_down(struct cs1550_sem * sem);
