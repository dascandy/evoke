#pragma once

#include <map>
#include <string>
#include <fw/filesystem.hpp>

struct Component;

void ReloadPredefComponents();
Component *GetPredefComponent(const fs::path &path);

