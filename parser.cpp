#include "parser.hpp"

Code_Tree parse_expression(std::vector<Token>& tokens);

#define EOF_ERROR(x) Code_Tree(Token(ERROR, -1, -1, x))
#define ERROR(x, t) \
	Code_Tree("ERROR", Token(ERROR, t.line, t.col, x), {Code_Tree(t)})
#define EAT_OP(t, tokens, op)                                                \
	if (!tokens.size()) throw EOF_ERROR(std::string("Expected '") + op + "'"); \
	auto t = tokens[0];                                                        \
	if (t.id != op) throw ERROR(std::string("Expected '") + op + "'", t);      \
	tokens.erase(tokens.begin(), tokens.begin() + 1);

#define EAT_IDENT(t, tokens, str)                                             \
	if (!tokens.size()) throw EOF_ERROR(std::string("Expected '") + str + "'"); \
	auto t = tokens[0];                                                         \
	if (t.id != IDENT) throw ERROR(std::string("Expected '") + str + "'", t);   \
	if (t.text != str) throw ERROR(std::string("Expected '") + str + "'", t);   \
	tokens.erase(tokens.begin(), tokens.begin() + 1);

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

Code_Tree parse_printable(std::vector<Token>& tokens) {
	return parse_expression(tokens);
	// else parse string
}

Code_Tree parse_printables_prime(std::vector<Token>& tokens,
                                 Code_Tree accumulator) {
	try {
		EAT_OP(t, tokens, ',');
		return parse_printables_prime(
		    tokens, Code_Tree("print", t, {accumulator, parse_printable(tokens)}));

	} catch (Code_Tree err) {
		return accumulator;
	}
}
Code_Tree parse_printables(std::vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROR");
	return parse_printables_prime(tokens, parse_printable(tokens));
}

Code_Tree parse_print_f(std::vector<Token>& tokens) {
	EAT_IDENT(t1, tokens, "print");
	EAT_OP(t2, tokens, '(');
	auto ret = parse_printables(tokens);
	EAT_OP(t3, tokens, ')');
	EAT_OP(t4, tokens, ';');
	return Code_Tree("print", t1, {ret});
}

Code_Tree parse_statement(std::vector<Token>& tokens) {
	return parse_print_f(tokens);
}

Code_Tree parse_statements(std::vector<Token>& tokens, Code_Tree accumulator) {
	if (!tokens.size()) return accumulator;
	auto t = tokens[0];
	if (t.id == '}') return accumulator;
	return parse_statements(
	    tokens, Code_Tree("block", t, {accumulator, parse_statement(tokens)}));
}

Code_Tree parse_global_block(std::vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROF");
	return Code_Tree("block",
	                 Token(TOKEN('{'), tokens[0].line, tokens[0].col, "block"),
	                 {parse_statements(tokens, parse_statement(tokens))});
}
Code_Tree parse_block(std::vector<Token>& tokens) {
	EAT_OP(t1, tokens, '{');
	auto ret = parse_global_block(tokens);
	EAT_OP(t2, tokens, '}');
	return ret;
}

Code_Tree parse(std::vector<Token>& tokens) {
	try {
		return parse_global_block(tokens);
	} catch (Code_Tree err) {
		return err;
	}
}
