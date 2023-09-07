section .text
    global _start

_start:
    ; Divide the number by 2 using the shr instruction
    shr eax, 1

    ; Multiply the result by 2 using the shl instruction
    shl eax, 1

    ; Compare the result with the original number
    cmp eax, ebx

    ; If the result is equal to the original number, the number is even
    je _is_even

    ; If the result is not equal to the original number, the number is odd
    jmp _is_odd

_is_even:
    ; The number is even, do something here
    ; ...

    ; Exit the program
    mov eax, 1       ; system call for exit
    xor ebx, ebx     ; return code 0
    int 0x80         ; call kernel

_is_odd:
    ; The number is odd, do something here
    ; ...

    ; Exit the program
    mov eax, 1       ; system call for exit
    xor ebx, ebx     ; return code 0
    int 0x80         ; call kernel
