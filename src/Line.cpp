#include "../inc/Line.hpp"
Line::Line(std::string line){
  this->text = line;
}

std::ostream& operator<<(std::ostream& os, Line line){
  return os<<line.text<<std::endl;
}
/*
sta sve moze da se nadje u jednoj liniji?
moze tacno 1 komentar koji pocinje sa #
moze najvise 1 asemblerska direktiva ili naredba/instrukcija
-> napravimo containsDirective i containsInstruction metode 
labela (moze i vise mislim) text koji se zavrsava dvotackom (:)
labela MORA na pocetku linije, moze samostalno ali moze i uz pratecu
direktivu/naredbu 
ako stoji samostalno to je kao da stoji na pocetku prve naredne linije koja ima
sadrzaj

*/