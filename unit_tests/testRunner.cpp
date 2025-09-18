#include <gtest/gtest.h>
#include <iostream>

using namespace std;

int main(int argc, char** argv) {
  cout << "Setting up the GTest Framework" << endl;
  testing::InitGoogleTest(&argc, argv);

  cout << "Running tests" << endl;
  auto status = RUN_ALL_TESTS();
  cout << "Tests finished, result: " << status << endl;

  return status;
}
