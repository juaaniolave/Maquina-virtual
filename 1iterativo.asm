	sys 15
	push bp
	mov bp,sp
	add sp,4
        mov ebx, 5
        mov ecx, 3
        push ebx
        push ecx
        call potencia
        add sp,8
	mov [bp-4],eax
	mov edx,bp
	sub edx,4
	mov ch,4
	mov cl,1
	mov al 1
	sys 2
        stop

potencia: push bp 
        mov bp,sp
        add bp,8
        push eex
        push efx
        push edx
        mov eex,[bp]
        mov efx,[bp+4]
        mov edx,eex

otro:   cmp efx,0
        jz fin
        mul eex,edx
        sub efx,1
        jmp otro

fin:    mov eax,eex
        pop edx
        pop efx
        pop eex
        pop bp
        ret         

                