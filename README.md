This program was written for the linux-2.6.23.1 kernel, which should include its own makefile.

First thing's first, to make my omdifications to the kernel, these files must go in their appropriate place:



```sys.c		-> linux-2.6.23.1/kernel/sys.c

unistd.h	->	linux-2.6.23.1/include/asm/unistd.h

prodcons.h	->	linux-2.6.23.1/include/linux/prodcons.h

syscalls.h	->	linux-2.6.23.1/include/linux/syscalls.h

syscall_table.S	->	linux-2.6.23.1/arch/i386/kernel/syscall_table.S

```

Then, you can compile the kernel and compile prodcons.c to see the producer-consumer problem being solved successfully.
