#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>

class JSExecutor {
public:
    JSExecutor();
    ~JSExecutor();
    
    void initialize();
    void loadScript(const std::string& filename);
    void removeScript(const std::string& filename);
    void editScript(const std::string& filename, const std::string& content);
    void listScripts();
    
    void execute(const std::string& code);
    void executeScript(const std::string& filename);
    
    void setScriptsPath(const std::string& path) { scriptsPath = path; }
    std::string getScriptsPath() const { return scriptsPath; }
    
    std::vector<std::string> getScriptList() const;
    
private:
    std::string scriptsPath;
    std::map<std::string, std::string> scripts;
    
    bool enabled;
    
    void registerAPI();
    void loadAllScripts();
    std::string readFile(const std::string& filename);
    void writeFile(const std::string& filename, const std::string& content);
    
    // API functions exposed to JS
    void api_log(const std::string& message);
    void api_print(const std::string& message);
    void api_setAimbotEnabled(bool enabled);
    void api_setESPEnabled(bool enabled);
    void api_setGodMode(bool enabled);
    void api_teleport(float x, float y, float z);
    void api_setHP(int hp);
    void api_getHP();
    void api_getPosition();
};
