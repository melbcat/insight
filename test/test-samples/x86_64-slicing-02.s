start:	
	mov	$end, %ah
	mov 	%ah, %bh
	mov	%ah, %ch
	jmp	*%rcx
	
end:
	jmp end

