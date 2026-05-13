#pragma once
#include <string>

// Simple compile-time string obfuscation helper
class Obfuscator {
public:
    static std::string xorString(const std::string& input, const std::string& key);
    static std::string deobfuscate(const std::string& obfuscated, const std::string& key);
    static std::string rot13(const std::string& input);
    static void secureClear(std::string& str);
};
