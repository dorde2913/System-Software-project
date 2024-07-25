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
        
        //vec imamo ovu sekciju
        if (name!=entry.section){
          if (temp_local_symbol_table[entry.section].find(name)!=temp_local_symbol_table[entry.section].end()) {
            std::cout<<"Greska, visestruka definicija lokalnog simbola: "<<name<<std::endl;
            return false;
          }
          //entry.value+=local_symbol_table[entry.section][entry.section].size;
          temp_local_symbol_table[entry.section][name] = entry;
        }
        else{
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

int Linker::begin(std::vector<std::string> input_files,std::unordered_map<std::string,unsigned int> place_addr,int hex, std::string output_file,bool print){

  //prvi prolaz
  for (auto& file:input_files){
    loadFile(file);
  }
  if (!checkSolved()){
    return -1;
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
    
    if (entry.second >= max_adr){
      location_counter+= local_symbol_table[entry.first][entry.first].size;
    }
    std::cout<<"location counter: "<<location_counter<<std::endl;
  }

  for (auto& entry:local_symbol_table){
    if (section_addr.find(entry.first) == section_addr.end()){
      section_addr[entry.first] = location_counter;
      location_counter+= entry.second[entry.first].size;
    }
  }
  printFinalAddr();

  //sad treba razresiti sve vrednosti simbola
  for (auto& entry:local_symbol_table){
    entry.second[entry.first].value = section_addr[entry.first];
  }
  int symbol_value;
  char byte1,byte2,byte3,byte4;
  for (auto& reloc_entry:relocation_table){
    for (auto& sym:reloc_entry.second){
      
      if (local_symbol_table.find(sym.symbol) != local_symbol_table.end()){
        //sekcija
        symbol_value = local_symbol_table[reloc_entry.first][sym.symbol].value+sym.addend;
        byte1 = (symbol_value >> 24) & 0xFF; // Most significant byte
        byte2 = (symbol_value >> 16) & 0xFF;
        byte3 = (symbol_value >> 8) & 0xFF;
        byte4 = symbol_value & 0xFF; // Least significant byte

        section_contents[reloc_entry.first][sym.offset-18] = byte1;
        section_contents[reloc_entry.first][sym.offset-17] = byte2;
        section_contents[reloc_entry.first][sym.offset-2] = byte3;
        section_contents[reloc_entry.first][sym.offset-1] = byte4;
      }
      else if (global_symbol_table.find(sym.symbol) != global_symbol_table.end()){
        //drugi neki simbol
        symbol_value = global_symbol_table[sym.symbol].value + section_addr[global_symbol_table[sym.symbol].section];
        std::cout<<"Simbol: "<<sym.symbol<<" value: "<<symbol_value<<std::endl;
        byte1 = (symbol_value >> 24) & 0xFF; // Most significant byte
        byte2 = (symbol_value >> 16) & 0xFF;
        byte3 = (symbol_value >> 8) & 0xFF;
        byte4 = symbol_value & 0xFF; // Least significant byte

        section_contents[reloc_entry.first][sym.offset-18] = byte1;
        section_contents[reloc_entry.first][sym.offset-17] = byte2;
        section_contents[reloc_entry.first][sym.offset-2] = byte3;
        section_contents[reloc_entry.first][sym.offset-1] = byte4;
      }
      else{
        std::cout<<"Error, nedefinisan simbol: "<<sym.symbol<<std::endl;
        return -1;
      }
    }
  }

  if (print){
    printTables();
  }
  if (!checkSections()){
    return -1;
  }

  generateOutput(output_file);
  

  return 0;
}
void Linker::printFinalAddr(){
  for (auto& entry:section_addr){
    std::cout<<"Section: "<<entry.first<<" Address: "<<std::hex<<entry.second<<std::endl;
  }
}

void Linker::generateOutput(std::string output_name){

  std::ofstream outfile("./"+output_name);
  if (!outfile) {
      std::cerr << "Error opening file for writing: " << output_name << std::endl;
      return;
  }


  std::vector<unsigned int> starting_adr;
  for (auto& section:section_addr){
    starting_adr.push_back((unsigned int)section.second);
  }

  std::sort(starting_adr.begin(),starting_adr.end());

  for (auto& adr:starting_adr){
    std::string section_name;
    for (auto& section:section_addr){
      if (section.second == adr){
        section_name = section.first;
        break;
      } 
    }

    //section_name i adr
    bool flag = false;
    for (int i=0;i<section_contents[section_name].size();i+=8){
      int n=0;
      outfile <<std::hex<< adr+i <<": ";
      while(n<8){
        if (i+n == section_contents[section_name].size()){
          flag = true;
          break;
        }
        unsigned char byte = section_contents[section_name][i+n];
        
        //std::cout<<byte<<" "<<first_half<<" "<<second_half<<std::endl;
        outfile<<std::hex<<std::setw(2)<<std::setfill('0')<<(int)byte<<' ';
        n++;
      }
      outfile<<'\n';
      if (flag)break;
    }
  }


  outfile.close();

}
bool Linker::checkSections(){
  for (auto& entry:local_symbol_table){
    int end = entry.second[entry.first].value + entry.second[entry.first].size;
    int start = entry.second[entry.first].value;

    for (auto& other_section:local_symbol_table){
      int other_start = other_section.second[other_section.first].value;
      int other_end = other_section.second[other_section.first].value + other_section.second[other_section.first].size;

      if (other_section.first != entry.first && ((start < other_start && other_start < end)||(start < other_end && other_end < end))){
        std::cout<<"Greska, sekcije "<<entry.first<<" i "<<other_section.first<<" se preklapaju!"<<std::endl;
        std::cout<<"sekcija "<<entry.first<<" pocinje na "<<entry.second[entry.first].value <<" i zavrsava na "<<end<<std::endl;
        std::cout<<"sekcija "<<other_section.first<<" pocinje na "<<other_section.second[other_section.first].value <<" i zavrsava na "<<other_section.second[other_section.first].value +other_section.second[other_section.first].size<<std::endl;
        return false;
      }
    }
  }
  return true;
}


