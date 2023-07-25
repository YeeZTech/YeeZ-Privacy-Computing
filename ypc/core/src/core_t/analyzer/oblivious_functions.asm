BITS 64

section .text
  global omove

omove:
    ;command:	
		;omove(i,&(array[i]),loc,leaf,newLabel)
		;Linux : rdi,rsi,rdx,rcx,r8,r9

		;If the callee wishes to use registers RBP, RBX, and R12-R15,
		;it must restore their original values before returning control to the caller.
		;All others must be saved by the caller if it wishes to preserve their values

    push rbx
		push rbp
		push r12
		push r13
		push r14
		push r15

		mov r13d, dword [rsi]	; r13d holds array[i]

		mov ebx, dword [rcx]	; Move value in leaf to rbx
		cmp edi, edx		; Compare i and loc

		;if i == loc
		cmovz ebx, dword [rsi]	; move value pointed by rdx to rbx (array[i] to rbx )
		cmovz r13d, r8d		; move newLabel to array[i]

		mov dword [rcx], ebx	; Push in leaflabel/prevleaf to leaf
		mov dword [rsi], r13d	; Push in newLabel/array[i] to array[i]

		pop r15
		pop r14
		pop r13
		pop r12
		pop rbp
		pop rbx

		ret