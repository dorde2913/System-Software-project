#include "../inc/Linker.hpp"
void Linker::printTables(){
  //print za test
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
    std::cout<<section.first<<std::endl;
    for (auto& b:section.second){
      if (count!=0 && count%32 == 0)std::cout<<std::endl;
      std::cout<<std::bitset<8>(b)<<" ";
      count+=8;
    }
    std::cout<<std::endl;
  }
}

bool Linker::loadFile(std::string filename){
  std::ifstream infile("./"+filename, std::ios::binary);
  if (!infile) {
      std::cerr << "Error opening file for reading: " << filename << std::endl;
      return false;
  }
  std::cout<<"opened file: "<<filename<<std::endl;
  // Read the number of entries
  size_t size;
  infile.read(reinterpret_cast<char*>(&size), sizeof(size));
  for (size_t i = 0; i < size; ++i) {
      // Read the length of the name string
    size_t name_length;
    infile.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));
    // Read the name string
    std::string name(name_length, '\0');
    infile.read(&name[0], name_length);
    // Read the symbol table entry
    SymbolTableEntry entry;
    entry.deserialize(infile);
    
    
    // Insert the entry into the unordered_map
    symbol_table[name] = entry;
  }

  //velicina reloc tabele
  infile.read(reinterpret_cast<char*>(&size), sizeof(size));
  for (int i=0;i<size;i++){
    
    size_t name_length;
    infile.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));

    std::string section_name(name_length,'\0');
    infile.read(&section_name[0],name_length);
    
    size_t temp;
    infile.read(reinterpret_cast<char*>(&temp), sizeof(temp));

    for (int i=0;i<temp;i++){
      RelocationTableEntry new_entry;
      infile.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));
      new_entry.symbol = std::string(name_length,'\0');
      infile.read(&new_entry.symbol[0],name_length);
      
      infile.read(reinterpret_cast<char*>(&new_entry.addend), sizeof(new_entry.addend));
      infile.read(reinterpret_cast<char*>(&new_entry.offset), sizeof(new_entry.offset));
      
      relocation_table[section_name].push_back(new_entry);
    }
    
    
  }

  infile.read(reinterpret_cast<char*>(&size), sizeof(size));
  for (int i=0;i<size;i++){
    size_t name_length;
    infile.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));
    std::string section_name(name_length,'\0');
    infile.read(&section_name[0],name_length);

    infile.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));
    char c;
    
    for (int i=0;i<name_length;i++){
      infile.read(reinterpret_cast<char*>(&c), sizeof(c));
      section_contents[section_name].push_back(c);
    }
  }
  infile.close();
  return true;
}