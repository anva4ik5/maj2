#pragma once

#include <string>
#include <vector>
#include <windows.h>

class HWIDManager {
public:
    HWIDManager();
    ~HWIDManager();
    
    // Get unique hardware ID
    std::string getHWID();
    
    // Get CPU ID
    std::string getCPUID();
    
    // Get Motherboard Serial
    std::string getMotherboardSerial();
    
    // Get MAC Address
    std::string getMACAddress();
    
    // Get Disk Serial
    std::string getDiskSerial();
    
    // Generate combined HWID hash
    std::string generateHWIDHash();
    
    // Verify HWID matches stored
    bool verifyHWID(const std::string& storedHWID);
    
    // Save HWID to file
    void saveHWID(const std::string& filename);
    
    // Load HWID from file
    std::string loadHWID(const std::string& filename);
    
private:
    std::string hwid;
    
    // Helper functions
    std::string executeCommand(const std::string& command);
    std::string hashString(const std::string& input);
    std::string bytesToHex(const unsigned char* bytes, size_t len);
};
