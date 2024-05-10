#ifndef _PARSER_H
#define _PARSER_H


#include <fstream>
#include "Line.hpp"


class Parser{
  private:
  std::ifstream input;
  std::string file_name;
  static Parser instance;
  Parser(){}


  public:
  bool end = false;
  static Parser& getInstance(){ return instance;}
  Line parseLine();
  void setFile(std::string fileName);

};

#endif