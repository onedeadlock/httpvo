#include <iostream>

int main(void)
{
  int x = 0;
  int y = 8;
  auto& v = x;
  std::cout << v << std::endl;
  v = y;
  std::cout << v << std::endl;
  return 0;
}
