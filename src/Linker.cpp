#include "../inc/Linker.hpp"
void Linker::printTables(){
  //print za test
  std::cout << "--------------------------------------------------"<<std::endl;
  std::cout<<"Global Symbol table: "<<std::endl;
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

  for (auto& entry:global_symbol_table) {
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
  std::cout<<"Local Symbol tables: "<<std::endl;
  std::cout << "--------------------------------------------------"<<std::endl;
  for (auto& e:local_symbol_table){
    std::cout<<"SECTION: "<<e.first<<std::endl;
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

    for (auto& entry:e.second) {
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
  std::unordered_map<std::string, SymbolTableEntry> temp_global_symbol_table;
  std::unordered_map<std::string,std::unordered_map<std::string,SymbolTableEntry>> temp_local_symbol_table;
  std::unordered_map<std::string,std::vector<char>> temp_section_contents;
  std::unordered_map<std::string,std::vector<RelocationTableEntry>> temp_relocation_table;
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
    //proveriti da li postoji ovaj name, ako postoji i extern je a nas entry nije, ubacujemo def,
    //ako su i postojeci i nas extern onda nista, ako oba nisu extern to je greska dupla definicija
    //na kraju linkovanja ako je ostalo ista extern to je greska nedefinisan simbol :)

    




    if (!entry.is_global && !entry.is_extern){
      if (temp_local_symbol_table.find(entry.section)!=temp_local_symbol_table.end()){
        
        std::cout<<"Vec imamo sekciju "<<entry.section<<std::endl;
        //vec imamo ovu sekciju
        if (name!=entry.section){
          if (temp_local_symbol_table[entry.section].find(name)!=temp_local_symbol_table[entry.section].end()) {
            std::cout<<"Greska, visestruka definicija lokalnog simbola: "<<name<<std::endl;
            return false;
          }
          //entry.value+=local_symbol_table[entry.section][entry.section].size;
          temp_local_symbol_table[entry.section][name] = entry;
        }
        
      }
      else{
        temp_local_symbol_table[entry.section][name] = entry;
      }
      
    }
    else{
      
      if (temp_global_symbol_table.find(name)!= temp_global_symbol_table.end()){
        //prisutan name
        if (temp_global_symbol_table[name].is_extern && !entry.is_extern){
          temp_global_symbol_table[name] = entry;
        }
        else if (!temp_global_symbol_table[name].is_extern && !entry.is_extern){
          std::cout<<"ERROR, visestruka definicija simbola: "<<name<<std::endl;
          return false;
        }
      }
      else{
        //nije prisutan
        temp_global_symbol_table[name] = entry;
      }
    }
    
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
      
      
      temp_relocation_table[section_name].push_back(new_entry);
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
      temp_section_contents[section_name].push_back(c);
    }
  }
  
  //prolazimo kroz lokalnu tabelu -> kad naidjemo na sekciju prodjemo kroz sve globalne(koji nisu extern) i dodamo na value
  //kad naidjemo na nesto drugo samo dodajemo na value ako ta sekcija postoji
  //DODAVANJE U FULL TABELU TEK POSLE 
  //prolazimo kroz globalne simbole, ako imamo duplikat ali sad definisan ubacujemo, kao i ako nije duplikat
  //prolaz kroz reloc tabelu,na svaki offset dodajemo velicinu postojece sekcije
  //prolaz kroz section_contents samo se sve nadovezuje

  for (auto& local_entry:temp_local_symbol_table){
    for (auto& global_entry:local_symbol_table){
      if (local_entry.first == global_entry.first){
        //imamo duplikat sekciju
        for (auto& entry:local_entry.second){
          if (entry.second.type!=3){
            entry.second.value+=global_entry.second[local_entry.first].size;
          }
        }

        for (auto& entry:temp_global_symbol_table){
          if (entry.second.section == local_entry.first){
            entry.second.value+=global_entry.second[local_entry.first].size;
          }
        }
      }
    }
  }

  for (auto& temp_entry:temp_global_symbol_table){
    //for (auto& global_entry:global_symbol_table){
    if (global_symbol_table.find(temp_entry.first) != global_symbol_table.end()){
      auto global_entry = global_symbol_table[temp_entry.first];
      if (!global_entry.is_extern && !temp_entry.second.is_extern){
        std::cout<<"Error, dupla definicija globalnog simbola: "<<temp_entry.first<<std::endl;
        return false;
      }
      if (global_entry.is_extern){
        global_symbol_table[temp_entry.first] = temp_entry.second;
      }
    }
    else{
        global_symbol_table[temp_entry.first] = temp_entry.second;
    }
  }

  for (auto& temp_entry:temp_relocation_table){
    for (auto& global_entry:relocation_table){
      if (global_entry.first == temp_entry.first){
        for (auto& entry:temp_entry.second){
          entry.offset += local_symbol_table[global_entry.first][global_entry.first].size;
        }
      }
    }
  }
  for (auto& temp_entry:temp_relocation_table){
    for (auto& entry:temp_entry.second){
      relocation_table[temp_entry.first].push_back(entry);
    }
  }

  for (auto& temp_entry:temp_section_contents){
    for (auto& entry:temp_entry.second){
      section_contents[temp_entry.first].push_back(entry);
    }
  }

  for (auto& temp_entry:temp_local_symbol_table){
    for (auto& global_entry:local_symbol_table){
      if (temp_entry.first == global_entry.first){
        local_symbol_table[temp_entry.first][temp_entry.first].size+=temp_entry.second[temp_entry.first].size;
      }
    }
  }
  for (auto& temp_entry:temp_local_symbol_table){
    if (local_symbol_table.find(temp_entry.first) != local_symbol_table.end()){
      if (local_symbol_table[temp_entry.first][temp_entry.first].type!=3){
        //duplikat
        std::cout<<"Error, dupla definicija lokalnog simbola: "<<temp_entry.first<<std::endl;
        return false;
      }
      
    }
    else{
      local_symbol_table[temp_entry.first] = temp_entry.second;
    }
  }


  infile.close();
  return true;
}
bool Linker::checkSolved(){
  bool solved = true;
  for (auto& entry:global_symbol_table){
    if (entry.second.is_extern) {
      std::cout<<"Nedefinisan simbol: "<<entry.first<<std::endl;
      solved = false;
    }
  }
  return solved;
}

int Linker::begin(std::vector<std::string> input_files,std::unordered_map<std::string,int> place_addr,int hex, std::string output_file){

  //prvi prolaz
  for (auto& file:input_files){
    loadFile(file);
  }
  if (checkSolved()){
    printTables();
  }
  else{
    printTables();
    //return -1;
  } 
  
  //drugi prolaz
  for (auto& entry:place_addr){
    if (local_symbol_table.find(entry.first) == local_symbol_table.end()){
      std::cout<<"Error, u place opciji navedena nepostojeca sekcija: "<<entry.first<<std::endl;
      return -1;
    }
    for (auto& e:section_addr){
      if (e.second == entry.second){
        std::cout<<"Error, nije moguce postaviti 2 razlicite sekcije na istu adresu ("<<e.first<<", "<<entry.first<<")\n";
        return -1;
      }
    }
    section_addr[entry.first] = entry.second;
    if (entry.second > max_adr){
      max_adr = entry.second;
      location_counter = max_adr;
    } 

    location_counter+= local_symbol_table[entry.first][entry.first].size;
  }

  for (auto& entry:local_symbol_table){
    if (section_addr.find(entry.first) == section_addr.end()){
      section_addr[entry.first] = location_counter;
      location_counter+= entry.second[entry.first].size;
    }
  }
  printFinalAddr();

  //sad treba razresiti sve vrednosti simbola, i pobrinuti se za spajanje istoimenih sekcija

  return 0;
}
void Linker::printFinalAddr(){
  for (auto& entry:section_addr){
    std::cout<<"Section: "<<entry.first<<" Address: "<<std::hex<<entry.second<<std::endl;
  }
}