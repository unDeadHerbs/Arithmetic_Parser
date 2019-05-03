#include "parser.hpp"
using std::vector;

Code_Tree parse_expression(vector<Token>& tokens);

#define _FUN std::string(__func__)
#define EOF_ERROR(x) Code_Tree(Token(ERROR, -1, -1, x))
#define ERROR(x, t) \
	Code_Tree("ERROR", Token(ERROR, t.line, t.col, x), {Code_Tree(t)})
#define EAT(t, n) t.erase(t.begin(), t.begin() + n)
#define EAT_OP(t, tokens, op)                                            \
	if (!tokens.size()) throw EOF_ERROR(_FUN + " Expected '" + #op + "'"); \
	auto t = tokens[0];                                                    \
	if (t.id != op) throw ERROR(_FUN + " Expected '" + #op + "'", t);      \
	EAT(tokens, 1);

#define EAT_IDENT(t, tokens, str)                                        \
	if (!tokens.size()) throw EOF_ERROR(_FUN + " Expected '" + str + "'"); \
	auto t = tokens[0];                                                    \
	if (t.id != IDENT) throw ERROR(_FUN + " Expected '" + str + "'", t);   \
	if (t.text != str) throw ERROR(_FUN + " Expected '" + str + "'", t);   \
	EAT(tokens, 1);

#define EAT_STRING(t, tokens)                                       \
	if (!tokens.size()) throw EOF_ERROR(_FUN + " Expected 'string'"); \
	auto t = tokens[0];                                               \
	if (t.id != STRING) throw ERROR(_FUN + " Expected 'string'", t);  \
	EAT(tokens, 1);

#define is_keyword(t) \
	(((t) == "print") || ((t) == "read") || ((t) == "true") || ((t) == "false"))

Code_Tree parse_ident(vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("Expected variable name");
	auto t = tokens[0];
	if (t.id != IDENT) throw ERROR("Expected identifier name", t);
	if (is_keyword(t.text)) throw ERROR("Keywords are not identifiers", t);
	EAT(tokens, 1);
	return Code_Tree("Ident", t, {});
}

Code_Tree parse_number(vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROR");
	auto t = tokens[0];
	if (t.id == INT || t.id == REAL) {
		EAT(tokens, 1);
		return Code_Tree("numeric_literal", t);
	}
	throw ERROR("Not a Litteral", t);
}

Code_Tree parse_value(vector<Token>& tokens) {
	auto toks = tokens;
	try {
		EAT_OP(t, toks, '(');
		auto ret = parse_expression(toks);
		EAT_OP(t1, toks, ')');
		tokens = toks;
		return ret;
	} catch (Code_Tree err1) {
		try {
			auto ret = parse_number(toks);
			tokens = toks;
			return ret;
		} catch (Code_Tree err2) {
			try {
				auto ret = parse_ident(toks);
				tokens = toks;
				return ret;
			} catch (Code_Tree err3) {
				throw Code_Tree(
				    "Expected a parenthetical, number, or ident.",
				    Token(ERROR, -1, -1, "Expected a parenthetical, number, or ident."),
				    {err1, err2, err3});
			}
		}
	}
}

Code_Tree parse_unary_signs(vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROR");
	auto t = tokens[0];
	if (t.id != '+' && t.id != '-') return parse_value(tokens);
	EAT(tokens, 1);
	return Code_Tree("unary", t, {parse_value(tokens)});
}

Code_Tree parse_exp(vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROR");
	auto ret = parse_unary_signs(tokens);
	if (!tokens.size()) return ret;
	auto t = tokens[0];
	if (t.id != '^') return ret;
	EAT(tokens, 1);
	return Code_Tree("exp", t, {ret, parse_exp(tokens)});
}

Code_Tree parse_mul_div_prime(vector<Token>& tokens, Code_Tree accumulator) {
	if (!tokens.size()) return accumulator;
	auto t = tokens[0];
	if (!(t.id == '*' || t.id == '/' || t.text == "mod")) return accumulator;
	EAT(tokens, 1);
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
	EAT(tokens, 1);
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

Code_Tree parse_bool_relation(vector<Token>& tokens) {
	auto fir = parse_expression(tokens);
	auto nzret = Code_Tree("bool_imp", {fir});
	if (!tokens.size()) return nzret;
	auto t = tokens[0];
	if (!((t.id == '<') || (t.id == LESS_EQ) || (t.id == '>') ||
	      (t.id == GREATER_EQ) || (t.id == '=') || (t.id == NOT_EQUAL)))
		return nzret;
	EAT(tokens, 1);
	try {
		auto sec = parse_expression(tokens);  // then eat arith
		return Code_Tree("bool_rel", {fir, Code_Tree(t), sec});
	} catch (Code_Tree err) {
		throw Code_Tree(_FUN + " Expected an arithmetic expresion while parsing",
		                {fir, Code_Tree(t), err});
	}
}

Code_Tree parse_bool_literal(vector<Token>& tokens) {
	try {
		auto tmp = tokens;
		EAT_IDENT(t, tmp, "true");
		tokens = tmp;
		return t;
	} catch (Code_Tree err1) {
		try {
			auto tmp = tokens;
			EAT_IDENT(t, tmp, "false");
			tokens = tmp;
			return t;
		} catch (Code_Tree err2) {
			throw Code_Tree("Expected true or false", {err1, err2});
		}
	}
}

Code_Tree parse_bool(vector<Token>& tokens) {
	try {
		auto tmp = tokens;
		auto ret = parse_bool_literal(tmp);
		tokens = tmp;
		return ret;
	} catch (Code_Tree err1) {
		try {
			auto tmp = tokens;
			auto ret = parse_bool_relation(tmp);
			tokens = tmp;
			return ret;
		} catch (Code_Tree err2) {
			throw Code_Tree("Expected a boolean expresion", {err1, err2});
		}
	}
}

Code_Tree parse_printable(vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("EOF ERROR");
	auto t = tokens[0];
	try {
		auto toks = tokens;
		auto ret = Code_Tree("print_i", {parse_expression(toks)});
		tokens = toks;
		return ret;
	} catch (Code_Tree err) {
		try {
			auto toks = tokens;
			auto ret = Code_Tree("print_s", {parse_string(toks)});
			tokens = toks;
			return ret;
		} catch (Code_Tree err2) {
			try {
				auto toks = tokens;
				auto ret = Code_Tree("print_b", {parse_bool(toks)});
				tokens = toks;
				return ret;
			} catch (Code_Tree err3) {
				throw Code_Tree(
				    "Expected expression, string, or bool.",
				    Token(ERROR, t.line, t.col, "Expected expression or string."),
				    {err, err2, err3});
			}
		}
	}
}

vector<Code_Tree> parse_printables_prime(vector<Token>& tokens,
                                         vector<Code_Tree> accumulator) {
	try {
		EAT_OP(t, tokens, ',');
		auto toks = tokens;
		accumulator.push_back(parse_printable(toks));
		tokens = toks;
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

Code_Tree parse_read_f(vector<Token>& tokens) {
	EAT_IDENT(t1, tokens, "read");
	EAT_OP(t2, tokens, '(');
	auto ret = parse_ident(tokens);
	EAT_OP(t3, tokens, ')');
	EAT_OP(t4, tokens, ';');
	return Code_Tree("Read", t1, {ret});
}

Code_Tree parse_func_f(vector<Token>& tokens) {
	if (!tokens.size())
		throw EOF_ERROR("Expected a call to a function or function-like builtin");
	auto t = tokens[0];
	auto toks = tokens;
	try {
		auto ret = parse_print_f(toks);
		tokens = toks;
		return ret;
	} catch (Code_Tree err1) {
		try {
			auto ret = parse_read_f(toks);
			tokens = toks;
			return ret;
		} catch (Code_Tree err2) {
			throw Code_Tree(
			    "Expected a call to a function or function-like builtin",
			    Token(ERROR, t.line, t.col,
			          "Expected a call to a function or function-like builtin"),
			    {err1, err2});
		}
	}
}

Code_Tree parse_declare(vector<Token>& tokens) {
	EAT_IDENT(t1, tokens, "int4");
	auto ret = parse_ident(tokens);
	EAT_OP(t2, tokens, ';');
	return Code_Tree("Declaration", t1, {ret});
}

Code_Tree parse_assignment(vector<Token>& tokens) {
	auto var = parse_ident(tokens);
	EAT_OP(t1, tokens, ASSIGN);
	auto val = parse_expression(tokens);
	EAT_OP(t2, tokens, ';');
	return Code_Tree("Assignment", t1, {var, val});
}

Code_Tree parse_statement(vector<Token>& tokens) {
	if (!tokens.size()) throw EOF_ERROR("Expected variable name");
	auto t = tokens[0];
	auto toks = tokens;
	try {
		auto ret = parse_func_f(toks);
		tokens = toks;
		return ret;
	} catch (Code_Tree err) {
		try {
			auto ret = parse_declare(toks);
			tokens = toks;
			return ret;
		} catch (Code_Tree err2) {
			try {
				auto ret = parse_assignment(toks);
				// TODO: move assignment to being a normal operator rather than
				// a builtin
				tokens = toks;
				return ret;
			} catch (Code_Tree err3) {
				throw Code_Tree(
				    "Expected a function, decleration, or assignment.",
				    Token(ERROR, t.line, t.col,
				          "Expected a function, decleration, or assignment."),
				    {err, err2, err3});
			}
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
		if (tokens.size()) {
			auto t = tokens[0];
			return Code_Tree("Parsing Error at " + std::to_string(t.line) + ':' +
			                     std::to_string(t.col),
			                 Token(ERROR, t.line, t.col,
			                       "Parsing Error at " + std::to_string(t.line) +
			                           ':' + std::to_string(t.col)),
			                 {err});
		}
		return Code_Tree("Parsing Error - EOF",
		                 Token(ERROR, -1, -1, "Parsing Error - EOF"), {err});
	}
}
