#include "sconst.h"

.set    _NR_printx,0
.set	_NR_sendrec,1
.set	INT_VECTOR_SYS_CALL,0x90

.global printx,sendrec

printx:
	mov		$_NR_printx,%eax
	mov		4(%esp),%edx
	int		$INT_VECTOR_SYS_CALL
	ret

sendrec:
	mov		$_NR_sendrec,%eax
	mov		4(%esp),%ebx
	mov		8(%esp),%ecx
	mov		12(%esp),%edx
	int		$INT_VECTOR_SYS_CALL
	ret
