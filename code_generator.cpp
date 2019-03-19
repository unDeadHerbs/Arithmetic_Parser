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
#define ADD_EAX_EDX ((char)(0x01)), ((char)(0xd0))
#define SUB_EAX_EDX ((char)(0x29)), ((char)(0xd0))
// that's a hope for 0x29
#define PUSH_EAX ((char)(0x50))
#define POP_EAX ((char)(0x58))
#define POP_EDX ((char)(0x58 + 0x02))
#define ZERO \
	{ (char)0xb8, 0, 0, 0, 0 }
#define LOADI(x) \
	{ MOVI_EAX, LITTLEENDIAN((int)strtol(x.c_str(), NULL, 10)) }

program generateI(Code_Tree ct) {
	if (ct.name == "literal") {
		if (ct.t.id == INT) return program(LOADI(ct.t.text));
		throw "Unusported Literal Type";
	}
	if (ct.name == "add") {
		auto p1 = generateI(ct.sub_tokens[0]);
		auto p2 = generateI(ct.sub_tokens[1]);
		if (ct.t.id == '+')
			return p1 + PUSH_EAX + p2 + POP_EDX + program({ADD_EAX_EDX});
		if (ct.t.id == '-')
			return p2 + PUSH_EAX + p1 + POP_EDX + program({SUB_EAX_EDX});
	}
	if (ct.name == "unary") {
		auto p = generateI(ct.sub_tokens[0]);
		if (ct.t.id == '+') return p;
		if (ct.t.id == '-')
			return p + PUSH_EAX + program(ZERO) + POP_EDX + program({SUB_EAX_EDX});
	}

	throw "Bad Parse Tree";
}
#define RETURN ((char)(0xC3))
program generate(Code_Tree ct) { return generateI(ct) + RETURN; }
