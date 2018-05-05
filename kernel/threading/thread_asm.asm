global thread_enter:
thread_enter:;(u32* parentESP, u32* threadESP)

;;; save the parent
; save current (parent) cpu data on the stack
pushfd
pushad

; save the parent stack pointer somewhere we can access later
mov eax, [esp+40]
mov [eax], esp

;;; load the thread
; restore its stack so we can load from it
mov eax, [esp+44]
mov esp, [eax]

; load the data from the stack
popad
popfd

;the thread eip is now the top value of the stack, so by returning we continue the thread
sti
ret

; the thread has finished on its own. the only item on stack now is our parent stack pointer.
; we can restore the stack like in thread_exit but this time return 0
cli
mov eax, [esp]
mov esp, [eax]
popad
pop cs
popfd
mov eax, 0
ret

global thread_exit:
thread_exit:;(u32* parentESP)
; the thread has preemted or exited. so we save its state. first we save the EIP, then the general registers
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

; the thread has preemted or exited. so we save its state.
pushfd
pushad

; if we are not actually in a thread, leave the routine
call is_current_core_in_thread
cmp eax, 1
jne end

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