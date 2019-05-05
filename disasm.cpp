#include "disasm.hpp"
#include <iostream>
using std::cout;
using std::endl;

using std::dec;
using std::hex;
#include <iomanip>
using std::setfill;
using std::setw;

#include <sstream>
using std::ostringstream;

#include <string>
using std::string;

static void print_offset(int offset);
static void print_byte_buffer(unsigned char buffer[], int n);
static string reg_to_str(int reg, int reg_size);
static void disasm_mod_r(int byte, string& reg, string& rm, int rex,
                         int reg_size);
static string disasm_imm(unsigned char* program, int& p_offset,
                         unsigned char* buffer, int& b_count, int size);
static string disasm_imm64(unsigned char* program, int& p_offset,
                           unsigned char* buffer, int& b_count);
static string disasm_imm32(unsigned char* program, int& p_offset,
                           unsigned char* buffer, int& b_count);
static string disasm_imm16(unsigned char* program, int& p_offset,
                           unsigned char* buffer, int& b_count);
static string disasm_imm8(unsigned char* program, int& p_offset,
                          unsigned char* buffer, int& b_count);
static void disasm_0f(unsigned char* program, int& offset,
                      unsigned char* byte_buffer, int& b_count,
                      ostringstream& text, int rex, int reg_size);

static void disasm_f6(unsigned char* program, int& offset,
                      unsigned char* byte_buffer, int& b_count,
                      ostringstream& text, int rex, int reg_size);

static void disasm_f7(unsigned char* program, int& offset,
                      unsigned char* byte_buffer, int& b_count,
                      ostringstream& text, int rex, int reg_size);

static void disasm_ff(unsigned char* program, int& offset,
                      unsigned char* byte_buffer, int& b_count,
                      ostringstream& text, int rex, int reg_size);

void disassemble(unsigned char* program, int prog_size) {
	unsigned char byte_buffer[16];
	int b_count = 0;
	int byte;

	ostringstream text;

	int default_reg_size = 32;  // 64 bit mode

	int offset = 0;
	int rex = 0;  // Register extension
	int reg_size;

	string reg, rm;

	while (offset < prog_size) {
		print_offset(offset);
		rex = 0;
		b_count = 0;
		text.str("");
		reg_size = default_reg_size;

		while (1) {
			// process prefixes
			if (program[offset] >= 0x40 && program[offset] <= 0x4f) {
				// Set register extension
				rex = program[offset];
			} else if (program[offset] == 0x66) {
				if (default_reg_size == 32)
					reg_size = 16;
				else
					reg_size = 32;
			} else
				break;  // Not a prefix
			byte_buffer[b_count++] = program[offset++];
		}

		switch ((unsigned char)program[offset]) {
			case 0x01:
				byte_buffer[b_count++] = program[offset++];
				text << "ADD  ";
				disasm_mod_r(program[offset], reg, rm, rex, reg_size);
				byte_buffer[b_count++] = program[offset++];
				text << rm << ", " << reg;
				break;
			case 0x03:
				byte_buffer[b_count++] = program[offset++];
				text << "ADD  ";
				disasm_mod_r(program[offset], reg, rm, rex, reg_size);
				byte_buffer[b_count++] = program[offset++];
				text << reg << ", " << rm;
				break;
			case 0x0f:
				byte_buffer[b_count++] = program[offset++];
				disasm_0f(program, offset, byte_buffer, b_count, text, rex, reg_size);
				break;
			case 0x29:
				byte_buffer[b_count++] = program[offset++];
				text << "SUB  ";
				disasm_mod_r(program[offset], reg, rm, rex, reg_size);
				byte_buffer[b_count++] = program[offset++];
				text << rm << ", " << reg;
				break;
			case 0x2b:
				byte_buffer[b_count++] = program[offset++];
				text << "SUB  ";
				disasm_mod_r(program[offset], reg, rm, rex, reg_size);
				byte_buffer[b_count++] = program[offset++];
				text << reg << ", " << rm;
				break;
			case 0x31:
				byte_buffer[b_count++] = program[offset++];
				text << "XOR  ";
				disasm_mod_r(program[offset], reg, rm, rex, reg_size);
				byte_buffer[b_count++] = program[offset++];
				text << rm << ", " << reg;
				break;
			case 0x33:
				byte_buffer[b_count++] = program[offset++];
				text << "XOR  ";
				disasm_mod_r(program[offset], reg, rm, rex, reg_size);
				byte_buffer[b_count++] = program[offset++];
				text << reg << ", " << rm;
				break;
			case 0x34:
				byte_buffer[b_count++] = program[offset++];
				text << "XOR  ";
				disasm_mod_r(program[offset], reg, rm, rex, 8);
				byte_buffer[b_count++] = program[offset++];
				text << rm << ", " << reg;
				break;
			case 0x39:
				byte_buffer[b_count++] = program[offset++];
				text << "CMP  ";
				disasm_mod_r(program[offset], reg, rm, rex, reg_size);
				byte_buffer[b_count++] = program[offset++];
				text << rm << ", " << reg;
				break;
			case 0x3b:
				byte_buffer[b_count++] = program[offset++];
				text << "CMP  ";
				disasm_mod_r(program[offset], reg, rm, rex, reg_size);
				byte_buffer[b_count++] = program[offset++];
				text << reg << ", " << rm;
				break;
			case 0x50:
			case 0x51:
			case 0x52:
			case 0x53:
			case 0x54:
			case 0x55:
			case 0x56:
			case 0x57:
				byte = program[offset] - 0x50;
				if (rex & 0x04) byte += 8;
				byte_buffer[b_count++] = program[offset++];
				text << "PUSH ";
				text << reg_to_str(byte, 64);
				break;
			case 0x58:
			case 0x59:
			case 0x5a:
			case 0x5b:
			case 0x5c:
			case 0x5d:
			case 0x5e:
			case 0x5f:
				byte = program[offset] - 0x58;
				if (rex & 0x04) byte += 8;
				byte_buffer[b_count++] = program[offset++];
				text << "POP  ";
				text << reg_to_str(byte, 64);
				break;
			case 0x74:
				byte_buffer[b_count++] = program[offset++];
				text << "JE   " << disasm_imm8(program, offset, byte_buffer, b_count);
				break;
			case 0x75:
				byte_buffer[b_count++] = program[offset++];
				text << "JNE  " << disasm_imm8(program, offset, byte_buffer, b_count);
				break;
			case 0x89:
				byte_buffer[b_count++] = program[offset++];
				text << "MOV  ";
				disasm_mod_r(program[offset], reg, rm, rex, reg_size);
				byte_buffer[b_count++] = program[offset++];
				text << rm << ", " << reg;
				break;
			case 0x8b:
				byte_buffer[b_count++] = program[offset++];
				text << "MOV  ";
				disasm_mod_r(program[offset], reg, rm, rex, reg_size);
				byte_buffer[b_count++] = program[offset++];
				text << reg << ", " << rm;
				break;
			case 0x90:
			case 0x91:
			case 0x92:
			case 0x93:
			case 0x94:
			case 0x95:
			case 0x96:
			case 0x97:
				byte = program[offset] - 0x90;
				if (rex & 0x04) byte += 8;
				byte_buffer[b_count++] = program[offset++];
				text << "XCHG ";
				if (rex & 0x08)
					text << "RAX, " << reg_to_str(byte, 64);
				else
					text << reg_to_str(0, reg_size) << ", " << reg_to_str(byte, 32);
				break;
			case 0x98:
				byte_buffer[b_count++] = program[offset++];
				if (rex & 0x08)
					text << "CDQE ";
				else if (reg_size == 32)
					text << "CWDE ";
				else
					text << "CBW  ";
				break;
			case 0x99:
				byte_buffer[b_count++] = program[offset++];
				if (rex & 0x08)
					text << "CQO";
				else if (reg_size == 32)
					text << "CDQ";
				else
					text << "CDW";
				break;
			case 0xa8:
				byte_buffer[b_count++] = program[offset++];
				text << "TEST AL, "
				     << disasm_imm8(program, offset, byte_buffer, b_count);
				break;
			case 0xb8:
			case 0xb9:
			case 0xba:
			case 0xbb:
			case 0xbc:
			case 0xbd:
			case 0xbe:
			case 0xbf:
				byte = program[offset] - 0xb8;
				if (rex & 0x04) byte += 8;
				byte_buffer[b_count++] = program[offset++];
				text << "MOV  ";
				if (rex & 0x08)
					text << reg_to_str(byte, 64) << ", "
					     << disasm_imm64(program, offset, byte_buffer, b_count);
				else
					text << reg_to_str(byte, 32) << ", "
					     << disasm_imm32(program, offset, byte_buffer, b_count);
				break;
			case 0xc3:
				byte_buffer[b_count++] = program[offset++];
				text << "RET";
				break;
			case 0xe9:
				byte_buffer[b_count++] = program[offset++];
				text << "JMP  " << disasm_imm32(program, offset, byte_buffer, b_count);
				break;
			case 0xeb:
				byte_buffer[b_count++] = program[offset++];
				text << "JMP  " << disasm_imm8(program, offset, byte_buffer, b_count);
				break;
			case 0xf6:
				byte_buffer[b_count++] = program[offset++];
				disasm_f6(program, offset, byte_buffer, b_count, text, rex, reg_size);
				break;
			case 0xf7:
				byte_buffer[b_count++] = program[offset++];
				disasm_f7(program, offset, byte_buffer, b_count, text, rex, reg_size);
				break;
			case 0xff:
				byte_buffer[b_count++] = program[offset++];
				disasm_ff(program, offset, byte_buffer, b_count, text, rex, reg_size);
				break;
			default:
				byte_buffer[b_count++] = program[offset++];
				break;
		}

		print_byte_buffer(byte_buffer, b_count);
		cout << "  ";
		cout << text.str();

		cout << endl;
	}
}

static void print_offset(int offset) {
	cout << hex << setfill('0') << setw(6) << offset << setfill(' ') << dec
	     << "  ";
}

static void print_byte_buffer(unsigned char buffer[], int n) {
	const int MAX_N_OPS = 10;
	cout << hex << setfill('0');
	int i;
	for (i = 0; i < n; i++) cout << setw(2) << (int)buffer[i] << " ";
	cout << setfill(' ') << dec;
	for (; i < MAX_N_OPS; i++) cout << "   ";
}

static string reg_to_str(int reg, int reg_size) {
	if (reg_size == 32) {
		switch (reg) {
			case 0:
				return "EAX";
				break;
			case 1:
				return "ECX";
				break;
			case 2:
				return "EDX";
				break;
			case 3:
				return "EBX";
				break;
			case 4:
				return "ESP";
				break;
			case 5:
				return "EBP";
				break;
			case 6:
				return "ESI";
				break;
			case 7:
				return "EDI";
				break;
			default:
				return "???32";
				break;
		}
	} else if (reg_size == 16) {
		switch (reg) {
			case 0:
				return "AX";
				break;
			case 1:
				return "CX";
				break;
			case 2:
				return "DX";
				break;
			case 3:
				return "BX";
				break;
			case 4:
				return "SP";
				break;
			case 5:
				return "BP";
				break;
			case 6:
				return "SI";
				break;
			case 7:
				return "DI";
				break;
			default:
				return "???16";
				break;
		}
	}

	else if (reg_size == 8) {
		switch (reg) {
			case 0:
				return "AL";
				break;
			case 1:
				return "CL";
				break;
			case 2:
				return "DL";
				break;
			case 3:
				return "BL";
				break;
			case 4:
				return "AH";
				break;
			case 5:
				return "CH";
				break;
			case 6:
				return "DH";
				break;
			case 7:
				return "BH";
				break;
			default:
				return "???8";
				break;
		}
	}

	else if (reg_size == 64) {
		switch (reg) {
			case 0:
				return "RAX";
				break;
			case 1:
				return "RCX";
				break;
			case 2:
				return "RDX";
				break;
			case 3:
				return "RBX";
				break;
			case 4:
				return "RSP";
				break;
			case 5:
				return "RBP";
				break;
			case 6:
				return "RSI";
				break;
			case 7:
				return "RDI";
				break;
			default:
				return "???64";
				break;
		}
	}

	return "?????";
}

static void disasm_mod_r(int byte, string& reg, string& rm, int rex,
                         int reg_size) {
	int ireg = (byte & 0x38) >> 3;
	if (rex & 0x04) ireg += 8;

	int irm = byte & 0x07;
	if (rex & 0x01) irm += 8;

	int imod = (byte & 0xc0) >> 6;

	reg = reg_to_str(ireg, reg_size);

	switch (imod) {
		case 0:
			switch (irm) {
				case 4:
				case 5:
					rm = "!!!";
					break;
				default:
					rm = '[' + reg_to_str(irm, reg_size) + ']';
					break;
			}
			break;
		case 3:
			rm = reg_to_str(irm, reg_size);
			break;
		default:
			rm = "!!!";
			break;
	}
}

static string disasm_imm(unsigned char* program, int& p_offset,
                         unsigned char* buffer, int& b_count, int size) {
	ostringstream label;

	for (int i = 0; i < size; i++) buffer[b_count + i] = program[p_offset++];

	label << hex << setfill('0');
	for (int i = size - 1; i >= 0; i--)
		label << setw(2) << (int)buffer[b_count + i];

	b_count += size;

	label << 'H' << setfill('0') << dec;

	return label.str();
}

static string disasm_imm64(unsigned char* program, int& p_offset,
                           unsigned char* buffer, int& b_count) {
	return disasm_imm(program, p_offset, buffer, b_count, 8);
}

static string disasm_imm32(unsigned char* program, int& p_offset,
                           unsigned char* buffer, int& b_count) {
	return disasm_imm(program, p_offset, buffer, b_count, 4);
}

static string disasm_imm16(unsigned char* program, int& p_offset,
                           unsigned char* buffer, int& b_count) {
	return disasm_imm(program, p_offset, buffer, b_count, 2);
}

static string disasm_imm8(unsigned char* program, int& p_offset,
                          unsigned char* buffer, int& b_count) {
	return disasm_imm(program, p_offset, buffer, b_count, 1);
}

static void disasm_0f(unsigned char* program, int& offset,
                      unsigned char* byte_buffer, int& b_count,
                      ostringstream& text, int rex, int reg_size) {
	int byte = program[offset];
	string reg, rm;

	switch (byte) {
		case 0x84:
			byte_buffer[b_count++] = program[offset++];
			text << "JE ";
			if (reg_size == 32)
				text << disasm_imm32(program, offset, byte_buffer, b_count);
			else
				text << disasm_imm16(program, offset, byte_buffer, b_count);
			break;
		case 0x85:
			byte_buffer[b_count++] = program[offset++];
			text << "JNE ";
			if (reg_size == 32)
				text << disasm_imm32(program, offset, byte_buffer, b_count);
			else
				text << disasm_imm16(program, offset, byte_buffer, b_count);
			break;
		case 0x94:
			byte_buffer[b_count++] = program[offset++];
			text << "SETE ";
			disasm_mod_r(program[offset], reg, rm, rex, 8);
			byte_buffer[b_count++] = program[offset++];
			text << rm;
			break;
		case 0x95:
			byte_buffer[b_count++] = program[offset++];
			text << "SETNE ";
			disasm_mod_r(program[offset], reg, rm, rex, 8);
			byte_buffer[b_count++] = program[offset++];
			text << rm;
			break;
		case 0x9c:
			byte_buffer[b_count++] = program[offset++];
			text << "SETL ";
			disasm_mod_r(program[offset], reg, rm, rex, 8);
			byte_buffer[b_count++] = program[offset++];
			text << rm;
			break;
		case 0x9d:
			byte_buffer[b_count++] = program[offset++];
			text << "SETGE ";
			disasm_mod_r(program[offset], reg, rm, rex, 8);
			byte_buffer[b_count++] = program[offset++];
			text << rm;
			break;
		case 0x9e:
			byte_buffer[b_count++] = program[offset++];
			text << "SETLE ";
			disasm_mod_r(program[offset], reg, rm, rex, 8);
			byte_buffer[b_count++] = program[offset++];
			text << rm;
			break;
		case 0x9f:
			byte_buffer[b_count++] = program[offset++];
			text << "SETG ";
			disasm_mod_r(program[offset], reg, rm, rex, 8);
			byte_buffer[b_count++] = program[offset++];
			text << rm;
			break;
		default:
			text << "%%%";
			break;
	}
}

static void disasm_f6(unsigned char* program, int& offset,
                      unsigned char* byte_buffer, int& b_count,
                      ostringstream& text, int rex, int reg_size) {
	int byte = program[offset];

	int ireg = (byte & 0x38) >> 3;

	int irm = byte & 0x07;
	if (rex & 0x01) irm += 8;

	string reg, rm;

	byte_buffer[b_count++] = program[offset++];

	if (rex & 0x08)
		disasm_mod_r(byte, reg, rm, rex, 64);
	else
		disasm_mod_r(byte, reg, rm, rex, reg_size);

	switch (ireg) {
		case 0:
			text << "TEST " << rm << ", "
			     << disasm_imm8(program, offset, byte_buffer, b_count);
			break;
		default:
			text << "unknown f6";
			break;
	}
}

static void disasm_f7(unsigned char* program, int& offset,
                      unsigned char* byte_buffer, int& b_count,
                      ostringstream& text, int rex, int reg_size) {
	int byte = program[offset];

	int ireg = (byte & 0x38) >> 3;

	int irm = byte & 0x07;
	if (rex & 0x01) irm += 8;

	string reg, rm;

	byte_buffer[b_count++] = program[offset++];

	if (rex & 0x08)
		disasm_mod_r(byte, reg, rm, rex, 64);
	else
		disasm_mod_r(byte, reg, rm, rex, reg_size);

	switch (ireg) {
		case 2:
			text << "NOT  " << rm;
			break;
		case 3:
			text << "NEG  " << rm;
			break;
		case 5:
			text << "IMUL EAX, " << rm;
			break;
		case 7:
			text << "IDIV EDX:EAX, " << rm;
			break;
		default:
			text << "unknown f7";
			break;
	}
}

static void disasm_ff(unsigned char* program, int& offset,
                      unsigned char* byte_buffer, int& b_count,
                      ostringstream& text, int rex, int reg_size) {
	int byte = program[offset];

	int ireg = (byte & 0x38) >> 3;

	int irm = byte & 0x07;
	if (rex & 0x01) irm += 8;

	string reg, rm;

	byte_buffer[b_count++] = program[offset++];

	switch (ireg) {
		case 0:
			text << "INC  ";
			disasm_mod_r(byte, reg, rm, rex, 64);
			text << rm;
			break;
		case 1:
			text << "DEC  ";
			disasm_mod_r(byte, reg, rm, rex, 64);
			text << rm;
			break;
		case 2:
			text << "CALL ";
			disasm_mod_r(byte, reg, rm, rex, 64);
			text << rm;
			break;
		default:
			text << "unknown ff";
			break;
	}
}
