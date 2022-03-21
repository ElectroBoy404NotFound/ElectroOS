ElectroOS
An OS made by ElectroBoy404NotFound

How to compile?
NOTE: Before you get started, just note that you can only build on Linux. The build is tested on the latest version of Ubuntu (20.04) only
Step 1:
Clone the repo using: git clone https://github.com/ElectroBoy404NotFound/ElectroOS and cd into it using: cd ElectroOS

Step 2:
Setup the build envronment by installing: nasm, gcc, make, qemu-system-x86_64, aqemu, xorriso, build-essential
You can use the command: sudo apt-get install nasm gcc make qemu-system-x86_64 aqemu xorriso build-essential to install them all at once

Step 3:
CD into the ElectroOS/gnu-efi directory and run the command: make bootloader

Step 4:
CD into the kernel folder using: cd ../kernel and run the command: make kernel && make buildiso && make buildimg

The kernel will now be build and generate both a .img and .iso file within the ElectroOS/kernel/bin directory. 

Step 5:
From within the ElectroOS/kernel directory, enter the command: make runiso to boot the ISO file within Qemu.
You can use the command: make run to boot the .img file instead. (There is no differnce between the .ISO and .IMG files except format)

If you are on Windows, you can run the .ISO and .IMG files by using the "run" and "runiso" batch files.

Known issues:

More then 3GB of ram for the OS causes a reboot
Status: Indentified

Copyright (C) 2022 ElectroBoy404NotFound