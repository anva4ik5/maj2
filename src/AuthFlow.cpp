#include "AuthFlow.h"
#include <iostream>
#include <fstream>
#include <conio.h>
#include <windows.h>

AuthFlow::AuthFlow(BackendAPI* apiPtr, const std::string& hwidVal)
    : api(apiPtr), hwid(hwidVal) {}

std::string AuthFlow::tokenFilePath() {
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    std::string p = path;
    size_t pos = p.find_last_of("\\/");
    if (pos != std::string::npos) p = p.substr(0, pos + 1);
    p += "auth.dat";
    return p;
}

bool AuthFlow::loadToken(std::string& token) {
    std::ifstream f(tokenFilePath(), std::ios::binary);
    if (!f) return false;
    std::getline(f, token);
    return !token.empty();
}

void AuthFlow::saveToken(const std::string& token) {
    std::ofstream f(tokenFilePath(), std::ios::binary | std::ios::trunc);
    if (f) f << token;
}

void AuthFlow::clearToken() {
    DeleteFileA(tokenFilePath().c_str());
}

std::string AuthFlow::promptLine(const std::string& prompt) {
    std::cout << prompt << std::flush;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

std::string AuthFlow::promptHidden(const std::string& prompt) {
    std::cout << prompt << std::flush;
    std::string s;
    int c;
    while ((c = _getch()) != '\r' && c != '\n') {
        if (c == 8 || c == 127) {
            if (!s.empty()) {
                s.pop_back();
                std::cout << "\b \b" << std::flush;
            }
        } else if (c == 3) {
            // Ctrl+C
            exit(0);
        } else if (c >= 32 && c < 127) {
            s += static_cast<char>(c);
            std::cout << '*' << std::flush;
        }
    }
    std::cout << std::endl;
    return s;
}

bool AuthFlow::tryAutoLogin(std::string& outToken, std::string& outLogin, std::string& outTelegramId) {
    std::string saved;
    if (!loadToken(saved)) return false;

    std::cout << "[Auth] Found saved session, verifying..." << std::endl;
    AuthResult r = api->verifyToken(saved, hwid);
    if (r.success) {
        outToken = saved;
        outLogin = r.login;
        outTelegramId = r.telegramId;
        std::cout << "[Auth] Welcome back, " << r.login << "!" << std::endl;
        return true;
    }
    std::cout << "[Auth] Saved session invalid: " << r.message << std::endl;
    clearToken();
    return false;
}

bool AuthFlow::doRegister(std::string& outToken, std::string& outLogin, std::string& outTelegramId) {
    std::cout << "\n=== Регистрация ===" << std::endl;
    std::cout << "Сначала запустите бота в Telegram (/start) чтобы он мог отправить вам код." << std::endl;

    std::string login, password, telegramId;
    while (true) {
        login = promptLine("Логин (3-32 символа, a-z, 0-9, _): ");
        if (login.size() >= 3 && login.size() <= 32) break;
        std::cout << "Неверный логин." << std::endl;
    }
    while (true) {
        password = promptHidden("Пароль (мин. 6 символов): ");
        if (password.size() >= 6) break;
        std::cout << "Слишком короткий пароль." << std::endl;
    }
    while (true) {
        telegramId = promptLine("Ваш Telegram ID: ");
        bool digits = !telegramId.empty();
        for (char c : telegramId) if (c < '0' || c > '9') { digits = false; break; }
        if (digits) break;
        std::cout << "ID должен состоять из цифр." << std::endl;
    }

    std::cout << "[Auth] Отправка запроса..." << std::endl;
    AuthResult r = api->registerStart(login, password, telegramId, hwid);
    if (!r.success) {
        std::cout << "[Auth] Ошибка: " << r.message << std::endl;
        return false;
    }
    std::cout << "[Auth] Код отправлен в Telegram. Введите его сюда." << std::endl;

    for (int attempt = 0; attempt < 3; ++attempt) {
        std::string code = promptLine("Код: ");
        AuthResult c = api->registerConfirm(r.sessionId, code);
        if (c.success) {
            saveToken(c.token);
            outToken = c.token;
            outLogin = login;
            outTelegramId = telegramId;
            std::cout << "[Auth] Регистрация успешна! Добро пожаловать, " << login << "." << std::endl;
            return true;
        }
        std::cout << "[Auth] Ошибка: " << c.message << std::endl;
    }
    return false;
}

bool AuthFlow::doLogin(std::string& outToken, std::string& outLogin, std::string& outTelegramId) {
    std::cout << "\n=== Вход ===" << std::endl;

    std::string login = promptLine("Логин: ");
    std::string password = promptHidden("Пароль: ");

    std::cout << "[Auth] Проверка..." << std::endl;
    AuthResult r = api->login(login, password, hwid);
    if (!r.success) {
        std::cout << "[Auth] Ошибка: " << r.message << std::endl;
        return false;
    }

    if (r.requireCode) {
        std::cout << "[Auth] Вход с нового устройства. Код отправлен в Telegram." << std::endl;
        for (int attempt = 0; attempt < 3; ++attempt) {
            std::string code = promptLine("Код: ");
            AuthResult c = api->loginConfirm(r.sessionId, code);
            if (c.success) {
                saveToken(c.token);
                outToken = c.token;
                outLogin = login;
                outTelegramId = c.telegramId;
                std::cout << "[Auth] Вход подтверждён." << std::endl;
                return true;
            }
            std::cout << "[Auth] Ошибка: " << c.message << std::endl;
        }
        return false;
    }

    // Direct login
    saveToken(r.token);
    outToken = r.token;
    outLogin = login;
    outTelegramId = r.telegramId;
    std::cout << "[Auth] Вход выполнен." << std::endl;
    return true;
}

bool AuthFlow::run(std::string& outToken, std::string& outLogin, std::string& outTelegramId) {
    if (tryAutoLogin(outToken, outLogin, outTelegramId)) return true;

    while (true) {
        std::cout << "\n=== Авторизация ===" << std::endl;
        std::cout << "1) Войти" << std::endl;
        std::cout << "2) Зарегистрироваться" << std::endl;
        std::cout << "3) Выход" << std::endl;
        std::string choice = promptLine("Выбор: ");

        if (choice == "1") {
            if (doLogin(outToken, outLogin, outTelegramId)) return true;
        } else if (choice == "2") {
            if (doRegister(outToken, outLogin, outTelegramId)) return true;
        } else if (choice == "3") {
            return false;
        } else {
            std::cout << "Неверный выбор." << std::endl;
        }
    }
}
