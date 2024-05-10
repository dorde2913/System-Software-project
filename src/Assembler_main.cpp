#include "../inc/Assembler.hpp"
#include <fstream>

int main(int argc, char* argv[]){
  if (argc!=4 || std::string(argv[1])!="-o"){return -1;}
  std::string input_file = "tests/"+std::string(argv[3]);
  std::string output = argv[2];

  Assembler a;
  a.startAssembly("./src/test.txt","");
  

  
}