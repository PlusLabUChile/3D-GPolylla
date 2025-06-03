//
// Created by vigb9 on 01-06-2025.
//
#include <Eigen/Dense>
#include <iostream>

// #include "polylla.h"

// using namespace Polylla;

void test(Eigen::ArrayXf* array) {
  Eigen::ArrayXf a(3);
  a << 1.0f, 2.0f, 3.0f;
  *array = a;
}

int main(int argc, char* argv[]) {
  // Vertex v(0, 0, 0);
  // std::cout << v << std::endl;
  Eigen::ArrayXf a;
  test(&a);
  std::cout << a << std::endl;
}
