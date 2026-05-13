# GameCheat — Инструкция по запуску

## 0. Архитектура

- **Backend (Node.js + PostgreSQL)** — крутится на Railway, обслуживает Telegram-бота и API авторизации/лицензий.
- **Клиент (C++ exe)** — собирается через GitHub Actions, скачивается готовым `.exe`. Подключается к backend через HTTPS.

---

## 1. Запуск backend (один раз, уже сделано)

В Railway проекте должны быть переменные окружения:

```
TELEGRAM_BOT_TOKEN   = <токен_бота_от_@BotFather>
ADMIN_TELEGRAM_ID    = <ваш_telegram_id>
API_SECRET_KEY       = <случайная_строка_например_64_символа>
NODE_ENV             = production
DATABASE_URL         = (Railway добавит автоматически от PostgreSQL plugin)
```

После пуша backend Railway сам подцепит репу `anva4ik5/majcheat` и задеплоит.

После старта в Telegram сделать `/start` боту — он сам зарегистрирует вас как админа (по `ADMIN_TELEGRAM_ID`).

---

## 2. Сборка клиента

Клиент собирается автоматически через GitHub Actions в репе `anva4ik5/maj2`:

1. Сделай `git push` в `main` (workflow `Build GameCheat` запустится).
2. Перейди в **Actions** на GitHub → последний запуск.
3. Скачай артефакт `GameCheat-EXE` — внутри `GameCheat_Restored.exe`.

Локально можно собрать через:

```powershell
cmake -B build -S . -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

---

## 3. Настройка клиента

Рядом с `GameCheat_Restored.exe` положи `config.ini`:

```ini
[Server]
url=https://majcheat-production.up.railway.app
shared_key=<ТОТ_ЖЕ_API_SECRET_KEY_ЧТО_В_RAILWAY>

[General]
target_process=auto
```

`target_process=auto` будет искать (в порядке): `altv.exe`, `altv-launcher.exe`, `GTA5.exe`, `rust.exe`.

Можно явно: `target_process=altv.exe` или несколько: `altv.exe,GTA5.exe`.

---

## 4. Порядок запуска: СНАЧАЛА СОФТ, ПОТОМ ИГРА

> ⚠️ **Важно:** запускайте `GameCheat_Restored.exe` **ДО** запуска игры.

### Почему:
- Софт сначала просит авторизацию (логин/пароль/код из Telegram) — это происходит до того, как игра запущена.
- Античит-байпасы и хуки нужно установить до того, как игра загрузит свой античит-модуль (Majestic AC, alt:V client, и т.д.).
- Если запустить после игры, многие защиты уже активированы, и часть функций не сработает.

### Шаги запуска:

1. **Запусти `GameCheat_Restored.exe`** (двойной клик).
2. В консоли появится меню:
   ```
   === Авторизация ===
   1) Войти
   2) Зарегистрироваться
   3) Выход
   ```
3. **Первый раз — выбирай `2) Зарегистрироваться`:**
   - Введи логин (3-32 символа, латиница/цифры/`_`).
   - Введи пароль (минимум 6 символов, не отображается).
   - Введи свой Telegram ID (получить можно у `@userinfobot` в Telegram).
   - Откроется бот в Telegram и пришлёт 6-значный код.
   - Введи код в консоль.
4. **Следующие запуски:**
   - Софт сам подхватит сохранённый токен из `auth.dat` и пропустит вход.
   - Если запустишь на другом ПК — попросит код из Telegram (2FA).
5. **После успешной авторизации** запусти игру (alt:V/GTA5/Rust).
6. Софт ждёт пока процесс игры появится, потом инжектится и активирует функции.
7. **F11 / END** — открыть/закрыть меню (по умолчанию `END`).

---

## 5. Telegram-бот: команды

### Для всех:
- `/start` — главное меню
- `/help` — список команд

### Только для админа:
- `/createkey <дни> <telegram_id>` — выдать ключ
- `/extendkey <key> <дней>` — продлить
- `/revoke <key>` — отозвать
- `/delkey <key>` — удалить
- `/reset_hwid <key>` — сбросить HWID привязку
- `/userkeys <tg_id>` — все ключи юзера
- `/finduser <tg_id>` — инфо о юзере
- `/banuser <tg_id>` / `/unbanuser <tg_id>`
- `/addadmin <tg_id>` — назначить нового админа
- `/stats` — статистика
- `/logs` — последние действия

---

## 6. Возможные проблемы

| Проблема | Решение |
|---|---|
| `[Auth] Не удалось отправить код в Telegram` | Сначала открой бота и нажми `/start` — Telegram не позволяет отправлять сообщения тем, кто не запустил бота. |
| `Failed to find process. Tried: altv.exe ...` | Игра не запущена или процесс называется иначе. Запусти игру или укажи точное имя в `target_process=`. |
| `HWID mismatch` | Ты вошёл с другого ПК. Подтверди код из Telegram, HWID обновится. |
| `Account banned` | Админ забанил аккаунт. Свяжись с админом. |
| Бот молчит на нажатие кнопок | Проверь что в Railway переменная `TELEGRAM_BOT_TOKEN` корректная и `ADMIN_TELEGRAM_ID` совпадает с твоим. |
| Антивирус ругается на `.exe` | Это нормально для overlay-софта (хуки в чужой процесс). Добавь в исключения. |

---

## 7. Поддерживаемые игры

- **alt:V** (`altv.exe`, `altv-launcher.exe`) — GTA V multiplayer
- **GTA V / Majestic RP / RAGEMP** (`GTA5.exe`)
- **Rust** (`rust.exe`)

---

## 8. Файлы которые софт создаёт

| Файл | Что внутри |
|---|---|
| `auth.dat` | Сохранённый токен авторизации (рядом с .exe). Удали для перелогина. |
| `config.ini` | Настройки. Можно править вручную. |

---

## 9. Безопасность

- Пароли хешируются bcrypt'ом на backend, никогда не сохраняются на клиенте в plain.
- Token действует 30 дней.
- На новый HWID — обязательное подтверждение через Telegram (2FA).
- API защищён `X-API-Key` (значение `shared_key` из config.ini = `API_SECRET_KEY` из Railway).
