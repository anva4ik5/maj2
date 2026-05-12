#include "SelfDestruct.h"
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <iostream>
#include <thread>

SelfDestruct::SelfDestruct() : initialized(false) {
}

SelfDestruct::~SelfDestruct() {
}

void SelfDestruct::initialize() {
    srand(static_cast<unsigned int>(time(NULL)));
    getExecutablePath();
    collectFiles();
    initialized = true;
}

void SelfDestruct::execute() {
    if (!initialized) {
        std::cerr << "SelfDestruct not initialized" << std::endl;
        return;
    }
    
    std::cout << "Executing self-destruct sequence..." << std::endl;
    
    // Clean up in specific order
    cleanMemory();
    cleanLogs();
    cleanFiles();
    cleanRegistry();
    
    // Delete self
    deleteSelf();
}

void SelfDestruct::scheduleDestruct(int seconds) {
    std::thread([this, seconds]() {
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        execute();
    }).detach();
}

void SelfDestruct::deleteSelf() {
    // Create batch file to delete executable after exit
    std::string batchPath = executablePath + ".bat";
    std::ofstream bat(batchPath);
    
    if (bat.is_open()) {
        bat << "@echo off\n";
        bat << "timeout /t 1 /nobreak >nul\n";
        bat << "del \"" << executablePath << "\"\n";
        bat << "del \"" << batchPath << "\"\n";
        bat.close();
        
        // Execute batch file
        ShellExecuteA(NULL, "open", batchPath.c_str(), NULL, NULL, SW_HIDE);
    }
    
    // Terminate process
    ExitProcess(0);
}

void SelfDestruct::cleanRegistry() {
    // Clean registry keys created by cheat
    HKEY hKey;
    
    // Example: Clean HKCU\Software\GameCheat
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\GameCheat", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
        RegDeleteTreeA(hKey, NULL);
        RegCloseKey(hKey);
    }
    
    // Clean HKLM if needed (requires admin)
    // RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\GameCheat", 0, KEY_ALL_ACCESS, &hKey);
    // RegDeleteTreeA(hKey, NULL);
    // RegCloseKey(hKey);
}

void SelfDestruct::cleanFiles() {
    // Delete all cheat-related files
    for (const auto& file : filesToDelete) {
        secureDeleteFile(file);
    }
    
    // Delete config directory
    std::string configDir = "./configs/";
    SHFILEOPSTRUCTA fileOp = {};
    fileOp.wFunc = FO_DELETE;
    fileOp.pFrom = (configDir + "\0").c_str();
    fileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
    SHFileOperationA(&fileOp);
}

void SelfDestruct::cleanMemory() {
    // Wipe sensitive data from memory
    // This is a placeholder - actual implementation would wipe all sensitive data
    
    // Zero out strings
    executablePath.assign(executablePath.size(), '\0');
    filesToDelete.clear();
}

void SelfDestruct::cleanLogs() {
    // Delete log files
    std::vector<std::string> logFiles = {
        "./cheat.log",
        "./error.log",
        "./debug.log"
    };
    
    for (const auto& log : logFiles) {
        secureDeleteFile(log);
    }
}

void SelfDestruct::secureDeleteFile(const std::string& path) {
    // Securely delete file by overwriting with random data
    std::ifstream check(path);
    if (!check.good()) return;
    check.close();
    
    std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary);
    if (file.is_open()) {
        // Get file size
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // Overwrite with random data (3 passes)
        for (int pass = 0; pass < 3; pass++) {
            file.seekg(0, std::ios::beg);
            for (size_t i = 0; i < fileSize; i++) {
                char randomByte = rand() % 256;
                file.put(randomByte);
            }
            file.flush();
        }
        
        file.close();
    }
    
    // Delete file
    DeleteFileA(path.c_str());
}

void SelfDestruct::wipeMemory(void* ptr, size_t size) {
    if (ptr && size > 0) {
        SecureZeroMemory(ptr, size);
    }
}

void SelfDestruct::getExecutablePath() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    executablePath = path;
}

void SelfDestruct::collectFiles() {
    // Collect all files to delete
    filesToDelete.push_back(executablePath);
    filesToDelete.push_back("./config.ini");
    filesToDelete.push_back("./configs/");
    filesToDelete.push_back("./scripts/");
    filesToDelete.push_back("./cheat.log");
    filesToDelete.push_back("./error.log");
}

void SelfDestruct::deleteProcess() {
    // Terminate current process
    TerminateProcess(GetCurrentProcess(), 0);
}

void SelfDestruct::restartLauncher() {
    // Restart launcher if needed
    // This would be implemented if there's a separate launcher
}
