main:           push bp
                mov bp,sp
                sub sp,4
                mov cx,-1
                mov edx, ds
                sys 3  
                push edx
                call slen
                add sp,4
                mov [bp-4],eax
                mov edx, bp
                sub edx, 4
                mov cl,1
                mov ch,4
                mov al,15
                sys 2              
                mov sp,bp
                pop bp

                stop

slen:           push bp
                mov bp,sp
                xor eax, eax
                push ecx
                mov ecx,[bp+8]

sigue:          cmp [ecx],0
                jz fin
                add ecx,1
                add eax,1
                jmp sigue      

fin:            pop ecx
                mov sp,bp
                pop bp
                ret
