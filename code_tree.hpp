#ifndef __CODE_TREE_HPP__
#define __CODE_TREE_HPP__
#include <iostream>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include "Tokenizer/tokenizer.hpp"

struct Code_Tree {
	std::string name;
	std::vector<Code_Tree> sub_tokens;
	std::optional<Token> t;
	friend std::ostream& operator<<(std::ostream&, Code_Tree);
	Code_Tree(Token tok) : t(tok) {}
	Code_Tree(std::string n, Token tok) : name(n), t(tok) {}
	Code_Tree(std::string n, Token tok, std::vector<Code_Tree> vct)
	    : name(n), sub_tokens(vct), t(tok) {}
	Code_Tree(std::string n, std::vector<Code_Tree> vct)
	    : name(n), sub_tokens(vct), t({}) {}
};
#endif
