#include "../inc/Linker.hpp"
#include <regex>

void handlePlace(std::string arg, std::unordered_map<std::string,unsigned int> &section_addr){
  std::string section_name="";
  std::string adr_string="";
  int adr;

  char* c = &arg[0];
  while(*c !='=')c++;
  c++;
  while(*c!='@'){
    section_name+=(*c);
    c++;
  }
  c++;
  while(*c!='\0'){
    adr_string+=(*c);
    c++;
  }
  if (adr_string[1] == 'x'){
    adr = stoul(adr_string,nullptr,16);
  }
  else{
    adr = stoi(adr_string);
  }

  section_addr[section_name] = adr;
  
}

int main(int argc, char* argv[]){
  //opcije: -o izlazna_datoteka -place=sekcija@adresa -hex + lista_fajlova
  //std::string filename = argv[1];

  /*
  Parsiranje opcija:
  */
  std::string output_file;
  std::vector<std::string> input_files; // ulaz, obavezno
  std::unordered_map<std::string,unsigned int> section_addr; //-place opcija
  bool hex = false;
  std::regex regex(R"(-place=([^@]+)@([^@]+))");
  bool print = false;


  for (int i=1;i<argc;i++){
    if (std::string(argv[i]) == "-p"){
      print = true;
      continue;
    }

    if (std::string(argv[i]) == "-o"){
      i++;
      output_file = std::string(argv[i]);
      continue;
    }

    if (std::string(argv[i])== "-hex"){
      hex = true;
      continue;
    }

    if (std::regex_match(std::string(argv[i]),regex)){
      handlePlace(std::string(argv[i]),section_addr);
      continue;
    }

    input_files.push_back(std::string(argv[i]));

  }

  Linker linker;
  linker.begin(input_files,section_addr,hex,output_file,print);

  return 0;
}