OSNAME=ElectroOS
BOOTEFI=output/bootx64.efi

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRCDIR := src
OBJDIR := bin
BUILDDIR = output

SRC = $(call rwildcard,$(SRCDIR),*.cpp)
ASM = $(call rwildcard,$(SRCDIR),*.asm)
OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC))
OBJS += $(patsubst $(SRCDIR)/%.asm, $(OBJDIR)/%_asm.o, $(ASM))
DIRS = $(wildcard $(SRCDIR)/*)

LDS = scripts/kernel.ld
CC = g++
c = gcc
LD = ld
ASMC = nasm

CFLAGS = -ffreestanding -fshort-wchar -mno-red-zone
LDFLAGS = -T $(LDS) -static -Bsymbolic -nostdlib
ASMFLAGS =

all:
	@echo "Please don't use 'make all', refer to the docs for information!"

setup:
	@make clean
	@rm -rf bin gnu-efi bootloader output scripts
	@git clone https://github.com/Absurdponcho/gnu-efi gnu-efi
	@git clone https://github.com/Absurdponcho/OVMFbin OVMFbin
	@rm gnu-efi/bootloader/main.c
	@cd gnu-efi/; make -f Makefile;
	@mkdir bin output scripts
	@mkdir bootloader
	@cd bootloader; wget "https://raw.githubusercontent.com/Absurdponcho/PonchoOS/Episode-16-PCI/gnu-efi/bootloader/main.c";
	@wget "https://raw.github.com/Absurdponcho/PonchoOS/main/kernel/startup.nsh"
	@cd scripts; wget "https://raw.githubusercontent.com/Absurdponcho/PonchoOS/Episode-6-Page-Frame-Allocator/kernel/kernel.ld"
	@rm -rf */.git
	@patch bootloader/main.c patches/main.c.patch
	@patch startup.nsh patches/startup.nsh.patch
bootloader:
	@rm -rf bin output
	@mkdir bin output
	@rm -rf gnu-efi/bootloader/*.c || true
	@mkdir output/efi/boot/ -pv
	@cp bootloader/* gnu-efi/bootloader || true
	@cd gnu-efi; make bootloader
build:
	@rm -rf bin output
	@mkdir bin output
	@make patch
	@rm -rf gnu-efi/bootloader/*.c || true
	@mkdir output/efi/boot/ -pv
	@cp bootloader/* gnu-efi/bootloader || true
	@cd gnu-efi; make bootloader
	@cp gnu-efi/x86_64/bootloader/main.efi output/bootx64.efi
	@mkdir eos-t/ || true
	@mkdir eos-t/efi || true
	@mkdir eos-t/efi/boot || true
	@cp gnu-efi/x86_64/bootloader/main.efi output/efi/boot/bootx64.efi
	#@cp gnu-efi/x86_64/bootloader/main.efi.debug output/bootx64.efi.debug
	@cp output/bootx64.efi /eos-t/efi/boot/bootx64.efi || true
	@cp startup.nsh output/startup.nsh || true
	@cp font.psf output/font.psf

	@make kernel
	@dd if=/dev/zero of=$(BUILDDIR)/$(OSNAME).img bs=512 count=93750
	@mformat -i $(BUILDDIR)/$(OSNAME).img -f 1440 ::
	@mmd -i $(BUILDDIR)/$(OSNAME).img ::/EFI
	@mmd -i $(BUILDDIR)/$(OSNAME).img ::/EFI/BOOT
	@mcopy -i $(BUILDDIR)/$(OSNAME).img $(BOOTEFI) ::/EFI/BOOT
	@mcopy -i $(BUILDDIR)/$(OSNAME).img startup.nsh ::
	@mcopy -i $(BUILDDIR)/$(OSNAME).img $(BUILDDIR)/kernel.elf ::
	@mcopy -i $(BUILDDIR)/$(OSNAME).img font.psf ::
	@mkdir output/iso || true
	@cp output/efi output/iso -r
	@cp output/font.psf output/iso/font.psf -r
	@cp output/kernel.elf output/iso/kernel.elf -r
	@cp output/startup.nsh output/iso/startup.nsh -r
	@make iso
	@rm eos-t/ -rf || true
	@echo "Build complete! Check log for errors."
iso:
	@genisoimage -quiet -V 'ElectroOS' -input-charset iso8859-1 -o output/ElectroOS.iso output/iso/
patch:
	@cd bootloader; rm *; wget "https://raw.githubusercontent.com/Absurdponcho/PonchoOS/Episode-16-PCI/gnu-efi/bootloader/main.c";
	@rm startup.nsh
	@wget "https://raw.github.com/Absurdponcho/PonchoOS/main/kernel/startup.nsh"
	@patch bootloader/main.c patches/main.c.patch
	@patch startup.nsh patches/startup.nsh.patch
patchnd:
	@patch bootloader/main.c patches/main.c.patch
	@patch startup.nsh patches/startup.nsh.patch
run:
	@qemu-system-x86_64 -drive file=output/ElectroOS.img -s -m 256M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="OVMFbin/OVMF_CODE-pure-efi.fd",readonly=on -drive if=pflash,format=raw,unit=1,file="OVMFbin/OVMF_VARS-pure-efi.fd" -net none -machine q35
clean:
	@rm -rf .vscode      || true
	@rm -rf gnu-efi		 || true
	@rm -rf OVMFbin		 || true
	@rm -rf bin    		 || true
	@rm -rf output 		 || true
	@rm -rf startup.nsh  || true
	@rm -rf bootloader   || true
	@rm -rf scripts      || true

kernel: $(OBJS) link

$(OBJDIR)/idt/intrruptes.o: $(SRCDIR)/idt/intrruptes.cpp
	@ echo !==== COMPILING $^
	@ mkdir -p $(@D)
	@$(CC) -mno-red-zone -mgeneral-regs-only $(CFLAGS) -c $^ -o $@
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@ echo !==== COMPILING $^
	@ mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $^ -o $@
$(OBJDIR)/%_asm.o: $(SRCDIR)/%.asm
	@ echo !==== COMPILING $^
	@ mkdir -p $(@D)
	@ $(ASMC) $(ASMFLAGS) $^ -f elf64 -o $@
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@ echo !==== COMPILING $^
	@ mkdir -p $(@D)
	@$(C) $(CFLAGS) -c $^ -o $@
link:
	@ echo !==== LINKING
	@ echo $(OBJS)
	@$(LD) $(LDFLAGS) -o $(BUILDDIR)/kernel.elf $(OBJS)

openvscode:
	@code ./ --user-data-dir='.vscode' --no-sandbox
.PHONY: all openvscode link kernel clean run patchnd iso build bootloader setup patch
