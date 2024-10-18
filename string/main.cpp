#include <cstring>
#include <iostream>
#include <string>
int main(int argc, char *argv[]) {

  std::string test1("this is a string");
  char test2[50];
  std::strcpy(test2, test1.c_str()); // copying from string into char[]
  std::string test3(test1);
  std::string test4; // empty string
  test4.assign(test1);
  std::string test5("before concate  ");
  test5.append(test1);
  test5.clear(); // clear the string and make it empty
  return 0;
}
