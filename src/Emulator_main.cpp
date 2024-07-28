#include "../inc/Emulator.hpp"

int main(int argc, char* argv[]){
  std::string input_file = std::string(argv[1]);
  bool debug = false;
  if (argc ==  3) {
    std::string b = argv[2];
    if (b == "-d" || b == "--debug"){
      debug = true;
    }
  }

  Emulator e(input_file);
  e.loadMem();
  e.beginExec(debug);
}