#include "parser.hpp"

Code_Tree parse_literal(std::vector<Token>& tokens) {
	if (tokens.size()) {
		auto t = tokens[0];
		tokens.erase(tokens.begin(), tokens.begin() + 1);
		if (t.id == INT || t.id == STRING || t.id == REAL)
			return Code_Tree("literal", t);
		throw Code_Tree("Not a Litteral", t);
	}
	throw Code_Tree(
	    Token(ERROR, -1, -1, "parse_ident: Reached end of Token Stream"));
}

Code_Tree parse(std::vector<Token>& tokens) {
	try {
		return parse_literal(tokens);
	} catch (Code_Tree err) {
		return err;
	}
}

// expression
//
