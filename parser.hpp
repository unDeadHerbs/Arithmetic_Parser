#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include <variant>
#include <vector>
#include <string>
#include "code_tree.hpp"
#include "Tokenizer/tokenizer.hpp"

Code_Tree parse(std::vector<Token>&);

#endif
