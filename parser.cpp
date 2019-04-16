#include "parser.hpp"
using std::vector;

Code_Tree parse_expression(vector<Token>& tokens);

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

#define EAT_STRING(t, tokens)                                            \
	if (!tokens.size()) throw EOF_ERROR(std::string("Expected 'string'")); \
	auto t = tokens[0];                                                    \
	if (t.id != STRING) throw ERROR(std::string("Expected 'string'"), t);  \
	tokens.erase(tokens.begin(), tokens.begin() + 1);

Code_Tree parse_number(vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROR");
	auto t = tokens[0];
	if (t.id == INT || t.id == REAL) {
		tokens.erase(tokens.begin(), tokens.begin() + 1);
		return Code_Tree("numeric_literal", t);
	}
	throw ERROR("Not a Litteral", t);
}

Code_Tree parse_value(vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROR");
	auto t = tokens[0];
	if (t.id != '(') return parse_number(tokens);
	tokens.erase(tokens.begin(), tokens.begin() + 1);
	auto ret = parse_expression(tokens);
	if (!tokens.size()) throw EOF_ERROR("Expected ')'.");
	t = tokens[0];
	if (t.id != ')') throw ERROR("Expected ')'.", t);
	tokens.erase(tokens.begin(), tokens.begin() + 1);
	return ret;
}

Code_Tree parse_unary_signs(vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROR");
	auto t = tokens[0];
	if (t.id != '+' && t.id != '-') return parse_value(tokens);
	tokens.erase(tokens.begin(), tokens.begin() + 1);
	return Code_Tree("unary", t, {parse_value(tokens)});
}

Code_Tree parse_exp(vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROR");
	auto ret = parse_unary_signs(tokens);
	if (!tokens.size()) return ret;
	auto t = tokens[0];
	if (t.id != '^') return ret;
	tokens.erase(tokens.begin(), tokens.begin() + 1);
	return Code_Tree("exp", t, {ret, parse_exp(tokens)});
}

Code_Tree parse_mul_div_prime(vector<Token>& tokens, Code_Tree accumulator) {
	if (!tokens.size()) return accumulator;
	auto t = tokens[0];
	if (!(t.id == '*' || t.id == '/' || t.text == "mod")) return accumulator;
	tokens.erase(tokens.begin(), tokens.begin() + 1);
	return parse_mul_div_prime(
	    tokens, Code_Tree("mul", t, {accumulator, parse_exp(tokens)}));
}

Code_Tree parse_mul_div(vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROR");
	auto ret = parse_exp(tokens);
	return parse_mul_div_prime(tokens, ret);
}

Code_Tree parse_sum_dif_prime(vector<Token>& tokens, Code_Tree accumulator) {
	if (!tokens.size()) return accumulator;
	auto t = tokens[0];
	if (!(t.id == '+' || t.id == '-')) return accumulator;
	tokens.erase(tokens.begin(), tokens.begin() + 1);
	return parse_sum_dif_prime(
	    tokens, Code_Tree("add", t, {accumulator, parse_mul_div(tokens)}));
}

Code_Tree parse_sum_dif(vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROR");
	auto ret = parse_mul_div(tokens);
	return parse_sum_dif_prime(tokens, ret);
}

Code_Tree parse_expression(vector<Token>& tokens) {
	return parse_sum_dif(tokens);
}

Code_Tree parse_string(vector<Token>& tokens) {
	EAT_STRING(t, tokens);
	return Code_Tree("String", t);
}

Code_Tree parse_printable(vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROR");
	try {
		return Code_Tree("print_i", {parse_expression(tokens)});
	} catch (Code_Tree err) {
		try {
			return Code_Tree("print_s", {parse_string(tokens)});
		} catch (Code_Tree err2) {
			throw Code_Tree("Error - Expected expression or string.", {err, err2});
		}
	}
}

vector<Code_Tree> parse_printables_prime(vector<Token>& tokens,
                                         vector<Code_Tree> accumulator) {
	try {
		EAT_OP(t, tokens, ',');
		accumulator.push_back(parse_printable(tokens));
		return parse_printables_prime(tokens, accumulator);
	} catch (Code_Tree err) {
		return accumulator;
	}
}

vector<Code_Tree> parse_printables(vector<Token>& tokens) {
	return parse_printables_prime(tokens, {parse_printable(tokens)});
}

Code_Tree parse_print_f(vector<Token>& tokens) {
	EAT_IDENT(t1, tokens, "print");
	EAT_OP(t2, tokens, '(');
	auto ret = parse_printables(tokens);
	EAT_OP(t3, tokens, ')');
	EAT_OP(t4, tokens, ';');
	return Code_Tree("print", t1, ret);
}

Code_Tree parse_declare(vector<Token>& tokens) {
	EAT_IDENT(t1, tokens, "int4");
	if (!tokens.size()) throw EOF_ERROR("Expected variable name");
	auto t = tokens[0];
	if (t.id != IDENT) throw ERROR("Expected variable name", t);
	tokens.erase(tokens.begin(), tokens.begin() + 1);
	EAT_OP(t2, tokens, ';');
	return Code_Tree("Declaration", t1, {Code_Tree("Variable", t, {})});
}

Code_Tree parse_statement(vector<Token>& tokens) {
	try {
		return parse_print_f(tokens);
	} catch (Code_Tree err) {
		try {
			return parse_declare(tokens);
		} catch (Code_Tree err2) {
			throw Code_Tree("Error - Expected a \"print\" or decleration.",
			                {err, err2});
		}
	}
}

vector<Code_Tree> parse_statements(vector<Token>& tokens,
                                   vector<Code_Tree> accumulator) {
	if (!tokens.size()) return accumulator;
	auto t = tokens[0];
	if (t.id == '}') return accumulator;
	accumulator.push_back(parse_statement(tokens));
	return parse_statements(tokens, accumulator);
}

Code_Tree parse_global_block(vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROF");
	return Code_Tree("block", parse_statements(tokens, {}));
}
Code_Tree parse_block(vector<Token>& tokens) {
	EAT_OP(t1, tokens, '{');
	auto ret = parse_global_block(tokens);
	EAT_OP(t2, tokens, '}');
	return Code_Tree("block", t1, {ret});
}

Code_Tree parse(vector<Token>& tokens) {
	try {
		return parse_global_block(tokens);
	} catch (Code_Tree err) {
		return err;
	}
}
