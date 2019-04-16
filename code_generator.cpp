#include "code_generator.hpp"
#include <stdlib.h>
#include <sys/mman.h>
#include <numeric>
using std::accumulate;
using namespace std::string_literals;

#define BUFFER_SIZE 50000
int program::operator()() {
	auto prog =
	    (char*)mmap(NULL /*we don't care where we point to.*/,
	                buffer.size() /*size of buffer*/,
	                PROT_EXEC | PROT_READ | PROT_WRITE /*permissions*/,
	                MAP_PRIVATE | MAP_ANONYMOUS /**/,
	                -1 /*ignored as not a file*/, 0 /*ignored as not a file*/);
	if (!prog) throw "Failed to alocate space";
	int end(0);
	for (auto c : buffer) prog[end++] = c;
	auto ret = ((int (*)())prog)();
	munmap(prog, buffer.size());
	return ret;
}
#define LITTLEENDIAN(x)                                                        \
	program({(char)(((x) >> 0x08 * 0) & 0xFF), (char)(((x) >> 0x08 * 1) & 0xFF), \
	         (char)(((x) >> 0x08 * 2) & 0xFF),                                   \
	         (char)(((x) >> 0x08 * 3) & 0xFF)})
#define _MOVI_EAX program({((char)(0xB8 + 0x0))})

#define ADD_EAX_EDX program({((char)(0x01 + 0x00)), ((char)(0xd0))})
#define SUB_EAX_EDX program({((char)(0x29 + 0x00)), ((char)(0xd0))})
#define MUL_EAX_EDX program({((char)(0x0F)), ((char)(0xAF)), ((char)(0xC2))})
#define DIV_EDAX_ECX program({((char)(0xF7)), ((char)(0xF9))})

#define PUSH_EAX program({((char)(0x50 + 0x00))})
#define PUSH_ECX program({((char)(0x50 + 0x01))})
#define PUSH_EDX program({((char)(0x50 + 0x02))})
#define POP_EAX program({((char)(0x58 + 0x00))})
#define POP_ECX program({((char)(0x58 + 0x01))})
#define POP_EDX program({((char)(0x58 + 0x02))})
#define MOD_EDAX_ECX DIV_EDAX_ECX + PUSH_EDX + POP_EAX
#define _LOADI(x) _MOVI_EAX + LITTLEENDIAN((int)(x))
#define ZERO _LOADI(0)
#define LOADI(x) _LOADI(strtol(x.c_str(), NULL, 10))

program generateI(Code_Tree ct) {
	// TODO: Those throws should get the TOKEN's name.
	if (ct.name == "literal") {
		if (ct.t.id == INT) return program(LOADI(ct.t.text));
		throw "Unusported Literal Type"s;
	}
	if (ct.name == "add") {
		auto p1 = generateI(ct.sub_tokens[0]);
		auto p2 = generateI(ct.sub_tokens[1]);
		if (ct.t.id == '+') return p2 + PUSH_EAX + p1 + POP_EDX + ADD_EAX_EDX;
		if (ct.t.id == '-') return p2 + PUSH_EAX + p1 + POP_EDX + SUB_EAX_EDX;
		throw "Unknown add OP "s + (char)ct.t.id;
	}
	if (ct.name == "mul") {
		auto p1 = generateI(ct.sub_tokens[0]);
		auto p2 = generateI(ct.sub_tokens[1]);
		if (ct.t.id == '*') return p2 + PUSH_EAX + p1 + POP_EDX + MUL_EAX_EDX;
		if (ct.t.id == '/')
			// TODO: properly handel sign extention
			return ZERO + PUSH_EAX + p2 + PUSH_EAX + p1 + POP_ECX + POP_EDX +
			       DIV_EDAX_ECX;
		if (ct.t.text == "mod")
			return ZERO + PUSH_EAX + p2 + PUSH_EAX + p1 + POP_ECX + POP_EDX +
			       MOD_EDAX_ECX;
		throw "Unknown mul OP "s + (char)ct.t.id;
	}
	if (ct.name == "exp") {
		if (ct.t.id == '^') return generateI(ct.sub_tokens[0]);
		throw "Unknown exp OP "s + (char)ct.t.id;
	}
	if (ct.name == "unary") {
		auto p = generateI(ct.sub_tokens[0]);
		if (ct.t.id == '+') return p;
		if (ct.t.id == '-')
			return p + PUSH_EAX + program(ZERO) + POP_EDX + program({SUB_EAX_EDX});
		throw "Unknown Unary Op "s + (char)ct.t.id;
	}
	if (ct.name == "block") {
		return accumulate(ct.sub_tokens.begin(), ct.sub_tokens.end(), program(),
		                  [](program p, Code_Tree c) { return p + generateI(c); });
	}

	throw "Bad Parse Tree - unknown '"s + ct.name + "'";
}
#define RETURN ((char)(0xC3))
program generate(Code_Tree ct) { return generateI(ct) + RETURN; }
