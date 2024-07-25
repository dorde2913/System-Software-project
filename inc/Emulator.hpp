#ifndef _EMULATOR_HPP
#define _EMULATOR_HPP

#include <string>
#include <fstream>
#include <vector>
#include <iostream>


class Emulator{
private:
  unsigned int regs[16] = {0}; //14 je SP, 15 je PC, r0 je 0
  

  
  int cause =0;
  int handler=0;
  int status=0;
  static const uint64_t array_size = 1ULL << 32;
  unsigned char* mem = new unsigned char[array_size];

  std::string input_file;

public:
  Emulator(std::string input_file){
    regs[15] = 0x40000000;
    this->input_file = input_file;
  }

  void loadMem();
  void beginExec();
  int getAdr(std::string line);
  std::vector<unsigned char> getContent(std::string line);

  std::vector<unsigned char> getInstruction();
  void printRegisters();

  //neke instrukcije koje se ponavljaju:
  void push(int content);
  unsigned int pop();
  
};

#endif