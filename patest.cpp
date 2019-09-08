#include <iostream>
#include <list>
#include <asyncplusplus/include/async++.h>

int main(void) {
  std::list<int> test;
  for(int i = 0; i != 1000; ++i) {
    test.push_back(i);
  }
  std::cout << "Right now, test is:";
  for(int &x: test) {
    std::cout << x << std::endl;
  }
  std::cout << "Changing.." << std::endl;
  async::parallel_for(test, [](int &x){
			      std::cout << x;
			      x *= 10;
			      std::cout << " -> " << x << std::endl;
			    });
  return 0;
}
