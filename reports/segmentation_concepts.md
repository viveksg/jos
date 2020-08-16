<pre>
x86 address handling involves conversion of logical addresses to linear addresses through
Segment based translation and then conversion of linear address to physical address through
Page translation

Segments are memory regions or chunks of address space, usually used for specific purpose,
Like storing codes/text, maintaining stacks, maintaining data-structures etc.

A segment is defined by many attributes but some important attributes are 
base (base address of segment), limit (added to base to define boundary of segment),
permission bit (DPL), type (depending on type of information stored in segment).

Modern operating systems usually use a flat address scheme which results in identical
linear address corresponding to logical address. So use of segments for address translation
is kind of trivial. But segmentation mechanism facilitates enforcing protection protocols for address space

Just like Paging concept where memory(virtual) is viewed as collection of pages and
actual physical memory is collection of frames(of size equal to the page), Segmentation
concepts involves dividing the address space into segments, such that each segment is used for
Store specific kind of data and has its own privelege settings. A Segment may contain more than one page.

Logical to Linear Address translation:
Segment address translation involves following data-structures
1. Descriptors
2. Descriptor tables
3. Selectors
4. Segment Registers

Descriptors: A Segment Descriptor is a datastructure which holds attributes that defines the segment, for example base address of segment, limit of segment, privelege level of segment, type of segment and some other attributes. Size of this data-structure is 8 bytes.

Descriptors table:A Segment descriptor table is a collection of descriptors. A global descriptor table is segment descriptor table which is accessible to multiple processes.
While local descriptor table is confined to single process. The base addresses of these tables
are loaded in Global Descriptor Table Register (GDTR), Local Descriptor Table Register(LDTR)

Selectors: A Segment selector is the part of logical address is bits which are combined with
GDTR (or LDTR) value to select a descriptor from descriptor table.

Segment Registers: A descriptor when selected from GDT(or LDT) using a selector, is loaded into the segment register for furthur usage. Segment register has a visible portion accessible by user program, and a invisible portion only accessible by kernel.
 

Concept of Gates: 
Gates are descriptors which points to the other descriptors(pointing to actual segment).
Gates are used for control transfer from one segment to the other during call, task switch,
interrupt and exception handling

Checks on segments:

Type Checking: 
The type field of the segments is use to distinguish between the categories of 
Segments, also the type field defines the intended usage of segment.
For example 'write' bit in type field of data segment descriptor helps  determine whether instructions can write into it. 
'Read' bit of executable descriptor type tells whether external instruction can read or write to the segment.

Type checking is use to detect programming errors that would attempt to use the segment in ways
not intended by the programmer.

Some important facts:
* CS register can only be loaded with selector of executable segment.
* Selectors of executable segment that are not readable cannot be loaded into data sagment    
  register.
* Only selectors of writable data segment cab be loaded into SS
* No instruction should be able to write into executable segment.
* No instruction may read from executable segment if data bit is not set.
* No instruction may write into data segment if write bit is not set.

Limit Checking:
Limit field of the descriptor help processor to determine the limit of segment or last address of segment. The processor interprets limit field based on "granularity" bit G. Limit field of descriptor is 20 bits long.

If G = 0, limit is interpreted at granularity of bytes so limit may range from 0 to FFFFF
If G = 1, processor appends 12 low order set bits to the limit. Range 0xFFF - 0xFFFFFFFF

For all segments except down segment limit is less than size of the segment.

The processor issue general protection exception if program tries to access
* Memory byte at and address > limit
* Memory word at an address >= limit
* Memory double at an address >= limit - 2

The "E" bit of type field of segment descriptor tells whether segment grows up or down.


</pre>