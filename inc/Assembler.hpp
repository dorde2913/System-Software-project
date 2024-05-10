#ifndef _ASSEMBLER_HPP
#define _ASSEMBLER_HPP


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
  typedef struct SymbolTableEntry{
    /************************************************
    po 1 entry za svaki simbol u fajlu
    umesto da imamo ovde name i da moramo da pravimo ceo pool stringova
    mozemo da name koristimo za mapiranje u tabelu,
     tabela ce biti map<string,symtableentry>
    ************************************************/
    int entry_size; //velicina samog entry-ja, mozda bude korisno, mozda ne
   
    bool is_global; // simboli su valjda samo global i local tkd moze bool
    int type; //da li je simbol sekcija, objekat, etc
    int value;//vrednost simbola, pomeraj od pocetka sekcije do definisanog simbola
    
    int size;//velicina samog simbola, po defaultu 0
    std::string section = "UNDEF";//ulaz u tabeli zaglavlja sekcija (section header table)
    
  };
  typedef struct RelocationTableEntry{

  };
  typedef struct ForwardRefsEntry{

  };
  typedef struct SectionTableEntry{
    //ovo mozda ne bude ni potrebno isk
  };
/************************************************
Atributi asemblera
************************************************/
  Parser parser; //parser koji ce nama da vraca sredjene linije za obradu
  std::string file_name; //ime fajla koji se asemblira
  std::string output_name; //ime objektnog fajla koji asembler pravi


};

#endif