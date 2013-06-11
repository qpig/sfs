CC=gcc
LD=ld
LDFILE_BOOT=boot/x86.ld
LDFILE_DOS=boot/x86_dos.ld
LDFILE_KERNEL=x86_kernel.ld

OBJCOPY=objcopy
TRIM_FLAGS=-R .pdr -R .comment -R .note -S -O binary 
CFLAGS=-c -g -fno-builtin -Wall -fno-stack-protector -fomit-frame-pointer -I include/
CBOOTFLAGS=-c -fno-builtin -Wall -I boot/include/
CKERNELFLAGS=$(CFLAGS) -I include/
CASMFLAGS=-c -g -fno-builtin -Wall -I include/
LDFLAGS_BOOT=-T$(LDFILE_BOOT)
LDFLAGS_DOS=-T$(LDFILE_DOS)
LDFLAGS_KERNEL=-T$(LDFILE_KERNEL)
	
KERNELNEEDFILE= kernel/console.o kernel/hd.o kernel/i8259.o kernel/keyboard.o kernel/main.o \
				kernel/proc.o kernel/protect.o kernel/sched.o \
				kernel/start.o kernel/systask.o kernel/time.o kernel/tty.o  \
				fs/b_tree.o fs/block_table.o  fs/buffer.o  fs/fs.o  fs/inode.o   \
				fs/k_memory.o  fs/namei.o  fs/open.o fs/read_write.o  \
				fs/super.o  \
				lib/kliba.o  lib/open.o  lib/printf.o  lib/read.o  lib/string.o lib/write.o  \
				lib/getpid.o lib/fork.o  lib/exit.o  lib/wait.o  lib/misc.o lib/vsprintf.o\
				lib/exec.o lib/close.o\
				mm/mm.o mm/fork.o  mm/exec.o
CMDNEEDFILE= cmd/hello.o cmd/ls.o cmd/cat.o cmd/touch.o cmd/write.o
INCLUDE=include/*

.PHONY: all copy image copy clean redo crt

all: boot/boot.bin boot/loader.bin kernel.bin image crt

boot/boot.bin:boot/boot.S boot/include/loader.h boot/include/fat12hdr.h boot/include/lib.h
	$(CC) $(CBOOTFLAGS) boot/boot.S -o boot/boot.o
	$(LD) boot/boot.o -o boot/boot.elf $(LDFLAGS_BOOT)
	$(OBJCOPY) $(TRIM_FLAGS) boot/boot.elf $@

boot/loader.bin:boot/loader.S boot/include/loader.h
	$(CC) $(CBOOTFLAGS)  boot/loader.S -o boot/loader.o
	$(LD) boot/loader.o -o boot/loader.elf $(LDFLAGS_DOS)
	$(OBJCOPY) $(TRIM_FLAGS) boot/loader.elf $@

kernel.bin:kernel/kernel.S $(KERNELNEEDFILE) lib/syscall.o
	$(CC) $(CKERNELFLAGS) -o kernel/kernel.o $<
	$(LD) kernel/kernel.o lib/syscall.o $(KERNELNEEDFILE) -o $@ $(LDFLAGS_KERNEL)
kernel/%.o:kernel/%.c  $(INCLUDE)
	$(CC) $(CKERNELFLAGS) -o $@ $<
lib/%.o:lib/%.s  $(INCLUDE)
	$(CC) $(CKERNELFLAGS) -o $@ $<
lib/%.o:lib/%.c $(INCLUDE)
	$(CC) $(CKERNELFLAGS) -o $@ $<
fs/%.o:fs/%.c $(INCLUDE)
	$(CC) $(CKERNELFLAGS) -o $@ $<
cmd/%.o:cmd/%.c $(INCLUDE)
	$(CC) $(CKERNELFLAGS) -o $@ $<
lib/syscall.o:lib/syscall.S $(INCLUDE)
	$(CC) $(CASMFLAGS) -o $@ $< 

image:boot/boot.bin
	dd if=boot/boot.bin of=image bs=512 count=1 conv=notrunc
crt:$(INCLUDE)
	ar rcs lib/crt.a lib/*.o
cmd:$(CMDNEEDFILE) cmd/start.o crt
	ld -Ttext 0x1000 -o cmd/hello cmd/hello.o cmd/start.o lib/crt.a
	ld -Ttext 0x1000 -o cmd/ls    cmd/ls.o cmd/start.o lib/crt.a
	ld -Ttext 0x1000 -o cmd/cat   cmd/cat.o cmd/start.o lib/crt.a
	ld -Ttext 0x1000 -o cmd/touch cmd/touch.o cmd/start.o lib/crt.a
	ld -Ttext 0x1000 -o cmd/write cmd/write.o cmd/start.o lib/crt.a
	dd if=cmd/hello of=c.img seek=17580544 bs=1 count=32k conv=notrunc
	dd if=cmd/ls 	of=c.img seek=17613312 bs=1 count=32k conv=notrunc
	dd if=cmd/cat   of=c.img seek=17646080 bs=1 count=32k conv=notrunc
	dd if=cmd/touch of=c.img seek=17678848 bs=1 count=32k conv=notrunc
	dd if=cmd/write of=c.img seek=17711616 bs=1 count=32k conv=notrunc
#32768

copy:image boot/loader.bin kernel.bin
	sudo mkdir	/tmp/floppy/;
	sudo mount -o loop image /tmp/floppy/ -o fat=12;
	sudo cp boot/loader.bin /tmp/floppy/;
	strip kernel.bin -o kernel.bin.stripped;
	sudo cp kernel.bin /tmp/floppy/kernel.bin;
	sudo umount /tmp/floppy/;
	sudo rmdir	/tmp/floppy/;

clean:
	rm -rf *.o *.elf *.bin boot/*.o boot/*.bin boot/*.elf kernel/*.o kernel/*.elf kernel/*.bin lib/*.o mm/*.o fs/*.o lib/*.a
redo:	clean all
