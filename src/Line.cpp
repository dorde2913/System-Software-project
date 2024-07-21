#include "../inc/Line.hpp"
Line::Line(std::string line){
  this->text = line;
}

std::ostream& operator<<(std::ostream& os, Line line){
  return os<<line.text<<std::endl;
}
/********************************************
 * U liniji moze da postoji komentar
 * Moze da postoji tacno 1 ili 0 LABELA, labela UVEK NA POCETKU
 * najvise 1 ili naredbu ili direktivu
*********************************************/

std::string Line::getLabel(){
  //labele uvek na pocetku i zavrsavaju se sa :
  int index = 0;
  int len = this->text.size();
  std::string ret_val = "";
  while (index < len && this->text[index]!=':')index++;
  if (index<len){
    //imamo labelu
    ret_val = this->text.substr(0,index);
  }
  return ret_val;
}
bool Line::hasLabel(){
  return this->getLabel()!="";
}

std::pair<std::string,std::vector<std::string>> Line::getDirective(){
  //direktive pocinju sa ., ima samo 1 po liniji ali moze da postoji i labela uz to
  //tako da trazimo substring od tacke do blanko
  //uz direktive idu i neki parametri ili stavec, tkd mozda treba da ih podelimo
  int index1 = 0,index2 = 0;
  
  int len = this->text.size();
  std::string directive_name = "";
  std::vector<std::string> operands;

  while (index1 < len && this->text[index1]!='.')index1++;
  if (index1<len){
    //nasli smo tacku, odnosno pocetak direktive, ako nema tacke vracamo prazno
    index2 = index1;
    while (index2<len && this->text[index2]!=' ')index2++;
    directive_name = this->text.substr(index1,index2);
    //sad i nije bitno da li je dosao do kraja, i end of line je kraj direktive
    while (index2<len && this->text[index2]==' ')index2++;  
    
    //sada imamo ime direktive, vadimo pojedinacne operande

    
    std::string operands_string = this->text.substr(index2,len-index2);
    operands = this->extractOperands(operands_string);
    
  }
  return std::pair<std::string,std::vector<std::string>>(directive_name,operands);

}
bool Line::hasDirective(){
  return this->getDirective().first!="";
}
std::pair<std::string,std::string> Line::getInstruction(){
//na pocetku moze da postoji labela, tkd proveravamo da li ima, ako ima onda je preskacemo
  int index1 = 0,index2 = 0;
  int len = this->text.size();
  std::string instruction = "";
  std::string operands = "";
  if (!this->hasDirective()){
    if (this->hasLabel()){
      while (this->text[index1]!=':')index1++;
      index1++;
    }
    while (this->text[index1] == ' ')index1++;
    //u index1 nam se sada sigurno nalazi pocetak instrukcije
    index2 = index1;
    

    while(index2<len && this->text[index2]!=' ')index2++;

    instruction = this->text.substr(index1,index2-index1);
    if (index2 !=len) index2++;
    operands = this->text.substr(index2,len-index2);
  }
  return std::pair<std::string,std::string>(instruction,operands);
  
}
bool Line::hasInstruction(){
  return this->getInstruction().first!="";
}


void Line::cleanComments(){
  int index = 0;
  int len = this->text.size();
  while (index<len && this->text[index]!='#') index++;
  if (index<len){
    this->text = this->text.substr(0,index);
  }
}

std::vector<std::string> Line::extractOperands(std::string operand_string){
    int index1 = 0;
    int index2 = 0;
    int len = operand_string.size();
    std::vector<std::string> operands;
    while (index2<len){
      //std::cout<<"u while-u"<<std::endl;
      while(operand_string[index2]!=',' && index2<len)index2++;
      std::string operand = operand_string.substr(index1,index2-index1);
      while (operand_string[index2] == ',' || operand_string[index2] == ' ')index2++;
      if (operand!=""){
        operands.push_back(operand);
      }
      index1 = index2;
    }
    return operands;
}
bool Line::isLiteral(std::string symbol){
  std::regex number_pattern(R"(^(0x[0-9A-Fa-f]+|\d+)$)");
  if (std::regex_match(symbol,number_pattern)) return true;
  return false;
}
bool Line::isHex(std::string literal){
  if (literal[0] == '0' && literal[1] =='x')return true;
  return false;
}
