
GLOBAL write_model_specific_register
write_model_specific_register:;(register, high, low)

mov ecx,[esp+4]
mov edx,[esp+8]
mov eax,[esp+12]
wrmsr

ret


GLOBAL read_model_specific_register_lower
read_model_specific_register_lower:;(register)

mov ecx, [esp+4]

ret


GLOBAL read_model_specific_register_higher
read_model_specific_register_higher:;(register)

mov ecx, [esp+4]
mov eax, edx

ret
