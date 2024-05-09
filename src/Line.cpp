#include "../inc/Line.hpp"
Line::Line(std::string line){
  this->text = line;
}

std::ostream& operator<<(std::ostream& os, Line line){
  return os<<line.text<<std::endl;
}