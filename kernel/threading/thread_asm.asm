global thread_entry
thread_entry:
;;; the entry point of each thread
; this function will make sure that the stack is correct for leaving the function
; we remember the stack size, which is saved on top by the thread initialisation.
; mov ebx, [esp+4]
; add esp, 8
mov ebx, [esp+4]

; then we move the stack to such a position that it will look like the current parameters in C
add esp, 8

; jump to the function
call [esp-8]

; then we remove all parameters from the stack
add esp, ebx

; we can now restore the stack like in thread_exit but this time return 0
mov eax, [esp]
mov esp, [eax]
popad
popfd
mov eax, 0
ret

global thread_enter:
thread_enter:;(u32* parentESP, u32 threadESP)

;;; save the parent
; save current (parent) cpu data on the stack
pushfd
pushad

; save the parent stack pointer to parentESP
mov eax, [esp+40]
mov [eax], esp

;;; load the thread
; restore its stack from threadESP so we can load from it
mov eax, [esp+44]
mov esp, [eax]

; load the data from the stack
popad
popfd

;the thread eip is now the top value of the stack, so by returning we continue the thread
sti
ret

global thread_finished:
thread_finished:
cli

; the thread has finished on its own. remaining on the stack is a counter of the amount of arguments (in bytes), the arguments themselves, and the parent stack pointer
; first we remove the arguments using the counter
pop eax
add esp, eax

; we can now restore the stack like in thread_exit but this time return 0
mov eax, [esp]
mov esp, [eax]
popad
popfd
mov eax, 0
ret

global thread_exit:
thread_exit:;(u32* parentESP)
; the thread has exited (but is not done computing). so we save its state. first we save the EIP, then the general registers
push dword [esp]
pushfd
pushad

; remember the current stack so we can save it soon
mov edx, esp

; load the parent stack
mov eax, [esp+44]
mov esp, [eax]

; now that we are in the parent stack we can save the current stack pointer again
mov eax, [esp+44]
mov [eax], edx

; restore its data
popad
popfd

; Finally we can return. This will return to the function that originally called thread_enter
; we return 0 because the thread hasn't finished yet
mov eax, 1
ret

extern end_interrupt
extern get_parent_stack
extern is_current_core_in_thread
global thread_interrupt:
thread_interrupt:;()

; disable interrupts for our routine
cli

; make sure that once the thread resumes it instantly executes an iret
push thread_interrupt_continue

; we save the thread state on its stack
pushfd
pushad

; if we are not actually in a thread, leave the routine
call is_current_core_in_thread
cmp eax, 0
je end

; get the parent stack so we can load it
call get_parent_stack

; remember the current stack so we can save it soon
mov edx, esp

; load the parent stack, whose pointer is now in eax
mov esp, [eax]

; now that we are in the parent stack we can save the current stack pointer again
mov eax, [esp+44]
mov [eax], edx

; end the interrupt, we know 33 is the IRQ of a thread interrupt. This will return to the parent, and not the thread that got interrupted
end:
; restore its data
push dword 33
call end_interrupt
add esp, 4
popad
popfd
ret

thread_interrupt_continue:
iret
