#ifndef JOS_KERN_KDEBUG_H
#define JOS_KERN_KDEBUG_H
#include <inc/types.h>

// Debug information about a particular instruction pointer
struct Eipdebuginfo {
	const char *eip_file;		// Source code filename for EIP
	int eip_line;			// Source code linenumber for EIP

	const char *eip_fn_name;	// Name of function containing EIP
					//  - Note: not null terminated!
	int eip_fn_namelen;		// Length of function name
	uintptr_t eip_fn_addr;		// Address of start of function
	int eip_fn_narg;		// Number of function arguments
};

    
int debuginfo_eip(uintptr_t , struct Eipdebuginfo *);
void print_address_metadata_extend(const void *);
void print_range_data(uint32_t, uint32_t);
void print_address_metadata(const void *);
void print_permission(int , const char** , const uint8_t[], const uint8_t );
void set_permissions(uint32_t, uint32_t);
void update_permissions(uint32_t, uint32_t, uint32_t);
void dump_data(uint32_t, uint32_t, bool);
void dump_page_directory(uint32_t);
void dump_page_table(uint32_t , uint32_t);
pde_t* get_page_table_base(uintptr_t);
#endif
