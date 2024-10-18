#include <iostream>
#include <ostream>
#include <string>
#include <vector>

void name_list(std::vector<std::string> *v) {
  for (auto str : *v) {
    std::cout << str.c_str() << " is a name on the list" << std::endl;
  }
}
// allocating memory dynamically from a function

int *build_array(size_t size, int element_value) {
  int *new_storage{nullptr};

  new_storage = new int[size];

  for (size_t i{0}; i < size; i++) {
    *(new_storage + i) = element_value;
  }
  return new_storage;
}

int main(int argc, char *argv[]) {
  std::vector<int> voters_age;
  std::vector<int> x{20, 30, 40};
  x.at(1) = 9;
  voters_age.push_back(65);

  std::vector<std::string> name{"james", "Shirley", "jack"};
  name_list(&name);
  int *new_array(build_array(40, 12));

  delete[] new_array;
  return 0;
}
