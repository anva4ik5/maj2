#pragma once

#include <string>
#include "BackendAPI.h"

// Console-based authorization flow.
// Returns true on successful auth and fills `token`/`login`/`telegramId`.
// Persists token to disk (auth.dat) so subsequent runs auto-verify.
class AuthFlow {
public:
    AuthFlow(BackendAPI* api, const std::string& hwid);

    bool run(std::string& outToken, std::string& outLogin, std::string& outTelegramId);

private:
    BackendAPI* api;
    std::string hwid;

    // Persistence
    static std::string tokenFilePath();
    static bool loadToken(std::string& token);
    static void saveToken(const std::string& token);
    static void clearToken();

    // Console helpers
    static std::string promptLine(const std::string& prompt);
    static std::string promptHidden(const std::string& prompt);

    bool tryAutoLogin(std::string& outToken, std::string& outLogin, std::string& outTelegramId);
    bool doRegister(std::string& outToken, std::string& outLogin, std::string& outTelegramId);
    bool doLogin(std::string& outToken, std::string& outLogin, std::string& outTelegramId);
};
