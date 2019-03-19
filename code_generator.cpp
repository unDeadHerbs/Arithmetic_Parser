#include "code_generator.hpp"
#include <stdlib.h>
#include <sys/mman.h>

#define BUFFER_SIZE 50000
int program::operator()() {
	auto prog =
	    (char*)mmap(NULL /*we don't care where we point to.*/,
	                buffer.size() /*size of buffer*/,
	                PROT_EXEC | PROT_READ | PROT_WRITE /*permissions*/,
	                MAP_PRIVATE | MAP_ANONYMOUS /**/,
	                -1 /*ignored as not a file*/, 0 /*ignored as not a file*/);
	int end(0);
	for (auto c : buffer) prog[end++] = c;
	auto ret = ((int (*)())prog)();
	munmap(prog, buffer.size());
	return ret;
}
#define LITTLEENDIAN(x)                                               \
	(char)(((x) >> 0x08 * 0) & 0xFF), (char)(((x) >> 0x08 * 1) & 0xFF), \
	    (char)(((x) >> 0x08 * 2) & 0xFF), (char)(((x) >> 0x08 * 3) & 0xFF)
#define MOVI_EAX ((char)(0xB8 + 0x0))
#define LOADI(x) \
	{ MOVI_EAX, LITTLEENDIAN((int)strtol(x.c_str(), NULL, 10)) }

program generateI(Code_Tree ct) {
	if (ct.name == "literal")
		if (ct.t.id == INT) return program(LOADI(ct.t.text));
	throw "Bad Parse";
}
#define RETURN ((char)(0xC3))
program generate(Code_Tree ct) { return generateI(ct) + RETURN; }
