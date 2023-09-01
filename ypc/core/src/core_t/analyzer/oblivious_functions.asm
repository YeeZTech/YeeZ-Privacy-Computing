BITS 64

section .text
  global omove
	global oset_value
	global oset_flag
	global oset_bytes

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

	mov ebx, dword [rcx]	; Move value in leaf to rbx
	mov r13d, dword [rsi]	; r13d holds array[i]

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

oset_value:
	; oset_value(&dest, value, flag);
	; Linux : rdi,rsi,rdx,rcx,r8,r9
	; Callee-saved : RBP, RBX, and R12–R15

	push rbx
	push rbp
	push r12
	push r13
	push r14
	push r15

	mov ebx, dword [rdi]
	
	cmp edx, 1

	cmovz ebx, esi

	mov dword [rdi], ebx

	pop r15
	pop r14
	pop r13
	pop r12
	pop rbp
	pop rbx

	ret

oset_flag:
	; oset_flag(&dest_flag, src_flag, flag);
	; Linux : rdi,rsi,rdx,rcx,r8,r9
	; Callee-saved : RBP, RBX, and R12–R15

	push rbx
	push rbp
	push r12
	push r13
	push r14
	push r15

	mov bl, byte [rdi]
	
	cmp dl, 1

	cmovz bx, si

	mov byte [rdi], bl

	pop r15
	pop r14
	pop r13
	pop r12
	pop rbp
	pop rbx

	ret

oset_bytes:
	; oset_bytes(dest_bytes.data(), src_bytes.data(), bytes_size, flag)
	; Linux : rdi,rsi,rdx,rcx,r8,r9
	; Callee-saved : RBP, RBX, and R12–R15

	push rbx
	push rbp
	push r12
	push r13
	push r14
	push r15


	; RCX will be lost for loop, store flag from rcx to rbp (1 byte , so bpl)
	mov bpl, cl

	; Oblivious evaluation of flag
	cmp bpl, 1

	mov ecx, edx

	loop_start:
		cmp bpl, 1

		mov r14b, byte [rdi]
		mov r15b, byte [rsi]

		cmovz r14, r15

		mov byte [rdi], r14b

		inc rdi
		inc rsi

		loop loop_start

	pop r15
	pop r14
	pop r13
	pop r12
	pop rbp
	pop rbx

	ret