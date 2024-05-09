#ifndef _LINE_HPP
#define _LINE_HPP
#include <string>
#include <iostream>

class Line{
  private:
  std::string text;

  public:
  Line(std::string line);

  friend std::ostream& operator<<(std::ostream& os, Line line);
};

#endif