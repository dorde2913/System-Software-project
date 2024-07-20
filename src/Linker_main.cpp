#include "../inc/Linker.hpp"


int main(int argc, char* argv[]){
  std::string filename = argv[1];

  Linker linker;

  linker.loadFile(filename);
  linker.printTables();


    
    


  

  
  return 0;
}