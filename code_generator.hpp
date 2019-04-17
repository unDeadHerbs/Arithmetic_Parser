#ifndef __CODE_GENERATOR_HPP__
#define __CODE_GENERATOR_HPP__
#include "code_tree.hpp"
#include <utility>
#include <string.h>
#include <memory>

typedef unsigned char binary;

struct sym_props{};
struct program;
struct Env{
  char string_table[5000];
  char* end;
  std::vector<program> cleanup;
  std::vector<std::pair<std::string,sym_props>> syms;
public:
  Env():string_table{0},end(string_table){}
  char* push(std::string s){
    strcpy(end,s.c_str());
    auto ret=end;
    while(*end++);
    return ret;
  }
};

struct program{
  std::vector<char> buffer;
  std::shared_ptr<Env> e;
public:
  program(){}
  program(std::vector<char>b):buffer(b){}
  void operator()(); //TODO: add optional parameters
  void init(){e=std::make_shared<Env>();}
  void cleanup(){for(auto &c:e->cleanup)operator+=(c);}
  int size(){return buffer.size();}
  program& operator+=(program p){
    for(auto c:p.buffer)
      buffer.push_back(c);
    return *this;
  }
  program& operator+=(char c){
    buffer.push_back(c);
    return *this;
  }
  program operator+(program p){
    program t(*this);
    return t+=p;
  }
  program operator+(char c){
    program t(*this);
    return t+=c;
  }
  //operator += vec<opcode>
  //operator [] for indexing
  //Add an insert? Nope that's hard.
};
program generate(Code_Tree);

#endif
