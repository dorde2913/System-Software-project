#include "../inc/Assembler.hpp"

void Assembler::startAssembly(std::string input_file,std::string output_file){
  this->file_name = input_file;
  this->output_name = output_file;

  Parser& parser = Parser::getInstance();

  parser.setFile(input_file);

  while (!Parser::end){
    //std::cout<<"u while"<<std::endl;
    Line line = parser.parseLine();
    this->addLine(line);
    if (this->end) break;
  }
  
  bool solved = this->solveSymbols();
  if (!solved){
    std::cout<<"ERROR"<<std::endl;
  }
  printTables();
}
void Assembler::addLine(Line& line){
  line.cleanComments();
  if (line.empty()) return; //linija je imala samo komentar, ignorisemo je



  if (line.hasLabel()){
    //obrada labeli
    //imamo labelu -> to je definicija simbola
    std::string label = line.getLabel();
    if (symbol_table.count(label)){//ima ulaz u tabeli simbola, ubacujemo definiciju
      symbol_table[label].defined = true;
      symbol_table[label].section = current_section;
      symbol_table[label].value = location_counter;
    }
    else{
      //prvi put vidimo
      symbol_table[label] = SymbolTableEntry(
        location_counter,current_section
      );
    }
  }

  if (line.hasDirective()){
    std::string directive = line.getDirective().first;
    std::vector<std::string> operands = line.getDirective().second;
    
    
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
      //relocation_table[current_section] = ;
      
    }

    if (directive == ".word"){
      //obrada word, operandi su lista simbola i/ili literala
      for (std::string operand:operands){
        //da li je simbol ili literal?
        if (line.isLiteral(operand)){
          int value = stoi(operand);
          //podaci su 2 bajta, little endian znaci nizi bajt niza adresa
          int low = value & 255;
          int high = value >> 8;
          section_contents[current_section].push_back(low);
          section_contents[current_section].push_back(high);
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
          //velicina svega je 2B, pa pushujemo 2B u sekciju
          section_contents[current_section].push_back(0);
          section_contents[current_section].push_back(0);
        }
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

  if (line.hasInstruction()){
    
    std::string instruction = line.getInstruction().first;
    
    std::vector<std::string> operands = line.extractOperands(line.getInstruction().second);

    //sve instrukcije su velicine 4 bajta, te mozemo odmah da lepo dodamo
    location_counter +=4;
    //U INSTRUKCIJAMA JE SOURCE PA DEST ??
    if (instruction == "halt"){
      //zaustavljanje izvrsavanja, nema operande
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(0);
    }
    else if (instruction == "int"){
      //softverski prekid, nema op
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(16);

    }
    else if (instruction == "iret"){
      //pop pc, pop status , nema op
      //ovo mora kao 2 instrukcije izgleda
      //pc = mem[sp];sp+=4;status=mem[sp];sp+=
      //prva je 10010011 gpr[A]<=mem[gpr[B]]; gpr[B]<=gpr[B]+D; D=4, A je pc, B je sp
      //druga je 10010111 ista stvar ali je csrA, odnosno status registar
      //2. bajt je regA,regB, 4. bajt ce nam biti pomeraj

      //r14 JE SP, r15 JE PC
      location_counter+=4;
      //pop pc

      section_contents[current_section].push_back(4);//najnizi bajt
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(254);
      section_contents[current_section].push_back(147);//najvisi bajt

      section_contents[current_section].push_back(4);
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(14);
      section_contents[current_section].push_back(151);
    }
    else if (instruction == "call"){
      //jedan operand, adresa funkcije koja se poziva
      //push pc, pc<= operand
      //ovo moze preko 1 instrukcije, samo treba da se resi vrednost operanda

      //operand je ili simbol ili literal, idu u D either way
      //00100000 00000000 0000gornja4 donjih8D
      std::string operand = operands.front();
      //ovaj deo ispod mozemo da stavimo u funkciju parseJumpOperands()
      parseJumpOperands(operand,line);
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(32);

    }
    else if (instruction == "ret"){
      //nema operanada, radi se pop pc
      //pc<=mem[sp];sp<=sp+4
      section_contents[current_section].push_back(4);//najnizi bajt
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(254);
      section_contents[current_section].push_back(147);//najvisi bajt
    }
    else if (instruction == "jmp"){
      //pc<=operand, 1 operand
      std::string operand = operands.front();
      parseJumpOperands(operand,line);
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(48);//najvisi bajt
    }
    else if (instruction == "beq"){
      //2 registra, operand koji predstavlja adresu za skok
      //ako r1==r2, pc<=operand
      std::string reg1 = operands[0];
      std::string reg2 = operands[1];
      std::string operand = operands[2];

      int reg_code1 = parseRegister(reg1);
      int reg_code2 = parseRegister(reg2);

      parseJumpOperands(operand,line);
      reg_code1 = reg_code1<<4;
      reg_code1+=reg_code2;
      section_contents[current_section].push_back(reg_code1);
      section_contents[current_section].push_back(49);

    }
    else if (instruction == "bne"){
      std::string reg1 = operands[0];
      std::string reg2 = operands[1];
      std::string operand = operands[2];

      int reg_code1 = parseRegister(reg1);
      int reg_code2 = parseRegister(reg2);

      parseJumpOperands(operand,line);
      reg_code1 = reg_code1<<4;
      reg_code1+=reg_code2;
      section_contents[current_section].push_back(reg_code1);
      section_contents[current_section].push_back(50);
    }
    else if (instruction == "bgt"){
      std::string reg1 = operands[0];
      std::string reg2 = operands[1];
      std::string operand = operands[2];

      int reg_code1 = parseRegister(reg1);
      int reg_code2 = parseRegister(reg2);

      parseJumpOperands(operand,line);
      reg_code1 = reg_code1<<4;
      reg_code1+=reg_code2;
      section_contents[current_section].push_back(reg_code1);
      section_contents[current_section].push_back(51);
    }
    else if (instruction == "push"){
      //jedan operand koji je neki registar,
      //sp-=4, mem[sp] = reg
      std::string reg = operands.front();
      int reg_code = parseRegister(reg);
      //1000 0001, u D upisujemo -4, u A sp, u B registar nas
      int sp = 14;
      sp = sp<<4;
      sp+=reg_code;
      section_contents[current_section].push_back(-4);//najnizi bajt
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(sp);
      section_contents[current_section].push_back(129);//najvisi bajt
    }
    else if (instruction == "pop"){
      //jedan operand-registar
      //reg = mem[sp], sp+=4
      std::string reg = operands.front();
      int reg_code = parseRegister(reg);
      int sp = 14;
      sp = sp<<4;
      sp+=reg_code;
      section_contents[current_section].push_back(4);//najnizi bajt
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(sp);
      section_contents[current_section].push_back(147);//najvisi bajt
    }
    else if (instruction == "xchg"){
      //2 operanada registri oba,
      //temp = r1, r1 = r2, r2=temp -> zamena vrednosti 2 registra

    }
    else if (instruction == "add"){
      //2 registra
      //r1 = r1+r2
    }
    else if (instruction == "sub"){
      //2 registra
      //r1 = r1-r2
    }
    else if (instruction == "mul"){
      //2 reg, r1 = r1*r2
    }
    else if (instruction == "div"){
      //2 reg, r1 = r1/r2
    }
    else if (instruction == "not"){
      //1 reg, r1 = !r1
    }
    else if (instruction == "and"){
      //2 reg, r1= r1&r2
    }
    else if (instruction == "or"){
      //2 reg , r1 = r1|r2
    }
    else if (instruction == "xor"){
      //2 reg, r1 = r1^r2

    }
    else if (instruction == "shl"){
      //2 registra
    }
    else if (instruction == "shr"){
      //2 registra
    }
    else if (instruction == "ld"){
      //operand + registar
    }
    else if (instruction == "st"){
      //registar + operand
    }
    else if (instruction == "csrrd"){
      //2 reg
    }
    else if (instruction == "csrwr"){
      //2 reg
    }
    
  }


}
bool Assembler::solveSymbols(){
  return false;
}

void Assembler::printTables(){
  std::cout<<"Symbol table: "<<std::endl;
  for (auto& entry:symbol_table){
    std::cout<<"Name: "<<entry.first<<" - "<<
    "Value: "<<entry.second.value<<" - "<<
    "Size: "<<entry.second.size<<" - "<<
    "Type: "<<entry.second.type<<" - "<<
    "Global: "<<entry.second.is_global<<" - "<<
    "Section: "<<entry.second.section<<" - "<<std::endl;
  }
}

void Assembler::parseJumpOperands(std::string operand,Line line){


  if (line.isLiteral(operand)){
      //literal je
    int literal = stoi(operand);
    int bits_11_to_8 = (literal >> 8) & 0xF;
    int lowest_8 = literal & 0xFF;
    
    section_contents[current_section].push_back(lowest_8);
    section_contents[current_section].push_back(bits_11_to_8);
    
  }
  else{
    //prvo proveravamo da li je simbol definisan
    if (symbol_table.count(operand) && symbol_table[operand].defined){
      //ako je global addend = 0 i value je vrednost simbola,
      //ako nije global value je sekcija i addend je 
      int offset = location_counter-4;//4. bajt instrukcije
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
          symbol_table[operand].flink->patch = location_counter-4;
        }
        else{
          while(temp->next)temp=temp->next;
          temp->next = new ForwardRefsEntry();
          temp->next->next = nullptr;
          temp->next->section = current_section;
          temp->next->patch = location_counter-4;
        }
      }
      else{
        symbol_table[operand] = SymbolTableEntry(
          0,current_section,0,0,false,true
        );
        symbol_table[operand].flink = new ForwardRefsEntry();
        symbol_table[operand].flink->next = nullptr;
        symbol_table[operand].flink->section = current_section;
        symbol_table[operand].flink->patch = location_counter-4;
      }
    }
  }
}
int Assembler::parseRegister(std::string register_name){
  if (register_name == "%r0") return 0;
  if (register_name == "%r1") return 1;
  if (register_name == "%r2") return 2;
  if (register_name == "%r3") return 3;
  if (register_name == "%r4") return 4;
  if (register_name == "%r5") return 5;
  if (register_name == "%r6") return 6;
  if (register_name == "%r7") return 7;
  if (register_name == "%r8") return 8;
  if (register_name == "%r9") return 9;
  if (register_name == "%r10") return 10;
  if (register_name == "%r11") return 11;
  if (register_name == "%r12") return 12;
  if (register_name == "%r13") return 13;
  if (register_name == "%r14") return 14;
  if (register_name == "%r15") return 15;
  
}