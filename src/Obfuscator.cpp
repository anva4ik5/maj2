#include "Obfuscator.h"
#include <algorithm>

std::string Obfuscator::xorString(const std::string& input, const std::string& key) {
    std::string output = input;
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = input[i] ^ key[i % key.size()];
    }
    return output;
}

std::string Obfuscator::deobfuscate(const std::string& obfuscated, const std::string& key) {
    return xorString(obfuscated, key);
}

std::string Obfuscator::rot13(const std::string& input) {
    std::string output = input;
    for (char& c : output) {
        if ((c >= 'A' && c <= 'Z')) c = ((c - 'A' + 13) % 26) + 'A';
        else if ((c >= 'a' && c <= 'z')) c = ((c - 'a' + 13) % 26) + 'a';
    }
    return output;
}

void Obfuscator::secureClear(std::string& str) {
    volatile char* p = str.data();
    for (size_t i = 0; i < str.size(); i++) {
        p[i] = 0;
    }
    str.clear();
}
