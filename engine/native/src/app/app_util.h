#pragma once
//
// app/app_util.h — small portable string/format helpers shared across the split
// native_app translation units. Deliberately free of <windows.h> and RmlUi so it
// can be included anywhere (behavior-neutral extraction from app/main.cpp).
//
#include <string>
#include <vector>

namespace ftd::native::app {

// Split a comma-separated list, trimming spaces and dropping empty fields.
std::vector<std::string> split_csv(const std::string& s);

std::string to_lower(std::string s);
std::string upper(std::string s);

// printf into a std::string. fmt takes one double; fmt3 takes three.
std::string fmt(const char* f, double v);
std::string fmt3(const char* f, double a, double b, double c);

}  // namespace ftd::native::app
