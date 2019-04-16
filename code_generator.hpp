#ifndef __CODE_GENERATOR_HPP__
#define __CODE_GENERATOR_HPP__
#include "code_tree.hpp"
typedef unsigned char binary;
class program{
private:
  std::vector<char> buffer;
public:
  program(){}
  program(std::vector<char>b):buffer(b){}
  void operator()(); //TODO: add optional parameters
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
