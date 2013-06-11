.extern  main
.extern  exit

.text
.global  _start

_start:
	push  %eax
	push  %ecx

	call  main

	push  %eax
	call  exit

	hlt
