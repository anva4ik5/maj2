#pragma once

#include <string>
#include <vector>

class SelfDestruct {
public:
    SelfDestruct();
    ~SelfDestruct();
    
    void initialize();
    void execute();
    void scheduleDestruct(int seconds);
    
    // Safety: SelfDestruct does NOTHING unless explicitly armed.
    // Call arm() only when a real panic situation is detected (admin spectating, ban incoming).
    void arm()    { armed = true; }
    void disarm() { armed = false; }
    bool isArmed() const { return armed; }
    
    // Cleanup methods
    void deleteSelf();
    void cleanRegistry();
    void cleanFiles();
    void cleanMemory();
    void cleanLogs();
    
    // Secure deletion
    void secureDeleteFile(const std::string& path);
    void wipeMemory(void* ptr, size_t size);
    
private:
    bool initialized;
    bool armed = false;
    std::string executablePath;
    std::vector<std::string> filesToDelete;
    
    void getExecutablePath();
    void collectFiles();
    void deleteProcess();
    void restartLauncher();
};
