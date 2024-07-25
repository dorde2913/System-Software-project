#include "../inc/Emulator.hpp"

int main(int argc, char* argv[]){
  std::string input_file = std::string(argv[1]);

  Emulator e(input_file);
  e.loadMem();
  e.beginExec();
}