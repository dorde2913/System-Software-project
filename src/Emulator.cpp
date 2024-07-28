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

void Emulator::beginExec(bool debug){
  std::vector<unsigned char> bytes = getInstruction(); 
  unsigned int instruction = (unsigned int) bytes[0];
  
  bool halt = false;
  
  while(!halt){
    if (debug){
      std::cin.get();
    }
    std::cout<<"INSTRUKCIJA: "<<(unsigned int)instruction<<std::endl;
    unsigned int reg_a = (bytes[1] & 0xF0)>>4;
    unsigned int reg_b = bytes[1] & 0x0F;
    unsigned int reg_c = (bytes[2] & 0xF0)>>4;
    //std::cout<<((((unsigned int) bytes[2]) & 15)<<8)<<std::endl;
    unsigned int offset =((((unsigned int) bytes[2]) & 15)<<8) + ((unsigned int) bytes[3]);
    

    std::cout<<"Reg A: "<<reg_a<<" Reg B: "<<reg_b <<" Reg C: "<<reg_c<<" Offset: "<<offset<<std::endl;
    printRegisters();
    unsigned int temp;
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
        //regs[1]=pop();//temp
        push(regs[15]);
        push(status);
        
        
       
        cause = 4;
        status = status & (~0x1);
        regs[15] = handler;
        std::cout<<"Izvrsen skok na "<<regs[15]<<std::endl;
        break;
      
      case 32:
        //call direkt i call mem
        std::cout<<"Izvrsen call"<<std::endl;
        address = getOperand();
        push(regs[15]);
        std::cout<<"32 "<<regs[15]<<std::endl;
        if (instruction == 32){
          regs[15] = address;
        }
        /*
        else if (instruction == 33){
          regs[15] = mem[regs[reg_a] + regs[reg_b] + offset];
        } mislim da ovo nemamo
        */
        std::cout<<"Izvrsen skok na "<<regs[15]<<std::endl;
        break;

      case 48://jmp
        address = getOperand();
        regs[15] = address;
        std::cout<<"Izvrsen skok na "<<regs[15]<<std::endl;
        break;
      
      case 49:
        address = getOperand();
        if (regs[reg_b] == regs[reg_c])regs[15] = address;
        
        std::cout<<"Izvrsen skok na "<<regs[15]<<std::endl;
        break;
      
      case 50:
        address = getOperand();
        if (regs[reg_b] != regs[reg_c])regs[15] = address;
        std::cout<<"Izvrsen skok na "<<regs[15]<<std::endl;
        break;
      
      case 51:
        address = getOperand();
        if (regs[reg_b] > regs[reg_c])regs[15] = address;
        std::cout<<"Izvrsen skok na "<<regs[15]<<std::endl;
        break;
      /*
      case 56:
        regs[15] = mem[regs[reg_a] + offset];
        //pop();
        std::cout<<"Izvrsen skok na "<<regs[15]<<std::endl;
        break;

      case 57:
        if (reg_b != reg_c){
          regs[15] = mem[regs[reg_a] + offset];
          regs[reg_a]=pop();
        }
        std::cout<<"Izvrsen skok na "<<regs[15]<<std::endl;
        break;
      
      case 58:
        if (reg_b != reg_c){
          regs[15] = mem[regs[reg_a] + offset];
          regs[reg_a]=pop();
        }
        std::cout<<"Izvrsen skok na "<<regs[15]<<std::endl;
        break;

      case 59:
        if (reg_b != reg_c){
          regs[15] = mem[regs[reg_a] + offset];
          regs[reg_a]=pop();
        }
        std::cout<<"Izvrsen skok na "<<regs[15]<<std::endl;
        break;
      */
      

      case 64:
        temp = regs[reg_b];
        regs[reg_b] = regs[reg_c];
        regs[reg_c] = temp;
        break;
      
      case 80:
        regs[reg_a] = regs[reg_c]+regs[reg_b];
        break;
      
      case 81:
        regs[reg_a] = regs[reg_c] - regs[reg_b];
        break;

      case 82:
        regs[reg_a] = regs[reg_c] * regs[reg_b];
        break;

      case 83:
        regs[reg_a] = regs[reg_c] / regs[reg_b];
        break;

      case 96:
        regs[reg_a] = ~(regs[reg_b]);
        break;
      
      case 97:
        regs[reg_a] = regs[reg_b] & regs[reg_a];
        break;
      
      case 98:
        regs[reg_a] = regs[reg_b] | regs[reg_a];
        break;

      case 99:
        regs[reg_a] = regs[reg_b] ^ regs[reg_a];
        break;

      case 112:
        regs[reg_a] = regs[reg_b]<<regs[reg_c];
        break;
      
      case 113:
        regs[reg_a] = regs[reg_b]>>regs[reg_c];
        break;

      case 128:
        temp_bytes[0] = (regs[reg_c] >> 24) & 0xFF;
        temp_bytes[1] = (regs[reg_c] >> 16) & 0xFF;
        temp_bytes[2] = (regs[reg_c] >> 8) & 0xFF;
        temp_bytes[3] = regs[reg_c] & 0xFF; // Least significant byte
        if (reg_b == 0){
          address = getOperand();
          for (int i=0;i<4;i++){
            mem[address+i] = temp_bytes[i];
            std::cout<<"Na adresu "<<address+i<<" upisano: "<<(unsigned int)temp_bytes[i]<<std::endl;
          }
        }
        else{
          address = regs[reg_b] + offset;
          for (int i=0;i<4;i++){
            mem[address+i] = temp_bytes[i];
            std::cout<<"Na adresu "<<address+i<<" upisano: "<<(unsigned int)temp_bytes[i]<<std::endl;
          }
        }
        

        
        break;
      
      case 130:
        temp_bytes[0] = (regs[reg_c] >> 24) & 0xFF;
        temp_bytes[1] = (regs[reg_c] >> 16) & 0xFF;
        temp_bytes[2] = (regs[reg_c] >> 8) & 0xFF;
        temp_bytes[3] = regs[reg_c] & 0xFF; // Least significant byte
        if (reg_b == 0){
          address = getOperand();
          final_address = 0;
          for (int i=0;i<4;i++){
            final_address+=mem[address+i];
            final_address = final_address<<8;
          }

          for (int i=0;i<4;i++){
            mem[final_address+i] = temp_bytes[i];
          }
        }
        else{
          address = regs[reg_b] + regs[reg_c] + offset;

          for (int i=0;i<4;i++){
            final_address+=mem[address+i];
            final_address = final_address<<8;
          }

          for (int i=0;i<4;i++){
            mem[final_address+i] = temp_bytes[i];
          }
        }
        
        break;
      
      case 129:
        if (reg_b == 15){
          push(regs[reg_b]);
          std::cout<<"pushed pc: "<<regs[reg_b]<<std::endl;
        }
        else{
          push(regs[reg_b]);
        }
        
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
        if (reg_c == 15){
          //znaci da je operand sledeci
          temp = getOperand();
         
          regs[reg_a] = regs[reg_b] + temp;
        }
        else{
          regs[reg_a] = regs[reg_b] + offset;
        }

        break;
      
      case 146:
        if (reg_c == 15){
          //znaci da je operand sledeci
          address = getOperand();
          temp = 0;
          regs[reg_a] = 0;
          for (int i=3;i>=0;i--){
            temp = mem[regs[reg_b]-i+3 + address];
            std::cout<<"Sa adrese "<<regs[reg_b]-i+3 + address<<" procitano: "<<temp<<std::endl;
            temp = temp<<(i*8);
            regs[reg_a]+=temp;
          }
        }
        else{
          temp = 0;
          regs[reg_a] = 0;
          for (int i=3;i>=0;i--){
            temp = mem[regs[reg_c]+regs[reg_b]-i+3 + offset];
            temp = temp<<(i*8);
            //std::cout<<"Temp: "<<temp<<std::endl;
            //std::cout<<"Adresa: "<<(regs[reg_c]+regs[reg_b]-i+3)<<std::endl;
            regs[reg_a]+=temp;
          }
        }
        

        
        break;

      case 147:
        regs[reg_b]=pop();
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

        switch(reg_a){
          case 0:
            status = pop();
            break;
          case 1:
            handler = pop();
            break;
          case 2:
            cause = pop();
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
  //std::cout<<"PC = "<<std::hex<<regs[15]<<std::endl;
  std::vector<unsigned char> ret;
  for (int i=0;i<4;i++){
    ret.push_back(mem[regs[15]+i]);
  }
  regs[15]+=4;
  
  return ret;
}

void Emulator::push(unsigned int content){
  
  unsigned char bytes[4];
  bytes[0] = content & 0xFF;
  bytes[1] = (content >> 8) & 0xFF;
  bytes[2] = (content >> 16) & 0xFF;
  bytes[3] = (content >> 24) & 0xFF; // Least significant byte
  for (int i=1;i<5;i++){
    //std::cout<<"Adresa u koju upisujemo: "<<regs[14]-i<<std::endl;
    mem[(unsigned int)(regs[14]-i)] = bytes[i-1];
  }
  regs[14]-=4;
}

unsigned int Emulator::pop(){
  unsigned int ret = 0;
  ret+=((unsigned int)mem[regs[14]+1])<<16;
  ret+=((unsigned int)mem[regs[14]])<<24;
  ret+=((unsigned int)mem[regs[14]+2])<<8;
  ret+=((unsigned int)mem[regs[14]+3]);
  
  
  regs[14]+=4;
  return ret;
}

void Emulator::printRegisters(){
  std::cout<<std::hex;
  for (int i=0;i<16;i+=4){
    std::cout<<"r"<<i<<"="<<regs[i]<<"     ";
    std::cout<<"r"<<i+1<<"="<<regs[i+1]<<"     ";
    std::cout<<"r"<<i+2<<"="<<regs[i+2]<<"     ";
    std::cout<<"r"<<i+3<<"="<<regs[i+3]<<"     \n";
  }
  std::cout<<"Cause "<<cause<<std::endl;
  std::cout<<"Handler "<<handler<<std::endl;
  std::cout<<"Status "<<status<<std::endl;

  std::cout<<"Mem[SP] "<<(unsigned int)mem[regs[14]]<<"|"
  <<(unsigned int)mem[regs[14]+1]<<"|"<<
  (unsigned int)mem[regs[14]+2]<<"|"<<
  (unsigned int)mem[regs[14]+3]<<"|"<<
  (unsigned int)mem[regs[14]+4]<<"|"<<
  (unsigned int)mem[regs[14]+5]<<"|"<<
  (unsigned int)mem[regs[14]+6]<<"|"<<
  (unsigned int)mem[regs[14]+7]<<"|"<<
  (unsigned int)mem[regs[14]+8]<<"|"<<
  (unsigned int)mem[regs[14]+9]<<"|"<<
  (unsigned int)mem[regs[14]+10]<<"|"<<
  (unsigned int)mem[regs[14]+11]<<"|"<<
  (unsigned int)mem[regs[14]+12]<<"|"<<
  (unsigned int)mem[regs[14]+13]<<"|"<<
  (unsigned int)mem[regs[14]+14]<<"|"<<
  (unsigned int)mem[regs[14]+15]<<"|"<<
  (unsigned int)mem[regs[14]+16]<<"|"<<
  (unsigned int)mem[regs[14]+17]<<"|"<<
  (unsigned int)mem[regs[14]+18]<<"|"<<
  (unsigned int)mem[regs[14]+19]<<"|"<<std::endl;
}

unsigned int Emulator::getOperand(){
  std::cout<<"CITAMO OPERAND SA: "<<regs[15]<<std::endl;
  unsigned int ret = 0;
  unsigned int temp;
  //std::cout<<"citamo operand sa adrese: "<<regs[15]<<" i sledece 3"<<std::endl;
  for (int i=3;i>=0;i--){
    //std::cout<<"Procitan bajt: "<<(unsigned int)mem[regs[15]+i]<<" sa adrese"<<regs[15]+i<<std::endl;
    temp = mem[regs[15]-i+3];
    temp = temp << (8*i);
    ret+=temp;
    //std::cout<<"RET:  "<<ret<<std::endl;
  }
  regs[15]+=4;
  std::cout<<"OPERAND: "<<ret<<std::endl;
  return ret;
}