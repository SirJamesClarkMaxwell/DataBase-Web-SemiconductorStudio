#pragma once
#include "pch.hpp"
#include "yaml-cpp/yaml.h"
void yaml()
{
    YAML::Emitter out;
    out << "Hello, World!";

    std::cout << "Here's the output YAML:\n"
              << out.c_str(); // prints "Hello, World!"
};
