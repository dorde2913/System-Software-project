#ifndef _LINE_HPP
#define _LINE_HPP
#include <string>
#include <iostream>
#include <vector>
class Line{
  private:
  std::string text;

  
  public:
  Line(std::string line);
  /******************************************
   * Sta treba ova klasa da odradi sve?
   * Treba da izbrise komentare,
   * da izvuce direktive, labele, simbole iz linije
   * da izvuce literale iz linije
   * i instrukcije
  *********************************************/

  void cleanComments(); //metoda koja ce da obrise komentare jer se oni ignorisu
  
  bool empty(){return this->text.size() == 0;}
  friend std::ostream& operator<<(std::ostream& os, Line line);
  std::string getLabel();
  std::pair<std::string,std::vector<std::string>> getDirective();
  std::pair<std::string,std::string> getInstruction();
  
  bool hasLabel();
  bool hasDirective();
  bool hasInstruction();

  std::vector<std::string> extractOperands(std::string operand_string);
  
  bool isLiteral(std::string symbol);
};

#endif