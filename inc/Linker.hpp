#ifndef _LINKER_HPP
#define _LINKER_HPP

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <iomanip>
#include <unordered_map>
#include <bitset>
#include <bits/stdc++.h>


class Linker{
private:
  struct RelocationTableEntry{
    int offset; //lokacija gde se izvrsava prepravka, offset u odnosu na pocetak datoteke
    std::string symbol; //indeks simbola u tabeli simbola prema kojem vrsimo relokaciju
    //int rela_type; //tip relokacije, mozda nam ne treba
    int addend; //nepostredna vrednost koja se dodaje prilikom preprvke/realokacije
    RelocationTableEntry(){}
    RelocationTableEntry(int offset,std::string symbol,int addend):offset(offset),symbol(symbol),addend(addend){}
  };
#pragma pack(push,1)


struct SymbolTableEntry{
  bool is_global; // simboli su valjda samo global i local tkd moze bool
    int type; //da li je simbol sekcija, objekat, etc
    int value;//vrednost simbola, pomeraj od pocetka sekcije do definisanog simbola
    
    int size;//velicina samog simbola, po defaultu 0
    std::string section;//ulaz u tabeli zaglavlja sekcija (section header table)

    bool defined;
    bool is_extern = false;
    void* flink = nullptr;
    
    void deserialize(std::ifstream& is) {
        is.read(reinterpret_cast<char*>(&value), sizeof(value));
        is.read(reinterpret_cast<char*>(&size), sizeof(size));
        is.read(reinterpret_cast<char*>(&type), sizeof(type));
        is.read(reinterpret_cast<char*>(&is_global), sizeof(is_global));
        /*
        za stringove moramo da imamo length pa same simbole 
        size_t name_length;
      infile.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));
      std::cout<<name_length<<std::endl;
      // Read the name string
      std::string name(name_length, '\0');
      infile.read(&name[0], name_length);
      std::cout<<name<<std::endl;
        */

        size_t section_string_len;
        is.read(reinterpret_cast<char*>(&section_string_len), sizeof(section_string_len));
        section = std::string(section_string_len,'\0');
        is.read(&section[0], section_string_len);

        is.read(reinterpret_cast<char*>(&defined), sizeof(defined));
        is.read(reinterpret_cast<char*>(&is_extern), sizeof(is_extern));

        flink = nullptr; // Reset the flink pointer
    }
};
#pragma pack(pop)


std::unordered_map<std::string, SymbolTableEntry> global_symbol_table;
std::unordered_map<std::string,std::unordered_map<std::string,SymbolTableEntry>> local_symbol_table;
std::unordered_map<std::string,std::vector<char>> section_contents;
std::unordered_map<std::string,std::vector<RelocationTableEntry>> relocation_table;

std::unordered_map<std::string,unsigned int> section_addr; //kopnacne adrese svih sekcija
int max_adr = 0;
int location_counter = 0;
public:
  void printTables();
  bool loadFile(std::string filename);
  bool checkSolved();

  int begin(std::vector<std::string> input_files,std::unordered_map<std::string,unsigned int> place_addr,int hex, std::string output_file,bool print);
  void printFinalAddr();

  void generateOutput(std::string output_name);
  bool checkSections(); // da li se preklapaju sekcije
  
};
#endif