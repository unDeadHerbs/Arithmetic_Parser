#include "parser.hpp"

Code_Tree parse_expression(std::vector<Token>& tokens);

#define EOF_ERROR(x) Code_Tree(Token(ERROR, -1, -1, x))
#define ERROR(x, t) Code_Tree(Token(ERROR, t.line, t.col, x))

Code_Tree parse_literal(std::vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROR");
	auto t = tokens[0];
	tokens.erase(tokens.begin(), tokens.begin() + 1);
	if (t.id == INT || t.id == STRING || t.id == REAL)
		return Code_Tree("literal", t);
	throw ERROR("Not a Litteral", t);
}

Code_Tree parse_value(std::vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROR");
	auto t = tokens[0];
	if (t.id != '(') return parse_literal(tokens);
	tokens.erase(tokens.begin(), tokens.begin() + 1);
	auto ret = parse_expression(tokens);
	if (!tokens.size()) throw EOF_ERROR("Expected ')'.");
	t = tokens[0];
	if (t.id != ')') throw ERROR("Expected ')'.", t);
	tokens.erase(tokens.begin(), tokens.begin() + 1);
	return ret;
}

Code_Tree parse_unary_signs(std::vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROR");
	auto t = tokens[0];
	if (t.id != '+' && t.id != '-') return parse_value(tokens);
	tokens.erase(tokens.begin(), tokens.begin() + 1);
	return Code_Tree("unary", t, {parse_value(tokens)});
}

Code_Tree parse_exp(std::vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROR");
	auto ret = parse_unary_signs(tokens);
	if (!tokens.size()) return ret;
	auto t = tokens[0];
	if (t.id != '^') return ret;
	tokens.erase(tokens.begin(), tokens.begin() + 1);
	return Code_Tree("exp", t, {ret, parse_exp(tokens)});
}

Code_Tree parse_mul_div_prime(std::vector<Token>& tokens,
                              Code_Tree accumulator) {
	if (!tokens.size()) return accumulator;
	auto t = tokens[0];
	if (!(t.id == '*' || t.id == '/' || t.text == "mod")) return accumulator;
	tokens.erase(tokens.begin(), tokens.begin() + 1);
	return parse_mul_div_prime(
	    tokens, Code_Tree("mul", t, {accumulator, parse_exp(tokens)}));
}

Code_Tree parse_mul_div(std::vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROR");
	auto ret = parse_exp(tokens);
	return parse_mul_div_prime(tokens, ret);
}

Code_Tree parse_sum_dif_prime(std::vector<Token>& tokens,
                              Code_Tree accumulator) {
	if (!tokens.size()) return accumulator;
	auto t = tokens[0];
	if (!(t.id == '+' || t.id == '-')) return accumulator;
	tokens.erase(tokens.begin(), tokens.begin() + 1);
	return parse_sum_dif_prime(
	    tokens, Code_Tree("add", t, {accumulator, parse_mul_div(tokens)}));
}

Code_Tree parse_sum_dif(std::vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROR");
	auto ret = parse_mul_div(tokens);
	return parse_sum_dif_prime(tokens, ret);
}

Code_Tree parse_expression(std::vector<Token>& tokens) {
	return parse_sum_dif(tokens);
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
