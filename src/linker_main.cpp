#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <iomanip>
#include <unordered_map>

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
        std::cout<<"symbol value: "<<value<<std::endl;

        is.read(reinterpret_cast<char*>(&size), sizeof(size));
        std::cout<<"symbol size: "<<size<<std::endl;

        is.read(reinterpret_cast<char*>(&type), sizeof(type));
        std::cout<<"symbol type: "<<type<<std::endl;

        is.read(reinterpret_cast<char*>(&is_global), sizeof(is_global));
        std::cout<<"is_global: "<<is_global<<std::endl;

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
        std::cout<<"symbol section: "<<section<<std::endl;

        is.read(reinterpret_cast<char*>(&defined), sizeof(defined));
        std::cout<<"symbol defined: "<<defined<<std::endl;

        is.read(reinterpret_cast<char*>(&is_extern), sizeof(is_extern));
        std::cout<<"is_extern "<<is_extern<<std::endl;

        flink = nullptr; // Reset the flink pointer
    }
};
#pragma pack(pop)

int main(int argc, char* argv[]){
  std::string filename = argv[1];

  std::unordered_map<std::string, SymbolTableEntry> symbolTable;
    std::ifstream infile("./"+filename, std::ios::binary);
    if (!infile) {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return 0;
    }
    std::cout<<"opened file"<<std::endl;
    // Read the number of entries
    size_t size;
    infile.read(reinterpret_cast<char*>(&size), sizeof(size));
    std::cout<<size<<std::endl;
    for (size_t i = 0; i < size; ++i) {
      std::cout<<"in for loop"<<std::endl;
        // Read the length of the name string
      size_t name_length;
      infile.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));
      std::cout<<name_length<<std::endl;
      // Read the name string
      std::string name(name_length, '\0');
      infile.read(&name[0], name_length);
      std::cout<<name<<std::endl;
      // Read the symbol table entry
      SymbolTableEntry entry;
      entry.deserialize(infile);
      
      
      // Insert the entry into the unordered_map
      symbolTable[name] = entry;
    }

    infile.close();
    


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

  for (auto& entry:symbolTable) {
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
  
  return 0;
}