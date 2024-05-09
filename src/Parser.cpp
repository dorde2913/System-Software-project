#include "../inc/Parser.hpp"

Parser Parser::instance;

Line Parser::parseLine(){
  std::string line;
  if (!getline(input,line)){
    end = true;
  }

  return Line(line);

}

void Parser::setFile(std::string fileName){
  this->file_name = fileName;
  this->input.open(file_name);
}