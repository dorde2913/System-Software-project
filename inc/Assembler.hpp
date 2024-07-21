#ifndef _ASSEMBLER_HPP
#define _ASSEMBLER_HPP

#include <unordered_map>
#include <vector>
#include <utility>
#include <regex>
#include <iomanip>
#include <bitset>
#include <iostream>
#include <fstream>

#include "Parser.hpp"

/*********************
 * Definicije tipova 
**********************/
#define NOTYPE 0
#define OBJECT 1
#define FUNC 2
#define SECTION 3
#define COMMON 4



/***********************************************************************
Treba prvo da napravimo parser koji ce da ide liniju po liniju
Assembler ima tabelu simbola, tabelu relok zapisa i forwardrefs tabelu
i section header table
prve 2 treba da idu u zasebne sekcije na kraju fajla

***********************************************************************/

class Assembler{
  private:
/************************************************
Strukture koje predstavljaju ulaze u razlicite tabele,
same tabele predstavicemo mapama ili vektorima 
sta znam
************************************************/

  struct ForwardRefsEntry{
    int patch; //adresa koja treba da se sredi;
    std::string section;
    ForwardRefsEntry* next;//sledeci entry
  };
  #pragma pack(push,1)
  struct SymbolTableEntry{
    /************************************************
    po 1 entry za svaki simbol u fajlu
    umesto da imamo ovde name i da moramo da pravimo ceo pool stringova
    mozemo da name koristimo za mapiranje u tabelu,
     tabela ce biti map<string,symtableentry>

     pri upisivanju svega u fajl koristimo .size() tkd ne moramo to da pamtimo
    ************************************************/
    
   
    bool is_global; // simboli su valjda samo global i local tkd moze bool
    int type; //da li je simbol sekcija, objekat, etc
    int value;//vrednost simbola, pomeraj od pocetka sekcije do definisanog simbola
    
    int size;//velicina samog simbola, po defaultu 0
    std::string section = "UNDEF";//ulaz u tabeli zaglavlja sekcija (section header table)

    bool defined;
    bool is_extern = false;
    ForwardRefsEntry* flink = nullptr;
    SymbolTableEntry(){}
    SymbolTableEntry(int value,std::string section,int size = 0, int type = NOTYPE, bool is_global = false, bool defined = true )
                : value(value),section(section),size(size), type(type), is_global(is_global),defined(defined){}

    void serialize(std::ofstream& os) const {
        /*
        size_t name_length = name.size();
        outfile.write(reinterpret_cast<const char*>(&name_length), sizeof(name_length));

        // Write the name string
        outfile.write(name.c_str(), name_length);
        */
        os.write(reinterpret_cast<const char*>(&value), sizeof(value));
        os.write(reinterpret_cast<const char*>(&size), sizeof(size));
        os.write(reinterpret_cast<const char*>(&type), sizeof(type));
        os.write(reinterpret_cast<const char*>(&is_global), sizeof(is_global));
        
        size_t section_string_length = section.size();
        os.write(reinterpret_cast<const char*>(&section_string_length), sizeof(section_string_length));
        os.write(section.c_str(), section_string_length);
        
        os.write(reinterpret_cast<const char*>(&defined), sizeof(defined));
        os.write(reinterpret_cast<const char*>(&is_extern), sizeof(is_extern));
    }

    // Deserialize function
    void deserialize(std::istream& is) {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
        is.read(reinterpret_cast<char*>(&size), sizeof(size));
        is.read(reinterpret_cast<char*>(&type), sizeof(type));
        is.read(reinterpret_cast<char*>(&is_global), sizeof(is_global));
        is.read(reinterpret_cast<char*>(&section), sizeof(section));
        is.read(reinterpret_cast<char*>(&defined), sizeof(defined));
        is.read(reinterpret_cast<char*>(&is_extern), sizeof(is_extern));
        flink = nullptr; // Reset the flink pointer
    }
  };
  #pragma pack(pop)
  struct RelocationTableEntry{
    int offset; //lokacija gde se izvrsava prepravka, offset u odnosu na pocetak datoteke
    std::string symbol; //indeks simbola u tabeli simbola prema kojem vrsimo relokaciju
    //int rela_type; //tip relokacije, mozda nam ne treba
    int addend; //nepostredna vrednost koja se dodaje prilikom preprvke/realokacije
    RelocationTableEntry(){}
    RelocationTableEntry(int offset,std::string symbol,int addend):offset(offset),symbol(symbol),addend(addend){}
  };
  
  // da li je nama potrebna tabela sekcija? potencijalno i nije
  struct SectionTableEntry{
    //ovo mozda ne bude ni potrebno 
    //i ovo cemo da mapiramo preko stringa, tako da nema polje za ime
    int sect_type; //tip sekcije
    int addr; //adresa pocetka sekcije, ovo je 0 ako sekcija ne treba da bude u memoriji tokom izvrsavanja
    int offset; //pomeraj od pocetka datoteke do posmatrane sekcije

  };
/************************************************
Atributi asemblera
************************************************/
  std::string file_name; //ime fajla koji se asemblira
  std::string output_name; //ime objektnog fajla koji asembler pravi
  //tabela simbola, mapirano po imenu simbola
  std::unordered_map<std::string,SymbolTableEntry> symbol_table;

  //relokacionih zapisa moze biti vise za jednu sekciju, mozemo svaka sekcija -> vektor zapisa
  std::unordered_map<std::string,std::vector<RelocationTableEntry>> relocation_table;
  std::unordered_map<std::string,std::vector<std::string>> debug_instructions;
  //forwardrefs je u tabeli simbola tkd nema svoj atribut ovde
  int location_counter = 0;
  std::string current_section = "und";

  //sadrzaj svake sekcije, upisujemo bajt po bajt pa smo stavili char
  //parovi su section_name - sadrzaj
  std::unordered_map<std::string,std::vector<char>> section_contents; 

  bool error = false;
  bool end = false; // end direktiva ovo postavi i asembliranje se zavrsava
  public:
  /**************************
   * Metode asemblera
  ***************************/
  void startAssembly(std::string input_file, std::string output_file,bool print);
  void addLine(Line& line);
  bool solveSymbols();
  
  void printTables();
  
  int parseRegister(std::string reg_name);
  int parseMemoryOperand(std::string symbol,int type);
  int getMemType(std::string operand);
  int handleSymbol(std::string sym);
  void pushInstruction(char first,char second, char third, char fourth);
  void writeToOutput();
  void push32bit(int value);
  void handleLabel(std::string label);
  void handleDirective(std::string directive, std::vector<std::string> operands);
  void loadOperandToRegister(int reg_code,std::string operand,std::string instruction);
  void handleLoadStore(std::string instruction,int reg_code,std::string operand,int mem_type);

  std::string removeDollar(std::string operand);
};

#endif