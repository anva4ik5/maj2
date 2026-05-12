#pragma once

#include <string>
#include <map>
#include <functional>
#include <windows.h>

enum class BindType {
    TOGGLE,  // Нажать один раз - включить, еще раз - выключить
    HOLD,    // Удерживать - включено, отпустить - выключено
    TRIGGER  // Срабатывает при каждом нажатии
};

struct KeyBind {
    int keyCode;
    BindType type;
    std::string name;
    std::function<void()> action;
    bool isActive;
    bool wasPressed;
    
    KeyBind() : keyCode(0), type(BindType::TOGGLE), isActive(false), wasPressed(false) {}
};

class KeyBindManager {
public:
    KeyBindManager();
    ~KeyBindManager();
    
    void initialize();
    void update();
    
    // Добавить бинд
    void addBind(const std::string& name, int keyCode, BindType type, std::function<void()> action);
    void removeBind(const std::string& name);
    
    // Изменить бинд
    void setBindKey(const std::string& name, int keyCode);
    void setBindType(const std::string& name, BindType type);
    
    // Получить информацию о бинде
    KeyBind* getBind(const std::string& name);
    std::vector<std::string> getBindNames() const;
    
    // Сохранить/загрузить бинды
    void saveBinds(const std::string& filename);
    void loadBinds(const std::string& filename);
    
    // Сбросить все бинды
    void resetBinds();
    
private:
    std::map<std::string, KeyBind> binds;
    
    bool isKeyPressed(int keyCode);
    std::string keyCodeToString(int keyCode);
    int stringToKeyCode(const std::string& str);
};
