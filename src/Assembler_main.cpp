#include "../inc/Parser.hpp"
#include <fstream>

int main(int argc, char* argv[]){
  if (argc!=4 || std::string(argv[1])!="-o"){return -1;}
  Parser& p = Parser::getInstance();
  std::string input_file = "tests/"+std::string(argv[3]);
  std::string output = argv[2];

  p.setFile("./src/test.txt");

  while(!p.end){
    std::cout<<p.parseLine();
  }

  
}