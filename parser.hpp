#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include <string>
#include <variant>
#include <vector>
#include "Tokenizer/tokenizer.hpp"
#include "code_tree.hpp"

Code_Tree parse(std::vector<Token>&);

#endif
