section .text
global start

start:
    mov al, 7      ; Calculate F(7)
    call fib       ; Result will be in AL
    hlt            ; Stop execution

fib:
    ; Base cases: if n <= 1, return n
    cmp al, 1      ; Compare AL with 1
    jle .return    ; If AL <= 1, return
    
    ; Recursive case: fib(n) = fib(n-1) + fib(n-2)
    push dx        ; Save dx
    push ax        ; Save current n
    
    ; Calculate fib(n-1)
    dec al         ; n = n - 1
    call fib       ; al = fib(n-1)
    mov dl, al     ; Save fib(n-1) in dl
    
    ; Calculate fib(n-2)
    pop ax         ; Restore original n
    dec al         ; First decrement
    dec al         ; Second decrement
    push dx        ; Save fib(n-1) while we calculate fib(n-2)
    call fib       ; al = fib(n-2)
    
    ; Add fib(n-1) + fib(n-2)
    pop dx         ; Restore fib(n-1)
    add al, dl     ; al = fib(n-2) + fib(n-1)
    
    pop dx         ; Restore original dx
    ret

.return:
    ret