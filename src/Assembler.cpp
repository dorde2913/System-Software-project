#include "../inc/Assembler.hpp"


void Assembler::startAssembly(std::string input_file,std::string output_file,bool print){
  this->file_name = input_file;
  this->output_name = output_file;

  Parser& parser = Parser::getInstance();

  parser.setFile(input_file);

  while (!Parser::end){
    Line line = parser.parseLine();
    this->addLine(line);
    if (this->end) break;
  }

  bool solved = this->solveSymbols();
  if (!solved || error){
    std::cout<<"solved: "<<solved<<std::endl;
    std::cout<<"error: "<<error<<std::endl;
    
    std::cout<<"ERROR"<<std::endl;
  }
  else{
    if (print)printTables();
    writeToOutput();
  }
  
}

void Assembler::addLine(Line& line){
  line.cleanComments();
  if (line.empty()) return; //linija je imala samo komentar, ignorisemo je

  if (line.hasLabel()){
    //obrada labeli
    //imamo labelu -> to je definicija simbola
    std::string label = line.getLabel();
    handleLabel(label);
  }

  if (line.hasDirective()){
    std::string directive = line.getDirective().first;
    std::vector<std::string> operands = line.getDirective().second;
    handleDirective(directive,operands);
  }

  if (line.hasInstruction()){
    

    std::string instruction = line.getInstruction().first;
    
    std::vector<std::string> operands = line.extractOperands(line.getInstruction().second);
    if (instruction!="iret" && instruction!="call" && instruction != "jmp"
    && instruction != "beq" && instruction != "bne" && instruction != "bgt"
    && instruction!="ld" && instruction !="st")debug_instructions[current_section].push_back(instruction);
    
    //sve instrukcije su velicine 4 bajta, te mozemo odmah da lepo dodamo
    location_counter +=4;
    //U INSTRUKCIJAMA JE SOURCE PA DEST ??
    if (instruction == "halt"){
      //zaustavljanje izvrsavanja, nema operande
      pushInstruction(0,0,0,0);//
      debug_instructions[current_section].push_back(instruction);
    }
    else if (instruction == "int"){
      //softverski prekid, nema op
      pushInstruction(16,0,0,0);//
      debug_instructions[current_section].push_back(instruction);
    }
    else if (instruction == "iret"){
      
      location_counter+=4;
      
      pushInstruction(151,14,0,4);
      debug_instructions[current_section].push_back("pop status (iret)");
      
      pushInstruction(147,239,0,4);
      debug_instructions[current_section].push_back("pop pc (iret)");
    }
    else if (instruction == "call"){
      //jedan operand, adresa funkcije koja se poziva
      //push pc, pc<= operand
      unsigned int sp = 14;
      unsigned int pc = 15;
      sp = sp<<4;
      sp+=pc;

      pushInstruction(32,0,0,0);
      debug_instructions[current_section].push_back("call");

      std::string operand = operands.front();
      pushOperand(pc,operand,instruction);//ovo sad samo baci 4 bajta u content

    }
    else if (instruction == "ret"){
      //nema operanada, radi se pop pc
      //pc<=mem[sp];sp<=sp+4
      pushInstruction(147,239,0,4);
    }
    else if (instruction == "jmp"){
      //pc<=operand, 1 operand
      std::string operand = operands.front();
      unsigned int pc = 15;
      //treba i sama instrukcija da se doda 
      pushInstruction(48,0,0,0);
      debug_instructions[current_section].push_back(instruction);
      pushOperand(pc,operand,instruction);
    }
    else if (instruction == "beq"){
      //2 registra, operand koji predstavlja adresu za skok
      //ako r1==r2, pc<=operand

      //beq r1 r2 operand -> u r0 ucitamo operand i onda ako B=C load pc r0
      std::string reg1 = operands[0];
      std::string reg2 = operands[1];
      std::string operand = operands[2];

      unsigned int reg_code1 = parseRegister(reg1);
      unsigned int reg_code2 = parseRegister(reg2);

      reg_code2 = reg_code2<<4;
      reg_code2+=reg_code1;

      pushInstruction(49,reg_code2,reg_code2,0);
      debug_instructions[current_section].push_back("beq");
      pushOperand(0,operand,instruction);
    }
    else if (instruction == "bne"){
      std::string reg1 = operands[0];
      std::string reg2 = operands[1];
      std::string operand = operands[2];

      unsigned int reg_code1 = parseRegister(reg1);
      unsigned int reg_code2 = parseRegister(reg2);
      
      reg_code2 = reg_code2<<4;
      reg_code2+=reg_code1;

      pushInstruction(50,reg_code2,reg_code2,0);
      debug_instructions[current_section].push_back("beq");

      pushOperand(0,operand,instruction);
    }
    else if (instruction == "bgt"){
      std::string reg1 = operands[0];
      std::string reg2 = operands[1];
      std::string operand = operands[2];

      unsigned int reg_code1 = parseRegister(reg1);
      unsigned int reg_code2 = parseRegister(reg2);
      
      reg_code2 = reg_code2<<4;
      reg_code2+=reg_code1;

      pushInstruction(51,reg_code2,reg_code2,0);
      debug_instructions[current_section].push_back("beq");

      pushOperand(0,operand,instruction);
    }
    else if (instruction == "push"){
      //jedan operand koji je neki registar,
      //sp-=4, mem[sp] = reg
      std::string reg = operands.front();
      unsigned int reg_code = parseRegister(reg);
      //1000 0001, u D upisujemo -4, u A sp, u B registar nas
      unsigned int sp = 14;
      sp = sp<<4;
      sp+=reg_code;

      pushInstruction(129,sp,0,-4);
    }
    else if (instruction == "pop"){
      //jedan operand-registar
      //reg = mem[sp], sp+=4
      std::string reg = operands.front();
      unsigned int reg_code = parseRegister(reg);
      unsigned int sp = 14;
      sp = sp<<4;
      sp+=reg_code;
      
      pushInstruction(147,sp,0,4);
    }
    else if (instruction == "xchg"){
      //2 operanada registri oba,
      //temp = r1, r1 = r2, r2=temp -> zamena vrednosti 2 registra
      //0100 0000 0000 BBBB CCCC 0000 0000

      unsigned int r1_code = parseRegister(operands[0]);
      unsigned int r2_code = parseRegister(operands[1]);
      r2_code = r2_code<<4;
      
      pushInstruction(64,r1_code,r2_code,0);
    }
    else if (instruction == "add"){
      //2 registra
      //r1 = r1+r2
      //0101 0000 AAAA BBBB CCCC 0000 0000 0000
      //A->r2, B->r1, C->r2
      unsigned int r1_code = parseRegister(operands[0]);
      unsigned int r2_code = parseRegister(operands[1]);
      unsigned int c_code = (r2_code<<4) & 0xFF;
       

      r2_code = (r2_code<<4) & 0xF0;
      r2_code+=(r1_code & 0x0F);

      pushInstruction(80,r2_code,c_code,0);
    }
    else if (instruction == "sub"){
      //2 registra
      //r1 = r1-r2
      unsigned int r1_code = parseRegister(operands[0]);
      unsigned int r2_code = parseRegister(operands[1]);
      unsigned int c_code = (r2_code<<4) & 0xFF;
       

      r2_code = (r2_code<<4) & 0xF0;
      r2_code+=(r1_code & 0x0F);

      pushInstruction(81,r2_code,c_code,0);
    }
    else if (instruction == "mul"){
      //2 reg, r1 = r1*r2
      unsigned int r1_code = parseRegister(operands[0]);
      unsigned int r2_code = parseRegister(operands[1]);
      unsigned int c_code = (r2_code<<4) & 0xFF;
       

      r2_code = (r2_code<<4) & 0xF0;
      r2_code+=(r1_code & 0x0F);

      pushInstruction(82,r2_code,c_code,0);
    }
    else if (instruction == "div"){
      //2 reg, r1 = r1/r2
      unsigned int r1_code = parseRegister(operands[0]);
      unsigned int r2_code = parseRegister(operands[1]);
      unsigned int c_code = (r2_code<<4) & 0xFF;
       

      r2_code = (r2_code<<4) & 0xF0;
      r2_code+=(r1_code & 0x0F);

      pushInstruction(83,r2_code,c_code,0);
    }
    else if (instruction == "not"){
      //1 reg, r1 = !r1
      // 0110 0000 AAAA BBBB CCCC 0000 0000 0000
      // ovde je c 0, znaci zadnja 2 su 0
      unsigned int r1_code = parseRegister(operands[0]);
      unsigned int r2_code = parseRegister(operands[1]);
      r2_code = (r2_code<<4) & 0xF0;
      r2_code+=(r1_code & 0x0F);

      pushInstruction(96,r2_code,0,0);
    }
    else if (instruction == "and"){
      //2 reg, r1= r1&r2
      //ovde c == A
      unsigned int r1_code = parseRegister(operands[0]);
      unsigned int r2_code = parseRegister(operands[1]);
      r2_code = (r2_code<<4) & 0xF0;
      r2_code+=(r1_code & 0x0F);

      pushInstruction(97,r2_code,0,0);
    }
    else if (instruction == "or"){
      //2 reg , r1 = r1|r2
      unsigned int r1_code = parseRegister(operands[0]);
      unsigned int r2_code = parseRegister(operands[1]);
      r2_code = (r2_code<<4) & 0xF0;
      r2_code+=(r1_code & 0x0F);

      pushInstruction(98,r2_code,0,0);
    }
    else if (instruction == "xor"){
      //2 reg, r1 = r1^r2
      unsigned int r1_code = parseRegister(operands[0]);
      unsigned int r2_code = parseRegister(operands[1]);
      r2_code = (r2_code<<4) & 0xF0;
      r2_code+=(r1_code & 0x0F);

      pushInstruction(99,r2_code,0,0);
    }
    else if (instruction == "shl"){
      //2 registra
      //r2 = r2<r1
      //0111 0000 AAAA BBBB CCCC 0000 0000 0000
      //A->r2, B->r1, c->r2
      unsigned int r1_code = parseRegister(operands[0]);
      unsigned int r2_code = parseRegister(operands[1]);
      unsigned int c_code = r2_code<<4;
      r2_code = r2_code<<4;
      r2_code+=r1_code;

      pushInstruction(112,r2_code,c_code,0);
    }
    else if (instruction == "shr"){
      //2 registra
      unsigned int r1_code = parseRegister(operands[0]);
      unsigned int r2_code = parseRegister(operands[1]);
      unsigned int c_code = r2_code<<4;
      r2_code = r2_code<<4;
      r2_code+=r1_code;
      
      pushInstruction(113,r2_code,c_code,0);
    }
    /*
    Ovde treba da se napravi prvo funkcija za parsiranje operanada za rad
    sa memorijom
    */
    else if (instruction == "ld" || instruction == "st"){
      location_counter-=4;//malo glupo ali stagod
      //ld operand, %gprD
      int reg_index;
      int operand_index;
      if (instruction == "ld"){
        reg_index = 1;
        operand_index = 0;
      }
      else{
        reg_index = 0;
        operand_index = 1;
      }
    
      int reg_code = parseRegister(operands[reg_index]);

      if (reg_code == -1){
        error = 1;
        end = 1;
        return;
      }
     int mem_type = getMemType(operands[operand_index]);
     
     /*
      0 - immed sym
      1 - immed literal
      2 - mem sym
      3 - mem literal 
      4 - regdir
      5 - regind
      6 - regindpom symbol
      7 - regindpom literal
      -1 - error
      */
    
     int value; 
     int tempval;
     std::string op;
    
    //ako je immed symbol/literal vraca vrednost simbola/literala
    op = removeDollar(operands[operand_index]);

     switch(mem_type){
      case 0:
        //immed symbol
        if (instruction == "ld"){
          pushInstruction(145,reg_code<<4,15<<4,0);
        }
        else if (instruction == "st"){
          pushInstruction(128,0,reg_code<<4,0);
        }
        debug_instructions[current_section].push_back(instruction);
        location_counter+=4;
        pushOperand(0,op,instruction);
        
        break;
      case 1:
        //immed literal

        if (instruction == "ld"){
          pushInstruction(145,reg_code<<4,15<<4,0);
        }
        else if (instruction == "st"){
          pushInstruction(128,0,reg_code<<4,0);
        }
        debug_instructions[current_section].push_back(instruction);
        location_counter+=4;
        pushOperand(0,op,instruction);
        
        break;
      case 2:
        //mem symbol

        if (instruction == "ld"){
          pushInstruction(146,reg_code<<4,15<<4,0);
        }
        else if (instruction == "st"){
          pushInstruction(128,0,reg_code<<4,0);
        }
        debug_instructions[current_section].push_back(instruction);
        location_counter+=4;
        pushOperand(0,op,instruction);

        break;
      case 3:
        //mem literal
        if (instruction == "ld"){
          pushInstruction(146,reg_code<<4,15<<4,0);
        }
        else if (instruction == "st"){
          pushInstruction(128,0,reg_code<<4,0);
        }
        debug_instructions[current_section].push_back(instruction);
        location_counter+=4;
        pushOperand(0,op,instruction);
        
        break;
      case 4:
        //reg
        value = parseMemoryOperand(operands[operand_index],mem_type);
        if (instruction == "ld") {
          pushInstruction(145,(reg_code<<4) + value,0,0);
        }
        else if (instruction == "st"){
          pushInstruction(145,(value<<4)+reg_code,0,0);
        } 
        debug_instructions[current_section].push_back(instruction);
        location_counter+=4;
        
        break;
      case 5:
        //regind 1001 0010 DEST SRC 0000 0000 0000 0000
        value = parseMemoryOperand(operands[operand_index],mem_type);
    
        if (instruction == "ld") {
          pushInstruction(146,(reg_code<<4) + value,0,0);
        }
        else if (instruction == "st") {
          pushInstruction(128,value,reg_code<<4,0);
        }
        debug_instructions[current_section].push_back(instruction);
        location_counter+=4;

        break;
      case 6:
        //regind sym
        // Ovde je obavezno da se poznaje vrednost simbola prilikom asembliranja, sto
        //u nasem slucaju nije moguce jer nije implementirana .equ direktiva
        std::cout<<"Error, non-supported addressing type"<<std::endl;
        break;
      case 7:
        //regind literal 1001 0010 DEST SRC 0000 OFFSET OFFSET OFFSET
        value = parseMemoryOperand(operands[operand_index],mem_type);
        tempval = value<<20;
        tempval = tempval>>20;
        if (instruction == "ld"){
          pushInstruction(146,(reg_code<<4) + (value>>16),tempval>>8,tempval&255);
        } 
        else if (instruction == "st") {
          pushInstruction(128,value>>16,(reg_code<<4) + tempval>>8,tempval & 255);
        }
        debug_instructions[current_section].push_back(instruction);
        location_counter+=4;

        break;
      default:
        std::cout<<"ERROR - BAD ADDRESSING"<<std::endl;
        end = true;
        error = true;
        return;
     }
    }
    else if (instruction == "csrrd"){
      //2 reg
      //1001 0000
      int r1_code = parseRegister(operands[0]);
      int r2_code = parseRegister(operands[1]);
      
      r2_code = r2_code<<4;
      r2_code+=r1_code;
      pushInstruction(144,r2_code,0,0);
      debug_instructions[current_section].push_back(instruction);
    }
    else if (instruction == "csrwr"){
      //2 reg
      //1001 0100 AAAA BBBB
      int r1_code = parseRegister(operands[0]);
      int r2_code = parseRegister(operands[1]);
      
      r2_code = r2_code<<4;
      r2_code+=r1_code;
      pushInstruction(148,r2_code,0,0);
      debug_instructions[current_section].push_back(instruction);
    }
    else{
      //greska
      std::cout<<"ERROR - BAD INSTRUCTION"<<std::endl;
    }
  }
}


bool Assembler::solveSymbols(){
  for (auto& symbol:symbol_table){
    if (!symbol.second.defined) return false;
  }
  return true;
}


void Assembler::printTables(){
  std::cout << "--------------------------------------------------"<<std::endl;
  std::cout<<"Symbol table: "<<std::endl;
  std::cout << "--------------------------------------------------"<<std::endl;
  std::cout << std::left
              << std::setw(25) << "Name"
              << std::setw(10) << "Value"
              << std::setw(10) << "Size"
              << std::setw(10) << "Type"
              << std::setw(10) << "Global"
              << std::setw(20) << "Section"
              << std::setw(10) << "Defined"
              << std::setw(10) << "Extern"
              << std::endl;

  for (auto& entry:symbol_table) {
      std::cout << std::left
                << std::setw(25) << entry.first
                << std::setw(10) << entry.second.value
                << std::setw(10) << entry.second.size
                << std::setw(10) << entry.second.type
                << std::setw(10) << entry.second.is_global
                << std::setw(20) << entry.second.section
                << std::setw(10) << entry.second.defined
                << std::setw(10) << entry.second.is_extern
                << std::endl;
  }
  std::cout << "--------------------------------------------------"<<std::endl;
  std::cout << "Relocation tables: "<<std::endl;
  std::cout << "--------------------------------------------------"<<std::endl;
  for(auto& table:relocation_table){
    std::cout << std::left
                << std::setw(25) << table.first << std::endl;
    std::cout << std::left
              << std::setw(10) << "Addend"
              << std::setw(10) << "Offset"
              << std::setw(10) << "Symbol"
              << std::endl;
    for (auto& entry: table.second ){
      std::cout << std::left
                << std::setw(10) << entry.addend
                << std::setw(10) << entry.offset
                << std::setw(10) << entry.symbol
                << std::endl;
    }
  }
  std::cout << "--------------------------------------------------"<<std::endl;
  std::cout<<"section contents"<<std::endl;
  std::cout << "--------------------------------------------------"<<std::endl;
  /*
  sve instrukcije po 4B
  */
  int count = 0;
  for (auto& section:section_contents){
    count = 0;
    std::cout<<section.first<<std::endl;
    int i=0;
    for (auto& b:section.second){
      if (count!=0 && count%32 == 0){
        std::cout<<std::endl;
      }
      std::cout<<std::bitset<8>(b)<<" ";
      count+=8;
      if (count!=0 && count%32 == 0){
        std::cout<<debug_instructions[section.first][i];
        i++;
      }
    }
    std::cout<<std::endl;
  }
}


int Assembler::parseRegister(std::string register_name){
  //treba da se doda za cause i jos neke
  //std::cout<<"Parsiramo registar: "<<register_name<<std::endl;
  std::string temp = "";
  char* ptr = &register_name[0];
  while(*ptr!='\0'){
    if (*ptr!=' ')temp+=(*ptr);
    ptr++;
  }

  if (temp == "%cause") return 2; //ovo nije ista instrukcija tkd moze da se preklapa valjda
  if (temp == "%handler") return 1;
  if (temp == "%status") return 0;

  if (temp == "%sp") return 14;
  if (temp == "%pc") return 15;

  if (temp == "%r0") return 0;
  if (temp == "%r1") return 1;
  if (temp == "%r2") return 2;
  if (temp == "%r3") return 3;
  if (temp == "%r4") return 4;
  if (temp == "%r5") return 5;
  if (temp == "%r6") return 6;
  if (temp == "%r7") return 7;
  if (temp == "%r8") return 8;
  if (temp == "%r9") return 9;
  if (temp == "%r10") return 10;
  if (temp == "%r11") return 11;
  if (temp == "%r12") return 12;
  if (temp == "%r13") return 13;
  if (temp == "%r14") return 14;
  if (temp == "%r15") return 15;
  return -1;
}

int Assembler::getMemType(std::string operand){
  /*
  0 - immed sym
  1 - immed literal
  2 - mem sym
  3 - mem literal 
  4 - regdir
  5 - regind
  6 - regindpom symbol
  7 - regindpom literal
  -1 - error
  
  memdir -> $literal, $symbol
  memind -> literal / symbol
  regdir -> %reg
  regind -> [%reg]
  regindpom -> [%reg + simbol/literal]

  * svuda mogu da budu ili simbol ili literal, ako je simbol mora biti definisan i manji od 2^12 

  */
  std::regex immediate_symbol_pattern(R"(\$?[a-zA-Z_][a-zA-Z0-9_]*)");
  std::regex immediate_literal_pattern(R"(\$?(?:0x[0-9a-fA-F]+|[0-9]+))");
  
  std::regex register_pattern(R"(%[a-zA-Z0-9_]+)");
  std::regex memory_register_pattern(R"(\[%[a-zA-Z0-9_]+\])");
  std::regex memory_register_symbol_pattern(R"(\[%[a-zA-Z0-9_]+\+[a-zA-Z_][a-zA-Z0-9_]*\])");

  std::regex memory_register_literal_pattern(R"(\[\s*%\w+\s*\+\s*(?:0x[0-9a-fA-F]+|[0-9]+)\s*\])");


  if (std::regex_match(operand, immediate_symbol_pattern)) {
      return operand[0] == '$' ? 0 : 2;
  } else if (std::regex_match(operand, immediate_literal_pattern)) {
      return operand[0] == '$' ? 1 : 3;
  } else if (std::regex_match(operand, register_pattern)) {
      return 4;
  } else if (std::regex_match(operand, memory_register_pattern)) {
      return 5;
  } else if (std::regex_match(operand, memory_register_symbol_pattern)) {
      return 6;
  } else if (std::regex_match(operand, memory_register_literal_pattern)) {
      return 7;
  } else {
      return -1;
  }
}
int Assembler::parseMemoryOperand(std::string symbol,int type){
  //type nam kaze koji tip adresiranja, pa mi vratimo zadnjih 12 bita ili koliko vec
  std::string temp = "";
  int ret;
  switch(type){
    case 4:
    case 5:
      for (char c:symbol){
        if (c!='[' && c!=']'){
          temp+=c;
        }
      }

      return parseRegister(temp);
      break;

    case 7:
      char* ptr = &symbol[0];
      while(*ptr != '%')ptr++;
      while(*ptr!='+'){
        if (*ptr!=' ')temp+=*ptr;
        ptr++;
      }
      
      ret = parseRegister(temp);
      temp = "";
      ret = ret<<16;

      while (*ptr!='\0'){
        if (*ptr!=' ' && *ptr!=']' && *ptr!='+'){
          temp+=*ptr;
        }
        ptr++;
      }
      if (temp[1] == 'x'){
        ret+=stoul(temp,nullptr,16);
      }
      else{
        ret+=stoi(temp);
      }
      
      return ret;
      break;
  } 
  return 0;
}

void Assembler::pushInstruction(unsigned char first,unsigned char second,unsigned char third,unsigned char fourth){
  section_contents[current_section].push_back(first);
  section_contents[current_section].push_back(second);
  section_contents[current_section].push_back(third);
  section_contents[current_section].push_back(fourth);
}

void Assembler::writeToOutput(){
  /*
  Znaci ide velicina symboltable, pa ime sekcije,velicina sadrzaja, sadrzaj sekcije,velicina tabele, relok tabela 
  */
  std::ofstream outfile("./"+output_name, std::ios::binary);
    if (!outfile) {
        std::cerr << "Error opening file for writing: " << output_name << std::endl;
        return;
    }

    // ubacujemo tabelu simbola u fajl, prvo velicina 
    size_t size = symbol_table.size();
    outfile.write(reinterpret_cast<const char*>(&size), sizeof(size));

    // Write each entry
    for (const auto& pair : symbol_table) {
        const std::string& name = pair.first;
        const SymbolTableEntry& entry = pair.second;
        size_t name_length = name.size();
        outfile.write(reinterpret_cast<const char*>(&name_length), sizeof(name_length));
        outfile.write(name.c_str(), name_length);
        entry.serialize(outfile);
    }

    //sada reloc tabele
    size = relocation_table.size();
    outfile.write(reinterpret_cast<const char*>(&size), sizeof(size));
    for (auto& entry:relocation_table){
      size = entry.first.size();
      outfile.write(reinterpret_cast<const char*>(&size), sizeof(size));
      outfile.write(entry.first.c_str(),size);
      //ubacili smo ime sekcije
      size = entry.second.size();
      outfile.write(reinterpret_cast<const char*>(&size), sizeof(size));
      
      //ubacili smo ime sekcije
      for (auto& sym: entry.second){
        size = sym.symbol.size();
        outfile.write(reinterpret_cast<const char*>(&size), sizeof(size));
        outfile.write(sym.symbol.c_str(),size);

        outfile.write(reinterpret_cast<const char*>(&sym.addend), sizeof(sym.addend));
        outfile.write(reinterpret_cast<const char*>(&sym.offset), sizeof(sym.offset));
      }
    }

    size = section_contents.size();
    outfile.write(reinterpret_cast<const char*>(&size), sizeof(size));
    for (auto& entry:section_contents){
      //ime sekcije
      size = entry.first.size();
      outfile.write(reinterpret_cast<const char*>(&size), sizeof(size));
      outfile.write(entry.first.c_str(),size);

      size = entry.second.size();
      outfile.write(reinterpret_cast<const char*>(&size), sizeof(size));
      for (auto& byte: entry.second){
        outfile.write(reinterpret_cast<const char*>(&byte), sizeof(byte));
      }
    }
    outfile.close();

}
void Assembler::push32bit(int value){
  char byte1,byte2,byte3,byte4;
  byte1 = (value >> 24) & 0xFF;
  byte2 = (value >> 16) & 0xFF;
  byte3 = (value >> 8) & 0xFF;
  byte4 = value & 0xFF;
  pushInstruction(byte1,byte2,byte3,byte4);
}

void Assembler::handleLabel(std::string label){
  if (symbol_table.count(label)){//ima ulaz u tabeli simbola, ubacujemo definiciju
    symbol_table[label].defined = true;
    symbol_table[label].section = current_section;
    symbol_table[label].value = location_counter;

    ForwardRefsEntry* temp = symbol_table[label].flink;
    while(temp){
      //section_contents[temp->section][temp->patch-1] = symbol_table[label].value>>8;
      //section_contents[temp->section][temp->patch] = symbol_table[label].value&255;
      relocation_table[current_section].push_back(RelocationTableEntry(
        temp->patch,symbol_table[label].section,symbol_table[label].value
      ));
      temp = temp->next; 
    }
    symbol_table[label].flink = nullptr;
  }
  else{
    //prvi put vidimo
    symbol_table[label] = SymbolTableEntry(
      location_counter,current_section
    );
  }
}

void Assembler::handleDirective(std::string directive,std::vector<std::string> operands){
  if (directive == ".global"){
    //obrada global, operandi su 
    for (std::string& operand:operands){
      this->symbol_table[operand] = SymbolTableEntry(0,"tbd",0,NOTYPE,true);
    }
  }
  if (directive == ".extern"){
    //obrada extern, oprandi su SIMBOLI
    for (std::string& operand:operands){
      this->symbol_table[operand] = SymbolTableEntry(0,"und",0,NOTYPE,true);
      this->symbol_table[operand].is_extern = true;
    }
  }
  if (directive == ".section"){
    //obrada section, operand je IME SEKCIJE
    if (current_section !="und"){
      //ako je und onda samo pravimo novu, und nema velicinu 
      symbol_table[current_section].size = location_counter - symbol_table[current_section].value;
    }
    current_section = operands.front();//ima 1 element ovde tkd ga vadimo kako god
    location_counter = 0;

    symbol_table[current_section] = SymbolTableEntry(0,current_section,0,SECTION,false);
    section_contents[current_section] = {};
  }

  if (directive == ".word"){
    //obrada word, operandi su lista simbola i/ili literala
    for (std::string operand:operands){
      debug_instructions[current_section].push_back(".word");
      //da li je simbol ili literal?
      if (Line::isLiteral(operand)){
        int value = stoi(operand);
        push32bit(value);
      }
      else{
        //ipak je simbol, treba nam vrednost simbola,
        //ako je simbol definisan, pravimo ulaz u relokacionoj tabeli
        //ako nije definisan, pravimo ulaz u tabeli simbola sa defined = false;
        
        if (symbol_table.count(operand)){// da li postoji bar 1 ulaz za ovaj simbol?
          if (symbol_table[operand].defined){
            int offset = location_counter;
            std::string symbol;
            int addend;

            if (symbol_table[operand].is_global){
              symbol = operand;
              addend = 0; //ne treba nam ovo za globalni
            }
            else{
              symbol = symbol_table[operand].section;
              addend = symbol_table[operand].value;
            }
            
            relocation_table[symbol_table[operand].section].push_back(
              RelocationTableEntry(offset,symbol,addend)
            );
          }
          else{
            ForwardRefsEntry* temp = symbol_table[operand].flink;
            if (!temp){
              symbol_table[operand].flink = new ForwardRefsEntry();
              temp->patch = location_counter;
              temp->section = current_section;
              temp->next = nullptr;
            }
            else{
              while(temp->next)temp=temp->next;
              temp->next = new ForwardRefsEntry();
              temp->patch = location_counter;
              temp->section = current_section;
              temp->next = nullptr;
            }
          }
        }
        else{
          symbol_table[operand] = SymbolTableEntry(
            0,"und",0,NOTYPE,false,false
          );
          symbol_table[operand].flink = new ForwardRefsEntry();
          symbol_table[operand].flink->section = current_section;
          symbol_table[operand].flink->patch = location_counter;
          symbol_table[operand].flink->next = nullptr;
        }
        pushInstruction(0,0,0,0);
      }
      location_counter+=4;
    }
  }
  if (directive == ".skip"){
    //obrada skip, operand je literal koji predstavlja broj bajtova koji se 
    int literal_value = stoi(operands.front());
    for (int i=0;i<literal_value;i++){
      section_contents[current_section].push_back(0);
    }
    location_counter += literal_value;
  }
  if (directive == ".end"){
    //obrada end, ne treba da ima operand
    symbol_table[current_section].size = location_counter - symbol_table[current_section].value;
    this->end = true;
    return;
  }
}

void Assembler::pushOperand(int reg_code,std::string operand,std::string instruction){
  //guramo 32bit operand u output
  if (Line::isLiteral(operand)){
      //literal je
    unsigned int literal;
    unsigned char byte1,byte2,byte3,byte4;
    if (Line::isHex(operand)){
      literal = stoul(operand,nullptr,16);
    }
    else{
      literal = stoi(operand);
    }
    
    byte1 = (literal >> 24) & 0xFF;
    byte2 = (literal >> 16) & 0xFF;
    byte3 = (literal >> 8) & 0xFF;
    byte4 = literal & 0xFF;
    
    pushInstruction(byte1,byte2,byte3,byte4);
    debug_instructions[current_section].push_back("operand");
    location_counter +=4;
  }
  else{
    pushInstruction(0,0,0,0);
    debug_instructions[current_section].push_back("operand");
    location_counter +=4;
    //prvo proveravamo da li je simbol definisan
    if (symbol_table.count(operand) && symbol_table[operand].defined){
      //ako je global addend = 0 i value je vrednost simbola,
      //ako nije global value je sekcija i addend je 
      int offset = location_counter;//4. bajt instrukcije
      
      std::string sym;
      int addend;

      if (symbol_table[operand].is_global){
        sym = operand;
        addend = 0;
      }
      else{
        sym = symbol_table[operand].section;
        addend = symbol_table[operand].value;
      }
    
      relocation_table[current_section].push_back(RelocationTableEntry(
        offset,sym,addend
      ));

    }
    else{
      //simbol ili nije u tabeli ili je defined = false;
      if (symbol_table.count(operand)){
        //dodajemo forwardlink
        ForwardRefsEntry* temp = symbol_table[operand].flink;
        if (!temp){
          symbol_table[operand].flink = new ForwardRefsEntry();
          symbol_table[operand].flink->next = nullptr;
          symbol_table[operand].flink->section = current_section;
          symbol_table[operand].flink->patch = location_counter;
        }
        else{
          while(temp->next)temp=temp->next;
          temp->next = new ForwardRefsEntry();
          temp->next->next = nullptr;
          temp->next->section = current_section;
          temp->next->patch = location_counter;
        }
      }
      else{
        symbol_table[operand] = SymbolTableEntry(
          0,current_section,0,0,false,true
        );
        symbol_table[operand].flink = new ForwardRefsEntry();
        symbol_table[operand].flink->next = nullptr;
        symbol_table[operand].flink->section = current_section;
        symbol_table[operand].flink->patch = location_counter;
      }
    }
  }
  
}

std::string Assembler::removeDollar(std::string operand){
  std::string ret = "";
  for (char c:operand){
    if (c!='$') ret+=c;
  }
  return ret;
}