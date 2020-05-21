<pre>
1. The implementation involving each exception/interrupt having its own handler allows to push 
   Error code onto the stack. Which would have not been possible if all exception and Interrupts were sent to same handler

   Also having individual handler corresponding to each interrupt/exception allows to enforce seperate protection rule. Like whether the handler can be invoked in kernel mode or user mode.

2. Program user/softint tries to call handler for page fault exception. But that handler can 
   only be executed in kernel mode, while user/softint is a user program. So when handler function call-chain reacher trap_dispatch, and DPL of segment containing page fault handler is found to be 0, i.e less than that of caller procedure, then general protection exception is raised.

3. The exception handler are supposed to be executed in user privelege level,
   If at any point during exception handling, a segment with kernel privelege level is accessed
   from user program then general protection exception will be raised. Otherwise exception will be handled in ususal manner and any exception related message will be delivered to user.

4. Presence of privelege level bit in segments, gates, descriptor allows to enforce a solid   
   Protection policy during control transfers and data-accesses.
   
</pre>