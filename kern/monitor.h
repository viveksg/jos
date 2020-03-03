#ifndef JOS_KERN_MONITOR_H
#define JOS_KERN_MONITOR_H
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

struct Trapframe;

// Activate the kernel monitor,
// optionally providing a trap frame indicating the current state
// (NULL if none).
void monitor(struct Trapframe *tf);

// Functions implementing monitor commands.
int mon_help(int argc, char **argv, struct Trapframe *tf);
int mon_kerninfo(int argc, char **argv, struct Trapframe *tf);
int mon_backtrace(int argc, char **argv, struct Trapframe *tf);
int showmappings(int argc, char **argv, struct Trapframe *tf);
int setperms(int argc, char **argv, struct Trapframe *tf);
int dumpdata(int argc, char **argv, struct Trapframe *tf);
int dump_pgdir(int argc, char **argv, struct Trapframe *tf);
int _test85();
int _test83();
int _test84();
void print_args(uint32_t, int);
void print_line_data(uint32_t);

#endif	// !JOS_KERN_MONITOR_H
