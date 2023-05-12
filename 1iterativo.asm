	sys 15
        mov ebx, 5
        mov ecx, 3
        push ebx
        push ecx
        call potencia
        add sp,8
	mov ch,4
	mov cl,1
	mov al 1
	mov edx,eax
	sys 2
        stop

potencia:        push bp 
        mov bp,sp
        add bp,8
        push eex
        push efx
        mov eex,[bp]
        mov efx,[bp+4]
otro:         cmp efx,0
        jz fin
        mul eex,eex
        sub efx,1
        jmp otro
fin:        mov eax,eex
        pop efx
        pop eex
        pop bp
        ret         

                