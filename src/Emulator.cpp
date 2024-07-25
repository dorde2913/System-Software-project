#include "../inc/Emulator.hpp"

void Emulator::loadMem(){
  std::ifstream input("./"+input_file);

  for (std::string line;getline(input,line);){
    unsigned int address = getAdr(line);
    std::vector<unsigned char> content = getContent(line);
    int i=0;
    for (char c:content){
     
      mem[address+i] = c;
      i++;
    }
  }
}

void Emulator::beginExec(){
  std::vector<unsigned char> bytes = getInstruction(); 
  unsigned int instruction = (unsigned int) bytes[0];
  
  bool halt = false;

  while(!halt){
    std::cout<<"INSTRUKCIJA: "<<(unsigned int)instruction<<std::endl;
    unsigned int reg_a = (bytes[1] & 0xF0)>>4;
    unsigned int reg_b = bytes[1] & 0x0F;
    unsigned int reg_c = (bytes[2] & 0xF0)>>4;
    //std::cout<<((((unsigned int) bytes[2]) & 15)<<8)<<std::endl;
    unsigned int offset =((((unsigned int) bytes[2]) & 15)<<8) + ((unsigned int) bytes[3]);
    

    std::cout<<"Reg A: "<<reg_a<<" Reg B: "<<reg_b <<" Reg C: "<<reg_c<<" Offset: "<<offset<<std::endl;

    int temp;
    unsigned int final_address;
    unsigned int address;
    char temp_bytes[4];

    switch(instruction){
      case 0:
        //halt
        halt = true;
        break;

      case 16:
        //int
        push(status);
        push(regs[15]);
        cause = 4;
        status = status & (~0x1);
        regs[15] = handler;
        break;
      
      case 32:
      case 33:
        push(regs[15]);
        if (instruction == 32){
          regs[15] = regs[reg_a] + regs[reg_b] + offset;
        }
        else if (instruction == 33){
          regs[15] = mem[regs[reg_a] + regs[reg_b] + offset];
        }
        std::cout<<"PC = "<<std::hex<<regs[15]<<std::endl;
        break;

      case 48:
        regs[15] = regs[reg_a] + offset;
        break;
      
      case 49:
        if (regs[reg_b] == regs[reg_c])regs[15] = regs[reg_a] + offset;
        break;
      
      case 50:
        if (regs[reg_b] != regs[reg_c])regs[15] = regs[reg_a] + offset;
        break;
      
      case 51:
        if (regs[reg_b] > regs[reg_c])regs[15] = regs[reg_a] + offset;
        break;

      case 56:
        regs[15] = mem[regs[reg_a] + offset];
        break;

      case 57:
        if (reg_b != reg_c)regs[15] = mem[regs[reg_a] + offset];
        break;
      
      case 58:
        if (reg_b != reg_c)regs[15] = mem[regs[reg_a] + offset];
        break;

      case 59:
        if (reg_b != reg_c)regs[15] = mem[regs[reg_a] + offset];
        break;

      case 64:
        temp = regs[reg_b];
        regs[reg_b] = regs[reg_c];
        regs[reg_c] = temp;
        break;
      
      case 80:
        regs[reg_a] = regs[reg_b] + regs[reg_c];
        break;
      
      case 81:
        regs[reg_a] = regs[reg_b] - regs[reg_c];
        break;

      case 82:
        regs[reg_a] = regs[reg_b] * regs[reg_c];
        break;

      case 83:
        regs[reg_a] = regs[reg_b] / regs[reg_c];
        break;

      case 96:
        regs[reg_a] = ~(regs[reg_b]);
        break;
      
      case 97:
        regs[reg_a] = regs[reg_b] & regs[reg_c];
        break;
      
      case 98:
        regs[reg_a] = regs[reg_b] | regs[reg_c];
        break;

      case 99:
        regs[reg_a] = regs[reg_b] ^ regs[reg_c];
        break;

      case 112:
        regs[reg_a] = regs[reg_b]<<regs[reg_c];
        break;
      
      case 113:
        regs[reg_a] = regs[reg_b]>>regs[reg_c];
        break;

      case 128:
        address = regs[reg_a] + regs[reg_b] + offset;
        temp_bytes[0] = (regs[reg_c] >> 24) & 0xFF;
        temp_bytes[1] = (regs[reg_c] >> 16) & 0xFF;
        temp_bytes[2] = (regs[reg_c] >> 8) & 0xFF;
        temp_bytes[3] = regs[reg_c] & 0xFF; // Least significant byte

        for (int i=0;i<4;i++){
          mem[address+i] = temp_bytes[i];
        }
        break;
      
      case 130:
        address = regs[reg_a] + regs[reg_b] + offset;
        final_address = 0;

        for (int i=0;i<4;i++){
          temp_bytes[i] = mem[address+i];
        }
        for (int i=0;i<4;i++){
          final_address += temp_bytes[i]<<(i*8);
        }

        temp_bytes[0] = (regs[reg_c] >> 24) & 0xFF;
        temp_bytes[1] = (regs[reg_c] >> 16) & 0xFF;
        temp_bytes[2] = (regs[reg_c] >> 8) & 0xFF;
        temp_bytes[3] = regs[reg_c] & 0xFF; // Least significant byte

        for (int i=0;i<4;i++){
          mem[final_address+i] = temp_bytes[i];
        }
        
        break;
      
      case 129:
        push(regs[reg_c]);
        break;

      case 144:
        switch(reg_b){
          case 0:
            regs[reg_a] = status;
            break;
          case 1:
            regs[reg_a] = handler;
            break;
          case 2:
            regs[reg_a] = cause;
            break;
          default:
            std::cout<<"Error, invalid csr register: "<<reg_b<<std::endl;
            return;
        }
        break;

      case 145:
        std::cout<<"TEST"<<std::endl;
        offset = (((unsigned int) bytes[2])<<8) + ((unsigned int) bytes[3]);
        std::cout<<"OFFSET U LOAD 16BIT: "<<offset<<std::endl;
        regs[reg_a] = regs[reg_a] & 0xFFFF0000;
        regs[reg_a] += ((regs[reg_b]+offset) & 0x0000FFFF);
        std::cout<<"Reg "<<reg_a<<": "<<regs[reg_a]<<std::endl;
        break;
      
      case 146:
        address = regs[reg_b]+regs[reg_c]+offset;
        regs[reg_a] = 0;

        for (int i=0;i<4;i++){
          regs[reg_a] += mem[address+i];
          regs[reg_a] = regs[reg_a] <<8;
        }
        break;

      case 147:
        regs[reg_a]=pop();
        break;
      
      case 148:
        switch(reg_a){
          case 0:
            status = regs[reg_b];
            break;
          case 1:
            handler = regs[reg_b];
            break;
          case 2:
            cause = regs[reg_b];
            break;
          default:
            std::cout<<"Error, invalid csr register: "<<reg_b<<std::endl;
            return;
        }
        break;
      
      case 149:
        switch(reg_a){
          case 0:
            status = regs[reg_b] | offset;
            break;
          case 1:
            handler = regs[reg_b] | offset;
            break;
          case 2:
            cause = regs[reg_b] | offset;
            break;
          default:
            std::cout<<"Error, invalid csr register: "<<reg_b<<std::endl;
            return;
        }
        break;

      case 150:
        address = regs[reg_b] + regs[reg_c] + offset;
        
        switch(reg_a){
          case 0:
            status = 0;
            for (int i=0;i<4;i++){
              status += mem[address+i];
              status = status<<8;
            }
            break;
          case 1:
            handler = 0;
            for (int i=0;i<4;i++){
              handler += mem[address+i];
              handler = handler<<8;
            }
            break;
          case 2:
            cause = 0;
            for (int i=0;i<4;i++){
              cause += mem[address+i];
              cause = cause<<8;
            }
            break;
          default:
            std::cout<<"Error, invalid csr register: "<<reg_b<<std::endl;
            return;
        }
        break;

      case 151:
        address = regs[reg_b] + regs[reg_c] + offset;
        final_address = 0;

        for (int i=0;i<4;i++){
          temp_bytes[i] = mem[address+i];
        }
        for (int i=0;i<4;i++){
          final_address += temp_bytes[i]<<(i*8);
        }

        switch(reg_a){
          case 0:
            status = 0;
            for (int i=0;i<4;i++){
              status += mem[final_address+i];
              status = status<<8;
            }
            break;
          case 1:
            handler = 0;
            for (int i=0;i<4;i++){
              handler += mem[final_address+i];
              handler = handler<<8;
            }
            break;
          case 2:
            cause = 0;
            for (int i=0;i<4;i++){
              cause += mem[final_address+i];
              cause = cause<<8;
            }
            break;
          default:
            std::cout<<"Error, invalid csr register: "<<reg_b<<std::endl;
            return;
        }
        break;
      
      default:
        std::cout<<"ERROR, UNKNOWN INSTRUCTION CODE: "<<instruction<<std::endl;
        return;
    }
    if (!halt){
      bytes = getInstruction();
      instruction = bytes[0];
      printRegisters();
    }
    else{
      std::cout<<"Emulated processor executed halt instgruction"<<std::endl;
      std::cout<<"Emulated processor state: "<<std::endl;

      printRegisters();
      return;
    }
  }

  
}

int Emulator::getAdr(std::string line){
  std::string temp = "";
  char* ptr = &line[0];
  while(*ptr!=':'){
    temp+=(*ptr);
    ptr++;
  }
  //std::cout<<"GETADR "<<temp<<std::endl;
  return stoul(temp,nullptr,16);
}

std::vector<unsigned char> Emulator::getContent(std::string line){
  std::vector<unsigned char> ret;
  char* ptr = &line[0];
  while(*ptr!=':')ptr++;
  ptr++;
  std::string temp = "";
  unsigned int num;
  while(*ptr!='\0'){
    while(*ptr!=' ' && *ptr!='\0' && *ptr!='\n'){
      temp+=(*ptr);
      ptr++;
    }
    //std::cout<<"GETCONTENT "<<temp<<std::endl;
    if (temp!=""){
      num = stoul(temp,nullptr,16);
      ret.push_back((unsigned char)num);
      //std::cout<<num<<std::endl;
      temp = "";
    }
    
    if (*ptr == '\0'){
      break;
    }
    else{
      ptr++;
    }
  }
  
  return ret;
}

std::vector<unsigned char> Emulator::getInstruction(){
  std::cout<<"PC = "<<std::hex<<regs[15]<<std::endl;
  std::vector<unsigned char> ret;
  for (int i=0;i<4;i++){
    ret.push_back(mem[regs[15]+i]);
  }
  regs[15]+=4;
  
  return ret;
}

void Emulator::push(int content){
  char bytes[4];
  bytes[0] = (content >> 24) & 0xFF;
  bytes[1] = (content >> 16) & 0xFF;
  bytes[2] = (content >> 8) & 0xFF;
  bytes[3] = content & 0xFF; // Least significant byte
  for (int i=0;i<4;i++){
    std::cout<<"Adresa u koju upisujemo: "<<regs[14]-i<<std::endl;
    mem[(unsigned int)(regs[14]-i)] = bytes[i];
  }
  regs[14]-=4;
}

unsigned int Emulator::pop(){
  unsigned int ret = mem[regs[14]];
  regs[14]+=4;
  return ret;
}

void Emulator::printRegisters(){
  for (int i=0;i<16;i+=4){
    std::cout<<"r"<<i<<"="<<regs[i]<<"     ";
    std::cout<<"r"<<i+1<<"="<<regs[i+1]<<"     ";
    std::cout<<"r"<<i+2<<"="<<regs[i+2]<<"     ";
    std::cout<<"r"<<i+3<<"="<<regs[i+3]<<"     \n";
  }
  std::cout<<"Cause "<<cause<<std::endl;
  std::cout<<"Handler "<<handler<<std::endl;
  std::cout<<"Status "<<status<<std::endl;
}