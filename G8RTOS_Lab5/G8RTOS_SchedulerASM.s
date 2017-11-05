; G8RTOS_SchedulerASM.s
; Holds all ASM functions needed for the scheduler
; Note: If you have an h file, do not have a C file and an S file of the same name

	; Functions Defined
	.def G8RTOS_Start, PendSV_Handler

	; Dependencies
	.ref CurrentlyRunningThread, G8RTOS_Scheduler

	.thumb		; Set to thumb mode
	.align 2	; Align by 2 bytes (thumb mode uses allignment by 2 or 4)
	.text		; Text section

; Need to have the address defined in file 
; (label needs to be close enough to asm code to be reached with PC relative addressing)
RunningPtr: .field CurrentlyRunningThread, 32

; G8RTOS_Start
;	Sets the first thread to be the currently running thread
;	Starts the currently running thread by setting Link Register to tcb's Program Counter
G8RTOS_Start:

	.asmfunc
	; Implement this
	
	ldr r0, RunningPtr								; r0 holds address of Currently Running Thread
	ldr r1, [r0]									; r1 holds the value pointed to by Currently Running Thread
	ldr sp, [r1]									; Stack Pointer has address pointed to by Currently Running Thread
	
	pop {r4-r11}									; r4-r11 restored for Currently Running Thread
	pop {r0-r3}										; r0-r3 restored
	pop {r12}										; r12 restored
	
	add sp, sp, #4									; discard the LR stored in the thread stack
	pop {LR}										; save the PC register into the CPU's LR 
	
	add sp, sp, #4									; discard the status register stored in the thread stack
													; SP now points to highest address in new stack
	
	CPSIE I											; Enable interrupts
	
	bx lr											; Branch to LR, return
	
	.endasmfunc


; PendSV_Handler
; - Performs a context switch in G8RTOS
; 	- Saves remaining registers into thread stack
;	- Saves current stack pointer to tcb
;	- Calls G8RTOS_Scheduler to get new tcb
;	- Set stack pointer to new stack pointer from new tcb
;	- Pops registers from thread stack
PendSV_Handler:
	
	.asmfunc
	;Implement this
	
	CPSID I											; Disable interrupts while configuring
													; status register, pc, lr, r12, r0-r3 are saved when entering interrupt
	push {r4-r11}									; save r4-r11
	
	ldr r0, RunningPtr								; r0 holds address of Currently Running Thread
	ldr r1, [r0]									; r1 holds value pointed to by Currently Running Thread
	
	str sp, [r1]									; save the stack pointer of the current thread into the tcb
	
	push{r0, lr}									; push lr for C function call
	BL G8RTOS_Scheduler								; branch to C function that will point
	pop {r0, lr}
	ldr r1, [r0] 	
	ldr sp, [r1]									; restore next threads stack pointer
	
	pop {r4-r11}									; restore r4-r11
	
	CPSIE I											; Enable interrupts
	
	bx lr											; Branch to LR, return
	
	.endasmfunc
	
	; end of the asm file
	.align
	.end
