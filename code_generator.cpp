#include "code_generator.hpp"
#include <stdlib.h>
#include <sys/mman.h>
#include <numeric>
using std::accumulate;
using namespace std::string_literals;

namespace {
#include <iostream>
#include <string>
void print_int(long long int x) { std::cout << x; }
void print_str(char* s) { std::cout << s; }
int read_int() {
	int x;
	std::cin >> x;
	return x;
}
}  // namespace

void program::operator()() {
	auto prog =
	    (char*)mmap(NULL /*we don't care where we point to.*/,
	                buffer.size() /*size of buffer*/,
	                PROT_EXEC | PROT_READ | PROT_WRITE /*permissions*/,
	                MAP_PRIVATE | MAP_ANONYMOUS /**/,
	                -1 /*ignored as not a file*/, 0 /*ignored as not a file*/);
	if (!prog) throw "Failed to alocate space"s;
	int end(0);
	for (auto c : buffer) prog[end++] = c;
	((int (*)())prog)();
	munmap(prog, buffer.size());
}
#define LITTLEENDIAN4(x)                                                       \
	program({(char)(((x) >> 0x08 * 0) & 0xFF), (char)(((x) >> 0x08 * 1) & 0xFF), \
	         (char)(((x) >> 0x08 * 2) & 0xFF),                                   \
	         (char)(((x) >> 0x08 * 3) & 0xFF)})
#define LITTLEENDIAN8(x)                                                       \
	program({(char)(((x) >> 0x08 * 0) & 0xFF), (char)(((x) >> 0x08 * 1) & 0xFF), \
	         (char)(((x) >> 0x08 * 2) & 0xFF), (char)(((x) >> 0x08 * 3) & 0xFF), \
	         (char)(((x) >> 0x08 * 4) & 0xFF), (char)(((x) >> 0x08 * 5) & 0xFF), \
	         (char)(((x) >> 0x08 * 6) & 0xFF),                                   \
	         (char)(((x) >> 0x08 * 7) & 0xFF)})
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
#define POP_RDI program({((char)(0x58 + 0x07))})
#define MOV_RDI_RAX program({((char)0x89), ((char)0xc7)})
#define MOD_EDAX_ECX DIV_EDAX_ECX + PUSH_EDX + POP_EAX
#define _LOADI(x) _MOVI_EAX + LITTLEENDIAN4((int)(x))
#define ZERO _LOADI(0)
#define LOADI(x) _LOADI(strtol(x.c_str(), NULL, 10))
#define _MOVI_RSI program({((char)(0x48)), ((char)(0xbe))})
#define _MOVI_RDI program({((char)(0x48)), ((char)(0xbf))})
#define _MOVI_RAX program({((char)(0x48)), ((char)(0xc8))})
#define LOADLL_RDI(x) _MOVI_RDI + LITTLEENDIAN8((x))
#define LOADLL_RAX(x) _MOVI_RAX + LITTLEENDIAN8((x))
#define LOADLL_STR(x) _MOVI_RDI + LITTLEENDIAN8((x))
#define LOADLL_RSI(x) _MOVI_RSI + LITTLEENDIAN8((x))
#define CALL_RSI program({((char)(0xff)), ((char)(0xd6))})
#define PRINT_EAX_I \
	MOV_RDI_RAX + LOADLL_RSI((long long int)print_int) + CALL_RSI
#define READ_EAX LOADLL_RSI((long long int)read_int) + CALL_RSI
#define MOV_EAX_STACK(d) \
	program({((char)0x8b), ((char)0x45), ((char)((-4 * (d + 1)) & 0xff))})
#define MOV_STACK_EAX(d) \
	program({((char)0x89), ((char)0x45), ((char)((-4 * (d + 1)) & 0xff))})
// mov eax,[ebp-4*d]

program generateI(Code_Tree ct, std::shared_ptr<Env> e) {
	// TODO: Those throws should get the TOKEN's name.
	if (ct.name == "numeric_literal") {
		if (ct.t->id == INT) return LOADI(ct.t->text);
		throw "Unusported Literal Type"s;
	}
	if (ct.name == "add") {
		auto p1 = generateI(ct.sub_tokens[0], e);
		auto p2 = generateI(ct.sub_tokens[1], e);
		if (ct.t->id == '+') return p2 + PUSH_EAX + p1 + POP_EDX + ADD_EAX_EDX;
		if (ct.t->id == '-') return p2 + PUSH_EAX + p1 + POP_EDX + SUB_EAX_EDX;
		throw "Unknown add OP "s + (char)ct.t->id;
	}
	if (ct.name == "mul") {
		auto p1 = generateI(ct.sub_tokens[0], e);
		auto p2 = generateI(ct.sub_tokens[1], e);
		if (ct.t->id == '*') return p2 + PUSH_EAX + p1 + POP_EDX + MUL_EAX_EDX;
		if (ct.t->id == '/')
			// TODO: properly handel sign extention
			return ZERO + PUSH_EAX + p2 + PUSH_EAX + p1 + POP_ECX + POP_EDX +
			       DIV_EDAX_ECX;
		if (ct.t->text == "mod")
			return ZERO + PUSH_EAX + p2 + PUSH_EAX + p1 + POP_ECX + POP_EDX +
			       MOD_EDAX_ECX;
		throw "Unknown mul OP "s + (char)ct.t->id;
	}
	if (ct.name == "exp") {
		if (ct.t->id == '^') return generateI(ct.sub_tokens[0], e);
		throw "Unknown exp OP "s + (char)ct.t->id;
	}
	if (ct.name == "unary") {
		auto p = generateI(ct.sub_tokens[0], e);
		if (ct.t->id == '+') return p;
		if (ct.t->id == '-')
			return p + PUSH_EAX + program(ZERO) + POP_EDX + program({SUB_EAX_EDX});
		throw "Unknown Unary Op "s + (char)ct.t->id;
	}
	if (ct.name == "block") {
		return accumulate(
		    ct.sub_tokens.begin(), ct.sub_tokens.end(), program(),
		    [&e](program p, Code_Tree c) { return p + generateI(c, e); });
	}
	if (ct.name == "print") {
		auto ret = program();
		for (auto c : ct.sub_tokens)
			if (c.name == "print_i")
				ret += generateI(c.sub_tokens[0], e) + PRINT_EAX_I;
			else if (c.name == "print_s") {
				auto p = e->push(c.sub_tokens[0].t->text);
				ret += LOADLL_STR((long long int)p) +
				       LOADLL_RSI((long long int)print_str) + CALL_RSI;
			} else
				throw "Unknown print mode "s + c.name;
		return ret;
	}
	if (ct.name == "Declaration") {
		e->cleanup.push_back(POP_EAX);
		e->syms.push_back(ct.sub_tokens[0].t->text);
		return PUSH_EAX;
	}
	if (ct.name == "Ident") {
		auto var = ct.t->text;
		return MOV_EAX_STACK(e->lookup(var));
	}
	if (ct.name == "Read") {
		auto var = ct.sub_tokens[0].t->text;
		return READ_EAX + MOV_STACK_EAX(e->lookup(var));
	}
	throw "Bad Parse Tree - unknown '"s + ct.name + "'";
}
#define RETURN ((char)(0xC3))
program generate(Code_Tree ct) {
	program r;
	r.init();
	r += generateI(ct, r.e);
	r.cleanup();
	return r + RETURN;
}
