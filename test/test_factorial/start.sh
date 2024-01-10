./assembler -o test1.o test1.s
./assembler -o factorial.o factorial.s
./linker -hex -place=math@0x40000000 -o prog.hex test1.o factorial.o
./emulator prog.hex