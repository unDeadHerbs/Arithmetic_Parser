#include "parser.hpp"

Code_Tree parse_expression(std::vector<Token>& tokens);

#define EOF_ERROR(x) Code_Tree(Token(ERROR, -1, -1, x))
#define ERROR(x, t) Code_Tree(Token(ERROR, t.line, t.col, x))

Code_Tree parse_literal(std::vector<Token>& tokens) {
	if (tokens.size()) {
		auto t = tokens[0];
		tokens.erase(tokens.begin(), tokens.begin() + 1);
		if (t.id == INT || t.id == STRING || t.id == REAL)
			return Code_Tree("literal", t);
		throw ERROR("Not a Litteral", t);
	}
	throw EOF_ERROR("parse_ident: Reached end of Token Stream");
}

Code_Tree parse_value(std::vector<Token>& tokens) {
	if (tokens.size()) {
		auto t = tokens[0];
		if (t.id == LPAREN) {
			tokens.erase(tokens.begin(), tokens.begin() + 1);
			auto ret = parse_expression(tokens);
			if (!tokens.size()) throw EOF_ERROR("Expected ')'.");
			t = tokens[0];
			if (t.id != RPAREN) throw ERROR("Expected ')'.", t);
			tokens.erase(tokens.begin(), tokens.begin() + 1);
			return ret;
		} else {
			return parse_literal(tokens);
		}
	}
	throw EOF_ERROR("parse_value: Reached end of Token Stream");
}

Code_Tree parse_expression(std::vector<Token>& tokens) {
	return parse_value(tokens);
}

Code_Tree parse(std::vector<Token>& tokens) {
	try {
		return parse_expression(tokens);
	} catch (Code_Tree err) {
		return err;
	}
}

// expression
//
