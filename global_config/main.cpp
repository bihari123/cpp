#include "config.h"
#include <iostream>
void initConfig() {
  auto &config = Config::getInstance();
  config.setValue("database_url", "localhost::5432");
  config.setValue("username", "admin");
}
void printConfig() {
  auto &cfg = Config::getInstance();
  std::cout << "Database URL: " << cfg.getValue("database_url") << std::endl;
  std::cout << "Username: " << cfg.getValue("username") << std::endl;
}
int main(int argc, char *argv[]) {
  initConfig();
  printConfig();
  return 0;
}
