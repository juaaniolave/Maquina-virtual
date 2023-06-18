main:               mov [ds], 10
                    mov ah, 3
                    mov al, 1
                    mov ch, 1
                    mov cl, 1
                    mov dh, 1
                    mov dl, 1
                    mov ebx, ds
                    sys 13
                    mov ah, 2
                    mov ebx, es
                    sys 13
                    mov edx, es
                    mov cl, 4
                    mov ch, 4
                    mov al, 1
                    sys 2
                    stop