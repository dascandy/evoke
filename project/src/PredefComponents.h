#pragma once

#include <map>
#include <string>

struct Component;

std::map<std::string, Component *> PredefComponentList();
