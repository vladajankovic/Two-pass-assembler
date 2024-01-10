CPPA = src/assembly.cpp src/Assembler.cpp src/parser.cpp
CPPL = src/linking.cpp src/Linker.cpp
CPPE = src/emulate.cpp src/Emulator.cpp
INC = -Iinc

assembler: makefile $(CPPA)
	g++ -g -o assembler $(CPPA) $(INC)
	cp ./assembler ./test/nivo-a/
	cp ./assembler ./test/test_factorial/

linker: makefile $(CPPL)
	g++ -g -o linker $(CPPL) $(INC)
	cp ./linker ./test/nivo-a/
	cp ./linker ./test/test_factorial/

emulator: makefile $(CPPE)
	g++ -g -o emulator $(CPPE) $(INC)
	cp ./emulator ./test/nivo-a/
	cp ./emulator ./test/test_factorial/

clean:
	find ./ -name assembler -delete
	find ./ -name linker -delete
	find ./ -name emulator -delete

	find ./test/nivo-a/ -name assembler -delete
	find ./test/nivo-a/ -name linker -delete
	find ./test/nivo-a/ -name emulator -delete
	find ./test/nivo-a/ -name *.o -delete
	find ./test/nivo-a/ -name linkerInfo.txt -delete
	find ./test/nivo-a/ -name *.hex -delete

	find ./test/test_factorial/ -name assembler -delete
	find ./test/test_factorial/ -name linker -delete
	find ./test/test_factorial/ -name emulator -delete
	find ./test/test_factorial/ -name *.o -delete
	find ./test/test_factorial/ -name linkerInfo.txt -delete
	find ./test/test_factorial/ -name *.hex -delete

