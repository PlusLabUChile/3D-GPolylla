//
// Created by vigb9 on 19-07-2025.
//
#include "logger.h"

#include <fstream>
#include <iostream>

using namespace Polylla;
using namespace std;

static ostream* out = &cout;

void Polylla::log(const std::string& message, LogLevel level) {
  switch (level) {
    case INFO:
      *out << "[INFO] " << message << endl;
      break;
    default:
      *out << message << endl;
      break;
  }
}
