#include "../inc/Assembler.hpp"
#include <fstream>




int main(int argc, char* argv[]){
  if ((argc!=4 && argc!=5) || std::string(argv[1])!="-o"){
    std::cout<<"error"<<std::endl;
    return -1;
  }
  std::string input_file = "./tests/"+std::string(argv[3]);
  std::string output = argv[2];
  bool print;
  std::string print_s;
  if (argc ==5){
    print_s = argv[4];
  }
  

  if (argc == 5){
    if (print_s == "-p"){
      print = true;
    }
  }
  else {
    print=false;
  }
  std::cout<<"print: "<<print<<std::endl;
  Assembler a;
  a.startAssembly(input_file,output,print);
  

  
}