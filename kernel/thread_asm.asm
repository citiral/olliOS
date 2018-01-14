global thread_enter:
thread_enter:;(u32* parentESP, u32* threadESP)

;;; save the parent
; save current (parent) cpu data on the stack
pushad

; save the parent stack pointer somewhere we can access later
mov eax, [esp+36]
mov [eax], esp

;;; load the thread
; restore its stack so we can load from it
mov eax, [esp+40]
mov esp, [eax]

; load the data from the stack
popad

;load the thread eip, we are now running the thread
pop eax
call eax

; the thread has finished on its own. the only item on stack now is our parent stack pointer.
; we can restore the stack like in thread_exit but this time return 0
mov eax, [esp]
mov esp, [eax]
popad
mov eax, 0
ret

global thread_exit:
thread_exit:;(u32* parentESP)
; the thread has preemted or exited. so we save its state. first we save the EIP, then the general registers
push dword [esp]
pushad

; remember the current stack so we can save it soon
mov edx, esp

; load the parent stack
mov eax, [esp+40]
mov esp, [eax]

; now that we are in the parent stack we can save the current stack pointer again
mov eax, [esp+40]
mov [eax], edx

; restore its data
popad

; Finally we can return. This will return to the function that originally called thread_enter
; we return 0 because the thread hasn't finished yet
mov eax, 1
ret