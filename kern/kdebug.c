#include <inc/stab.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/mmu.h>
#include <kern/pmap.h>
#include <kern/kdebug.h>

extern const struct Stab __STAB_BEGIN__[];	// Beginning of stabs table
extern const struct Stab __STAB_END__[];	// End of stabs table
extern const char __STABSTR_BEGIN__[];		// Beginning of string table
extern const char __STABSTR_END__[];		// End of string table

const char *pdte_four_kb_perms[] = {
                               "Present: ",
                               "Read/Write: ",
                               "User/Supervison: ",
                               "Write-Through: ",
                               "Cache Disabled: ",
                               "Accessed: ",
                               "Dirty: ",
                               "Page Table Attribute Index: ",
                               "Global Page: ",
                               "Available: "
                               };

const char *pde_four_mb_perms[] = {
                               "Present: ",
                               "Read/Write: ",
                               "User/Supervison: ",
                               "Write-Through: ",
                               "Cache Disabled: ",
                               "Accessed: ",
                               "Dirty: ",
                               "Page Size (1 indicates 4 MBytes) : ",
                               "Global Page: ",
                               "Available: ",
                               "Page table attribute index"
                               };
const uint8_t pdte_four_kb_byte_info[10] = {1,1,1,1,1,1,1,1,1,3};
const uint8_t pdte_four_mb_byte_info[11] = {1,1,1,1,1,1,1,1,1,3,1};
const uint8_t pdte_four_kb_len = 10;    
const uint8_t pdte_four_mb_len = 11;      
// stab_binsearch(stabs, region_left, region_right, type, addr)
//
//	Some stab types are arranged in increasing order by instruction
//	address.  For example, N_FUN stabs (stab entries with n_type ==
//	N_FUN), which mark functions, and N_SO stabs, which mark source files.
//
//	Given an instruction address, this function finds the single stab
//	entry of type 'type' that contains that address.
//
//	The search takes place within the range [*region_left, *region_right].
//	Thus, to search an entire set of N stabs, you might do:
//
//		left = 0;
//		right = N - 1;     /* rightmost stab */
//		stab_binsearch(stabs, &left, &right, type, addr);
//
//	The search modifies *region_left and *region_right to bracket the
//	'addr'.  *region_left points to the matching stab that contains
//	'addr', and *region_right points just before the next stab.  If
//	*region_left > *region_right, then 'addr' is not contained in any
//	matching stab.
//
//	For example, given these N_SO stabs:
//		Index  Type   Address
//		0      SO     f0100000
//		13     SO     f0100040
//		117    SO     f0100176
//		118    SO     f0100178
//		555    SO     f0100652
//		556    SO     f0100654
//		657    SO     f0100849
//	this code:
//		left = 0, right = 657;
//		stab_binsearch(stabs, &left, &right, N_SO, 0xf0100184);
//	will exit setting left = 118, right = 554.
//
static void
stab_binsearch(const struct Stab *stabs, int *region_left, int *region_right,
	       int type, uintptr_t addr)
{
	int l = *region_left, r = *region_right, any_matches = 0;

	while (l <= r) {
		int true_m = (l + r) / 2, m = true_m;

		// search for earliest stab with right type
		while (m >= l && stabs[m].n_type != type)
			m--;
		if (m < l) {	// no match in [l, m]
			l = true_m + 1;
			continue;
		}

		// actual binary search
		any_matches = 1;
		if (stabs[m].n_value < addr) {
			*region_left = m;
			l = true_m + 1;
		} else if (stabs[m].n_value > addr) {
			*region_right = m - 1;
			r = m - 1;
		} else {
			// exact match for 'addr', but continue loop to find
			// *region_right
			*region_left = m;
			l = m;
			addr++;
		}
	}

	if (!any_matches)
		*region_right = *region_left - 1;
	else {
		// find rightmost region containing 'addr'
		for (l = *region_right;
		     l > *region_left && stabs[l].n_type != type;
		     l--)
			/* do nothing */;
		*region_left = l;
	}
}


// debuginfo_eip(addr, info)
//
//	Fill in the 'info' structure with information about the specified
//	instruction address, 'addr'.  Returns 0 if information was found, and
//	negative if not.  But even if it returns negative it has stored some
//	information into '*info'.
//
int
debuginfo_eip(uintptr_t addr, struct Eipdebuginfo *info)
{
	const struct Stab *stabs, *stab_end;
	const char *stabstr, *stabstr_end;
	int lfile, rfile, lfun, rfun, lline, rline;
	// Initialize *info
	info->eip_file = "<unknown>";
	info->eip_line = 0;
	info->eip_fn_name = "<unknown>";
	info->eip_fn_namelen = 9;
	info->eip_fn_addr = addr;
	info->eip_fn_narg = 0;

	// Find the relevant set of stabs
	if (addr >= ULIM) {
		stabs = __STAB_BEGIN__;
		stab_end = __STAB_END__;
		stabstr = __STABSTR_BEGIN__;
		stabstr_end = __STABSTR_END__;
	} else {
		// Can't search for user-level addresses yet!
  	        panic("User address");
	}

	// String table validity checks
	if (stabstr_end <= stabstr || stabstr_end[-1] != 0)
		return -1;

	// Now we find the right stabs that define the function containing
	// 'eip'.  First, we find the basic source file containing 'eip'.
	// Then, we look in that source file for the function.  Then we look
	// for the line number.

	// Search the entire set of stabs for the source file (type N_SO).
	lfile = 0;
	rfile = (stab_end - stabs) - 1;
	stab_binsearch(stabs, &lfile, &rfile, N_SO, addr);
	if (lfile == 0)
		return -1;

	// Search within that file's stabs for the function definition
	// (N_FUN).
	lfun = lfile;
	rfun = rfile;
	stab_binsearch(stabs, &lfun, &rfun, N_FUN, addr);

	if (lfun <= rfun) {
		// stabs[lfun] points to the function name
		// in the string table, but check bounds just in case.
		if (stabs[lfun].n_strx < stabstr_end - stabstr)
			info->eip_fn_name = stabstr + stabs[lfun].n_strx;
		info->eip_fn_addr = stabs[lfun].n_value;
		addr -= info->eip_fn_addr;
		// Search within the function definition for the line number.
		lline = lfun;
		rline = rfun;
	} else {
		// Couldn't find function stab!  Maybe we're in an assembly
		// file.  Search the whole file for the line number.
		info->eip_fn_addr = addr;
		lline = lfile;
		rline = rfile;
	}
	// Ignore stuff after the colon.
	info->eip_fn_namelen = strfind(info->eip_fn_name, ':') - info->eip_fn_name;


	// Search within [lline, rline] for the line number stab.
	// If found, set info->eip_line to the right line number.
	// If not found, return -1.
	//
	// Hint:
	//	There's a particular stabs type used for line numbers.
	//	Look at the STABS documentation and <inc/stab.h> to find
	//	which one.
	// Your code here.
    stab_binsearch(stabs, &lline, &rline, N_SLINE, addr);
	if(lline > rline)
	    return -1;	 
    info->eip_line = stabs[lline].n_desc;
	// Search backwards from the line number for the relevant filename
	// stab.
	// We can't just use the "lfile" stab because inlined functions
	// can interpolate code from a different file!
	// Such included source files use the N_SOL stab type.
	while (lline >= lfile
	       && stabs[lline].n_type != N_SOL
	       && (stabs[lline].n_type != N_SO || !stabs[lline].n_value))
		lline--;
	if (lline >= lfile && stabs[lline].n_strx < stabstr_end - stabstr)
		info->eip_file = stabstr + stabs[lline].n_strx;

	// Set eip_fn_narg to the number of arguments taken by the function,
	// or 0 if there was no containing function.
	if (lfun < rfun)
		for (lline = lfun + 1;
		     lline < rfun && stabs[lline].n_type == N_PSYM;
		     lline++)
			info->eip_fn_narg++;

	return 0;
}

void
print_range_data(uint32_t start_addr, uint32_t end_addr){
     uint32_t i = 0;
	 const void * addr = NULL;
	 for( i = start_addr ; i <= end_addr; i += PGSIZE)
	 {
		addr = (const void *)i;
		if(is_extended_mapping(kern_pgdir[PDX(addr)]))
		   print_address_metadata_extend(addr);
		else   
	       print_address_metadata(addr);
		cprintf("--------------------------------------------------------------------------------");
	 }	
}

void
print_address_metadata_extend(const void * va)
{
   pde_t * pd_entry = &kern_pgdir[PDX(va)];
   int permissions = *pd_entry & 0x1FFF;
   cprintf("\nAddress = 0x%08x: base = 0x%08x\n", va, PDX(va));
   if(pd_entry && (*pd_entry & PTE_P))
   {
       cprintf("Physical address: 0x%08x\n", (*pd_entry & 0xFFC00000)|(((uintptr_t) va & 0x004FFFFF)));
	   cprintf("Page Directory entry permissions\n");
	   print_permission(((*pd_entry) & 0xFFF), pde_four_mb_perms, pdte_four_mb_byte_info, pdte_four_mb_len);
   }
   else
   {
       cprintf("No page directory entry exists for this address\n");
   }
   
}
void 
print_address_metadata(const void * va)
{
	cprintf("\nAddress = 0x%08x:\n", va);
	pde_t * pt_base = &kern_pgdir[PDX(va)];
	if(pt_base && (*pt_base & PTE_P))
	{
        cprintf("Page Table base = 0x%08x ", *pt_base);
        pte_t * pt_entry = (pte_t*)KADDR(PTE_ADDR(*pt_base));
        pte_t * pt_data = &pt_entry[PTX(va)];
		if(pt_data && (*pt_data & PTE_P))
		{
           cprintf("Page Address, Physical = 0x%08x, Virtual = 0x%08x, Permissions = 0x%08x\n",PTE_ADDR(*pt_data), KADDR(PTE_ADDR(*pt_data)), ((*pt_data) & 0xFFF));
		   cprintf("Page Directory entry permissions\n");
		   print_permission(((*pt_base) & 0xFFF), pdte_four_kb_perms, pdte_four_kb_byte_info, pdte_four_kb_len);
		   cprintf("\nPage Table entry permissions\n");
		   print_permission(((*pt_data) & 0xFFF), pdte_four_kb_perms, pdte_four_kb_byte_info, pdte_four_kb_len);
		}
		else
		{
	       cprintf("No valid page table entry present\n");
		}
		
	}	    
	else
	{
        cprintf("No page directory entry exists for this address\n");
	}
	
}

void 
print_permission(int perms, const char** perm_str, const uint8_t bit_info[], const uint8_t size){
    uint8_t i = 0, bit_len = 0, bit = 0, j = 0;
	char  perm_data[4];
	char *perm_bits = perm_data;
	for(i = 0; i < size ; i++){
       bit_len = bit_info[i];
	   perm_data[bit_len] = '\0';
	   perm_bits = &perm_data[bit_len - 1];
	       while(bit_len-- > 0){
              bit = perms & 1;
			  perms = perms >> 1;
			  *perm_bits = (('0') + bit);
			  perm_bits--;
		}
		perm_bits = perm_data;
		cprintf("%s %s\n", perm_str[i], perm_data);
	}

}

void 
set_permissions(uint32_t address, uint32_t permission){
	if(is_extended_mapping(address)){
		pde_t *pd_entry = &kern_pgdir[PDX(address)];
		if(pd_entry && (*pd_entry & PTE_P))
		   kern_pgdir[PDX(address)] = (*pd_entry & 0xFFFFE000)| permission;
		else
		   cprintf("No valid mapping found");
		return;   
	}

	pte_t * pt_entry = pgdir_walk(kern_pgdir, (const void*)address, 0);
	if(pt_entry)
		*pt_entry = PTE_ADDR(*pt_entry) | permission;
	else
	    cprintf("No valid mapping found");
}

void
update_permissions(uint32_t address, uint32_t mode, uint32_t permission){
	switch (mode)
	{
	    case 1:
		   set_permissions(address,0x000);
		   break;
	    case 2:
	       set_permissions(address, 0xFFF);
		   break;
	    case 3:
	       set_permissions(address,permission);
		   break;
        default:
	       break;		 	
	}
}

void
dump_data(uint32_t start_addr, uint32_t end_addr, bool is_physical_address_range){
	int i = 0; 
	uintptr_t curr_addr = 0;
	uint32_t *addr;
	for(i = 0 ;i < end_addr - start_addr; i++){
		curr_addr = start_addr + i;
		addr =  is_physical_address_range ? (uint32_t*) KADDR(curr_addr) : (uint32_t*) curr_addr;
		if(i%4 == 0)
		{
            cprintf("\n 0x%08x: ", curr_addr);
		}
		cprintf(" 0x%08x ",*addr);
	} 
	cprintf("\n");
}