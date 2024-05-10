#include "../inc/Assembler.hpp"

void Assembler::startAssembly(std::string input_file,std::string output_file){
  this->file_name = input_file;
  this->output_name = output_file;

  Parser& parser = Parser::getInstance();

  parser.setFile(input_file);

  while (!Parser::end){
    //std::cout<<"u while"<<std::endl;
    Line line = parser.parseLine();
    this->addLine(line);
  }
  //nakon dodavanja svih linija treba da se odradi backtrack
  bool solved = this->solveSymbols();
  if (!solved){
    std::cout<<"ERROR"<<std::endl;
  }
}
void Assembler::addLine(Line& line){
  line.cleanComments();
  if (line.empty()) return; //linija je imala samo komentar, ignorisemo je
  if (line.hasLabel()){
    std::cout<<line.getLabel()<<std::endl;
  }

  if (line.hasDirective()){
    std::cout<<line.getDirective().first<<"|"<<line.getDirective().second<<std::endl;
  }

  if (line.hasInstruction()){
    std::cout<<line.getInstruction().first<<"|"<<line.getInstruction().second<<std::endl;
  }


}
bool Assembler::solveSymbols(){
  return false;
}