// include statements...
#include <linux/prodcons.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

// Program globals

int num_prod; // Number of producers
int num_cons; // Number of consumers
int buf_size; // Size of buffer
int * buffer;

int * buff_in;  // Shared buffer value (producer)
int * buff_out; // Shared buffer value (consumer)
int * product_count; // Shared product count value (used primarily for producer - consumer reads from buffer)
int * dispatched_processes;

int total_processes;

int active_producers = 1; // One because we start our while loop after the first fork
int active_consumers = 1; // One because we start our while loop after the first fork

struct cs1550_sem * empty;
struct cs1550_sem * full;
struct cs1550_sem * mutex;
//struct cs1550_sem * consumer_mutex;

int main(int argc, char * argv[]) {
	// Assure 3 arguments (arg[0] is program executable)
	if (argc != 4) {
		printf("Run program as prodcons <num producers> <num consumers> <buffer_size>");
		exit(-1);
	}

	// Program arguments to global vars
	num_prod = atoi(argv[1]);
	num_cons = atoi(argv[2]);
	buf_size = atoi(argv[3]);
	
	total_processes = num_prod+num_cons;
	
	// Make sure our arguments are valid
	if (num_prod < 1) {
		printf("Number of producers must be at least 1!");
		exit(-1);
	} else if (num_cons < 1) {
		printf("Number of consumers must be at least 1!");
		exit(-1);
	} else if (buf_size < 2) {
		printf("Buffer size must be at least 2!");
		exit(-1);
	}

	// Allocate memory for shared buffer
	buffer = (int *)mmap(NULL, (sizeof(int) * buf_size), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
	
	// Allocate memory for consumer and producer semaphores and respective mutexes
	empty = (struct cs1550_sem *) mmap(NULL, (sizeof(struct cs1550_sem)), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
	full = (struct cs1550_sem *) mmap(NULL, (sizeof(struct cs1550_sem)), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
	mutex = (struct cs1550_sem *) mmap(NULL, (sizeof(struct cs1550_sem)), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
	//consumer_mutex = (struct cs1550_sem *) mmap(NULL, (sizeof(struct cs1550_sem)), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);

	// Allocate memory for producer and consumer values (where the buffer is must be shared across processes)	
	buff_in  = (int *) mmap(NULL, (sizeof(int)), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
	buff_out = (int *) mmap(NULL, (sizeof(int)), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
	product_count= (int *) mmap(NULL, (sizeof(int)), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
	
	// This is a hack that may get my producers and consumers to start at the same time
	dispatched_processes = (int *) mmap(NULL, (sizeof(int)), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);

	// Init all semaphores
	cs1550_sem_init(empty, buf_size);
	cs1550_sem_init(full, 0);
	cs1550_sem_init(mutex, 1);
	//cs1550_sem_init(consumer_mutex, 1);


	// Begin forking as many producers as we need
	
	pid_t pid = fork(); // Fork process...
	while(active_producers < num_prod && pid != 0) { // While we have more proceses to fork & we are the parent process
		active_producers++; // Increment active producers
		pid = fork(); // Create a new process
	}

	// If we're the child process...
	if (pid == 0) {
		producer(active_producers); // Enter an infinite producer loop
	}
	
	// Only the main thread will ever reach this point...

	// Now, begin forking as many consumers as we need

	pid = fork(); // Fork process....
	while(active_consumers < num_cons && pid != 0) { // While we have more consumers to fork & we are the parent process
		active_consumers++; // Increment active consumers
		pid = fork(); // Create a new process
	}

	// If we're the child process...
	if (pid == 0) {
		consumer(active_consumers); // Enter an infinite consumer loop
	}
	
	// Only a parent process will ever get here
	wait(NULL);
}


int producer(int id) {
	*dispatched_processes += 1;
	while(*dispatched_processes < total_processes)
		; // busy wait
	// Infinitely loop
	while(1) {
		// Enter critical section
		cs1550_down(empty); // Decrement empty semaphore (1 less empty as we're producing)
		cs1550_down(mutex); // Decrement mutex
		
		// In critical section
		*product_count += 1;
		buffer[*buff_in] = *product_count;
		printf("Producer %d produced %d\n", id, buffer[*buff_in]);
		fflush(stdout);
		*buff_in = (*buff_in + 1) % buf_size;
		

		// Exit critical section
		cs1550_up(mutex); // Increment mutex
		cs1550_up(full); // Increment full semaphore (1 more full as we've produced)
	}
}

int consumer(int id) {
	*dispatched_processes += 1;
	while(*dispatched_processes < total_processes)
		;
	// Infinitely loop
	while(1) {
		// Enter critical section
		cs1550_down(full); // Decrement full semaphore (1 less full as we're consuming)
		cs1550_down(mutex); // Decrement mutex
		
		// In critical section
		printf("Consumer %d consumed %d\n", id, buffer[*buff_out]);
		fflush(stdout);
		*buff_out = (*buff_out + 1) % buf_size;
		
		// Exit critical section
		cs1550_up(mutex); // Increment mutex
		cs1550_up(empty); // Increment empty semaphore (1 more empty because we consumed)
	}
}




// Syscall stuff
void cs1550_sem_init(struct cs1550_sem * sem, int initialValue) {
	syscall(__NR_cs1550_sem_init, sem, initialValue);
}

void cs1550_down(struct cs1550_sem * sem) {
	syscall(__NR_cs1550_down, sem);
}

void cs1550_up(struct cs1550_sem * sem) {
	syscall(__NR_cs1550_up, sem);
}
