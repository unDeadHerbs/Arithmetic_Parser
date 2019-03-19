#include "code_tree.hpp"
using std::endl;
using std::ostream;
using std::string;
#include <iomanip>
using std::setw;
ostream& operator<<(ostream& o, Code_Tree ct) {
	static int depth = 0;
	for (int i = 0; i < depth; i++) o << "  ";
	o << ct.t.text << "\n";
	// o << ct.name << ct.t << "\n";
	depth++;
	for (auto& e : ct.sub_tokens) o << e;
	depth--;
	return o;
}

string TOKEN_name(TOKEN t) {
#define echocase(x) \
	case x:           \
		return #x
	switch (t) {
		echocase(EOF_T);
		echocase(UNKNOWN);
		echocase(ERROR);
		echocase(PLUS);
		echocase(MINUS);
		echocase(MULT);
		echocase(DIV);
		echocase(EXP);
		echocase(LESS);
		echocase(LESS_EQ);
		echocase(GREATER);
		echocase(GREATER_EQ);
		echocase(EQUAL);
		echocase(NOT_EQUAL);
		echocase(ASSIGN);
		echocase(NOT);
		echocase(LPAREN);
		echocase(RPAREN);
		echocase(LBRACE);
		echocase(RBRACE);
		echocase(LBRACKET);
		echocase(RBRACKET);
		echocase(AND);
		echocase(OR);
		echocase(DOT);
		echocase(AT);
		echocase(COLON);
		echocase(SEMICOLON);
		echocase(COMMA);
		echocase(IDENT);
		echocase(INT);
		echocase(STRING);
		echocase(REAL);
#undef echocase
		default:
			return "bad token";
	}
}

ostream& operator<<(ostream& o, Token t) {
	return o << "|" << setw(11) << TOKEN_name(t.id) << " | '" << t.text << "' at "
	         << t.line << ":" << t.col;
}
