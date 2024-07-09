#include "../inc/Assembler.hpp"
#include <bitset>
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

      ForwardRefsEntry* temp = symbol_table[label].flink;
      while(temp){
        section_contents[temp->section][temp->patch] = symbol_table[label].value;
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
      section_contents[current_section].push_back(16);
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(0);
      

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
      section_contents[current_section].push_back(147);//najvisi bajt
      section_contents[current_section].push_back(254);
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(4);//najnizi bajt
      
      
      section_contents[current_section].push_back(151);
      section_contents[current_section].push_back(14);
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(4);
      
    }
    else if (instruction == "call"){
      //jedan operand, adresa funkcije koja se poziva
      //push pc, pc<= operand
      //ovo moze preko 1 instrukcije, samo treba da se resi vrednost operanda

      //operand je ili simbol ili literal, idu u D either way
      //00100000 00000000 0000gornja4 donjih8D
      std::string operand = operands.front();
      //ovaj deo ispod mozemo da stavimo u funkciju parseJumpOperands()
      section_contents[current_section].push_back(32);
      section_contents[current_section].push_back(0);
      parseJumpOperands(operand,line);
      
      

    }
    else if (instruction == "ret"){
      //nema operanada, radi se pop pc
      //pc<=mem[sp];sp<=sp+4
      section_contents[current_section].push_back(147);//najvisi bajt
      section_contents[current_section].push_back(254);
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(4);//najnizi bajt
      
      
      
    }
    else if (instruction == "jmp"){
      //pc<=operand, 1 operand
      std::string operand = operands.front();
      section_contents[current_section].push_back(48);//najvisi bajt
      section_contents[current_section].push_back(0);
      parseJumpOperands(operand,line);
      
      
    }
    else if (instruction == "beq"){
      //2 registra, operand koji predstavlja adresu za skok
      //ako r1==r2, pc<=operand
      std::string reg1 = operands[0];
      std::string reg2 = operands[1];
      std::string operand = operands[2];

      int reg_code1 = parseRegister(reg1);
      int reg_code2 = parseRegister(reg2);
      reg_code1 = reg_code1<<4;
      reg_code1+=reg_code2;

      section_contents[current_section].push_back(49);
      section_contents[current_section].push_back(reg_code1);
      parseJumpOperands(operand,line);
      
      
      

    }
    else if (instruction == "bne"){
      std::string reg1 = operands[0];
      std::string reg2 = operands[1];
      std::string operand = operands[2];

      int reg_code1 = parseRegister(reg1);
      int reg_code2 = parseRegister(reg2);
      reg_code1 = reg_code1<<4;
      reg_code1+=reg_code2;
      section_contents[current_section].push_back(50);
      section_contents[current_section].push_back(reg_code1);
      parseJumpOperands(operand,line);
      
      
      
    }
    else if (instruction == "bgt"){
      std::string reg1 = operands[0];
      std::string reg2 = operands[1];
      std::string operand = operands[2];

      int reg_code1 = parseRegister(reg1);
      int reg_code2 = parseRegister(reg2);
      reg_code1 = reg_code1<<4;
      reg_code1+=reg_code2;
      section_contents[current_section].push_back(51);
      section_contents[current_section].push_back(reg_code1);
      parseJumpOperands(operand,line);
      
      
      
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
      section_contents[current_section].push_back(129);//najvisi bajt
      section_contents[current_section].push_back(sp);
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(-4);//najnizi bajt
      
      
      
    }
    else if (instruction == "pop"){
      //jedan operand-registar
      //reg = mem[sp], sp+=4
      std::string reg = operands.front();
      int reg_code = parseRegister(reg);
      int sp = 14;
      sp = sp<<4;
      sp+=reg_code;
      section_contents[current_section].push_back(147);//najvisi bajt
      section_contents[current_section].push_back(sp);
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(4);//najnizi bajt
      
      
      
    }
    else if (instruction == "xchg"){
      //2 operanada registri oba,
      //temp = r1, r1 = r2, r2=temp -> zamena vrednosti 2 registra
      //0100 0000 0000 BBBB CCCC 0000 0000

      


      int r1_code = parseRegister(operands[0]);
      int r2_code = parseRegister(operands[1]);
      r2_code<<4;
      section_contents[current_section].push_back(64);//najvisi bajt
      section_contents[current_section].push_back(r1_code);
      section_contents[current_section].push_back(r2_code);
      section_contents[current_section].push_back(0);//najnizi bajt
      
      
      
    }
    else if (instruction == "add"){
      //2 registra
      //r1 = r1+r2
      //0101 0000 AAAA BBBB CCCC 0000 0000 0000
      //A->r2, B->r1, C->r2
      int r1_code = parseRegister(operands[0]);
      int r2_code = parseRegister(operands[1]);
      int c_code = r2_code<<4;
      r2_code<<4;
      r2_code+=r1_code;
      section_contents[current_section].push_back(80);//najvisi bajt
      section_contents[current_section].push_back(r2_code);
      section_contents[current_section].push_back(c_code);
      section_contents[current_section].push_back(0);//najnizi bajt
      
      
      
    }
    else if (instruction == "sub"){
      //2 registra
      //r1 = r1-r2
      int r1_code = parseRegister(operands[0]);
      int r2_code = parseRegister(operands[1]);
      int c_code = r2_code<<4;
      r2_code<<4;
      r2_code+=r1_code;
      section_contents[current_section].push_back(81);//najvisi bajt
      section_contents[current_section].push_back(r2_code);
      section_contents[current_section].push_back(c_code);
      section_contents[current_section].push_back(0);//najnizi bajt
      
      
      
    }
    else if (instruction == "mul"){
      //2 reg, r1 = r1*r2
      int r1_code = parseRegister(operands[0]);
      int r2_code = parseRegister(operands[1]);
      int c_code = r2_code<<4;
      r2_code<<4;
      r2_code+=r1_code;
      section_contents[current_section].push_back(82);//najvisi bajt
      section_contents[current_section].push_back(r2_code);
      section_contents[current_section].push_back(c_code);
      section_contents[current_section].push_back(0);//najnizi bajt
      
      
      
    }
    else if (instruction == "div"){
      //2 reg, r1 = r1/r2
      int r1_code = parseRegister(operands[0]);
      int r2_code = parseRegister(operands[1]);
      int c_code = r2_code<<4;
      r2_code<<4;
      r2_code+=r1_code;
      section_contents[current_section].push_back(83);//najvisi bajt
      section_contents[current_section].push_back(r2_code);
      section_contents[current_section].push_back(c_code);
      section_contents[current_section].push_back(0);//najnizi bajt
      
      
      
    }
    else if (instruction == "not"){
      //1 reg, r1 = !r1
      // 0110 0000 AAAA BBBB CCCC 0000 0000 0000
      // ovde je c 0, znaci zadnja 2 su 0
      int r1_code = parseRegister(operands[0]);
      int r2_code = parseRegister(operands[1]);
      r2_code<<4;
      r2_code+=r1_code;
      section_contents[current_section].push_back(96);//najvisi bajt
      section_contents[current_section].push_back(r2_code);
      section_contents[current_section].push_back(0);//najnizi bajt
      section_contents[current_section].push_back(0);
      
      
      
    }
    else if (instruction == "and"){
      //2 reg, r1= r1&r2
      //ovde c == A
      int r1_code = parseRegister(operands[0]);
      int r2_code = parseRegister(operands[1]);
      int c_code = r2_code<<4;
      r2_code<<4;
      r2_code+=r1_code;

      
      section_contents[current_section].push_back(97);//najvisi bajt
      section_contents[current_section].push_back(r2_code);
      section_contents[current_section].push_back(c_code);
      section_contents[current_section].push_back(0);//najnizi bajt
      
      
      

    }
    else if (instruction == "or"){
      //2 reg , r1 = r1|r2
      int r1_code = parseRegister(operands[0]);
      int r2_code = parseRegister(operands[1]);
      int c_code = r2_code<<4;
      r2_code<<4;
      r2_code+=r1_code;

      
      section_contents[current_section].push_back(98);//najvisi bajt
      section_contents[current_section].push_back(r2_code);
      section_contents[current_section].push_back(c_code);
      section_contents[current_section].push_back(0);//najnizi bajt
      
      
      
    }
    else if (instruction == "xor"){
      //2 reg, r1 = r1^r2
      int r1_code = parseRegister(operands[0]);
      int r2_code = parseRegister(operands[1]);
      int c_code = r2_code<<4;
      r2_code<<4;
      r2_code+=r1_code;

      section_contents[current_section].push_back(99);//najvisi bajt
      section_contents[current_section].push_back(r2_code);
      section_contents[current_section].push_back(c_code);
      section_contents[current_section].push_back(0);//najnizi bajt
      
      
      
    }
    else if (instruction == "shl"){
      //2 registra
      //r2 = r2<r1
      //0111 0000 AAAA BBBB CCCC 0000 0000 0000
      //A->r2, B->r1, c->r2
      int r1_code = parseRegister(operands[0]);
      int r2_code = parseRegister(operands[1]);
      int c_code = r2_code<<4;
      r2_code<<4;
      r2_code+=r1_code;

      section_contents[current_section].push_back(112);//najvisi bajt
      section_contents[current_section].push_back(r2_code);
      section_contents[current_section].push_back(c_code);
      section_contents[current_section].push_back(0);//najnizi bajt
      
      
      
    }
    else if (instruction == "shr"){
      //2 registra
      int r1_code = parseRegister(operands[0]);
      int r2_code = parseRegister(operands[1]);
      int c_code = r2_code<<4;
      r2_code<<4;
      r2_code+=r1_code;

      
      section_contents[current_section].push_back(113);//najvisibajt
      section_contents[current_section].push_back(r2_code);
      section_contents[current_section].push_back(c_code);
      section_contents[current_section].push_back(0);//najnizi bajt
      
      
      
    }
    /*
    Ovde treba da se napravi prvo funkcija za parsiranje operanada za rad
    sa memorijom
    */
    else if (instruction == "ld"){
      //ld operand, %gprD
      
     int mem_type = getMemType(operands[0]);
     switch(mem_type){
      case 0:

        break;
      case 1:

        break;
      case 2:

        break;
      case 3:

        break;
      case 4:

        break;
      default:
        std::cout<<"ERROR - BAD ADDRESSING"<<std::endl;
        return;
      
     }
  
     //nakon adresiranja gledamo da li je ld ili st
     if (instruction == "ld"){

     }
     else{
      //instruction == "st"
     }

    }
    else if (instruction == "csrrd"){
      //2 reg
      //1001 0000
      int r1_code = parseRegister(operands[0]);
      int r2_code = parseRegister(operands[1]);
      r2_code<<4;
      r2_code+=r1_code;

      section_contents[current_section].push_back(144);//najvisibajt
      section_contents[current_section].push_back(r2_code);
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(0);//najnizi bajt
    }
    else if (instruction == "csrwr"){
      //2 reg
      //1001 0100 AAAA BBBB
      int r1_code = parseRegister(operands[0]);
      int r2_code = parseRegister(operands[1]);
      r2_code<<4;
      r2_code+=r1_code;

      section_contents[current_section].push_back(148);//najvisibajt
      section_contents[current_section].push_back(r2_code);
      section_contents[current_section].push_back(0);
      section_contents[current_section].push_back(0);//najnizi bajt

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
  std::cout<<"Symbol table: "<<std::endl;
  for (auto& entry:symbol_table){
    std::cout<<"Name: "<<entry.first<<" - "<<
    "Value: "<<entry.second.value<<" - "<<
    "Size: "<<entry.second.size<<" - "<<
    "Type: "<<entry.second.type<<" - "<<
    "Global: "<<entry.second.is_global<<" - "<<
    "Section: "<<entry.second.section<<" - "<<std::endl;
  }
  std::cout<<"section contents"<<std::endl;
  /*
  sve instrukcije po 4B
  */
  int count = 0;
  for (auto& section:section_contents){
    std::cout<<section.first<<std::endl;
    for (auto& b:section.second){
      if (count!=0 && count%32 == 0)std::cout<<std::endl;
      std::cout<<std::bitset<8>(b)<<" ";
      count+=8;
    }
    std::cout<<std::endl;
  }
}

void Assembler::parseJumpOperands(std::string operand,Line line){


  if (line.isLiteral(operand)){
      //literal je
    int literal = stoi(operand);
    int bits_11_to_8 = (literal >> 8) & 0xF;
    int lowest_8 = literal & 0xFF;

    section_contents[current_section].push_back(bits_11_to_8);
    section_contents[current_section].push_back(lowest_8);
    
    
  }
  else{
    //prvo proveravamo da li je simbol definisan
    section_contents[current_section].push_back(0);
    section_contents[current_section].push_back(0);
    if (symbol_table.count(operand) && symbol_table[operand].defined){
      //ako je global addend = 0 i value je vrednost simbola,
      //ako nije global value je sekcija i addend je 
      int offset = location_counter-1;//4. bajt instrukcije
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
          symbol_table[operand].flink->patch = location_counter-1;
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
        symbol_table[operand].flink->patch = location_counter-1;
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
  return -1;
}

int Assembler::getMemType(std::string operand){
  /*
  0 - memdir
  1 - memind
  2 - regdir
  3 - regind 
  4 - regindpom 
  
  memdir -> $literal, $symbol
  memind -> literal / symbol
  regdir -> %reg
  regind -> [%reg]
  regindpom -> [%reg + simbol/literal]

  * svuda mogu da budu ili simbol ili literal, ako je simbol mora biti definisan i manji od 2^12 

  */
  

  
  return -1;
}
bool Assembler::validSymbol(std::string symbol){
  //da li je simbol definisan tokom asembliranja (nije extern) i manji od 2^12
  //ovde samo proveravamo da li je extern jer ako nije i nije def asembler svjd baca gresku 
  for (auto& s:symbol_table){
    if (s.first == symbol && s.second.is_extern){
      return false;
    }
  }
  return true; // ako nije jos definisan onda skoro sigurno nije extern pa ce potencijalnu gresku kasnije pokupiti
}