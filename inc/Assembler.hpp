#ifndef _ASSEMBLER_HPP
#define _ASSEMBLER_HPP

#include <unordered_map>
#include <vector>
#include <utility>

#include "Parser.hpp"

/*********************
 * Definicije tipova 
**********************/
#define NOTYPE 0;
#define OBJECT 1;
#define FUNC 2;
#define SECTION 3;
#define COMMON 4;



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
    ForwardRefsEntry* next;//sledeci entry
  };
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
    ForwardRefsEntry* flink;
    
  };
  struct RelocationTableEntry{
    int offset; //lokacija gde se izvrsava prepravka, offset u odnosu na pocetak datoteke
    int symbol; //indeks simbola u tabeli simbola prema kojem vrsimo relokaciju
    int rela_type; //tip relokacije
    int addend; //nepostredna vrednost koja se dodaje prilikom prepravke/relokacije
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
  std::unordered_map<std::string,RelocationTableEntry> relocation_table;

  //forwardrefs je u tabeli simbola tkd nema svoj atribut ovde


  public:
  /**************************
   * Metode asemblera
  ***************************/
  void startAssembly(std::string input_file, std::string output_file);
  void addLine(Line& line);
  bool solveSymbols();
  

};

#endif