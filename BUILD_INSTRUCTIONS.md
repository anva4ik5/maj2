# Инструкции по установке и сборке

## Проблема
Компиляторы (CMake, MSVC, g++) не установлены в системе PATH.

## Решение 1: Установка Visual Studio Build Tools (рекомендуется)

### Шаг 1: Скачайте Visual Studio Build Tools
1. Перейдите на https://visualstudio.microsoft.com/downloads/
2. Скачайте "Build Tools for Visual Studio 2022"
3. Запустите установщик

### Шаг 2: Установите необходимые компоненты
В установщике выберите:
- "Desktop development with C++" (Разработка классических приложений на C++)
- Или выберите индивидуальные компоненты:
  - MSVC v143 - VS 2022 C++ x64/x86 build tools
  - Windows 10 SDK (или Windows 11 SDK)
  - CMake tools for Visual Studio

### Шаг 3: Настройте переменные окружения
После установки:
1. Откройте "Developer Command Prompt for VS 2022" из меню Пуск
2. Или добавьте пути в системные переменные:
   - `C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\{версия}\bin\Hostx64\x64`
   - `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\{версия}\bin\Hostx64\x64`

### Шаг 4: Сборка проекта
```bash
cd C:\Users\SERVER\Desktop\GameCheat_Restored
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

Или напрямую с MSVC:
```bash
cd C:\Users\SERVER\Desktop\GameCheat_Restored
cl /EHsc /std:c++17 /I. main.cpp src/*.cpp /link d3d11.lib d3dcompiler.lib wininet.lib ws2_32.lib psapi.lib /OUT:GameCheat_Restored.exe
```

## Решение 2: Установка MinGW-w64

### Шаг 1: Скачайте MinGW-w64
1. Перейдите на https://www.mingw-w64.org/
2. Или используйте MSYS2: https://www.msys2.org/

### Шаг 2: Установите MinGW
Для MSYS2:
```bash
pacman -S mingw-w64-x86_64-gcc
```

### Шаг 3: Добавьте в PATH
Добавьте `C:\msys64\mingw64\bin` в системные переменные PATH

### Шаг 4: Сборка проекта
```bash
cd C:\Users\SERVER\Desktop\GameCheat_Restored
g++ -std=c++17 main.cpp src/*.cpp -o GameCheat_Restored.exe -ld3d11 -ld3dcompiler -lwininet -lws2_32 -lpsapi -static
```

## Решение 3: Использование Visual Studio IDE

### Шаг 1: Откройте проект в Visual Studio
1. Установите Visual Studio Community (бесплатно)
2. Откройте папку `GameCheat_Restored` в Visual Studio
3. Visual Studio автоматически определит CMake проект

### Шаг 2: Соберите проект
1. Нажмите `Ctrl+Shift+B` для сборки
2. Или выберите "Build" -> "Build All"

## Автоматическая установка (PowerShell)

Создайте и запустите скрипт `install_and_build.ps1`:

```powershell
# Проверка наличия Visual Studio Build Tools
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -property installationPath
    if ($vsPath) {
        Write-Host "Visual Studio найдена: $vsPath"
        # Запуск Developer Command Prompt
        $devCmd = "$vsPath\VC\Auxiliary\Build\vcvars64.bat"
        cmd /c "$devCmd && cd C:\Users\SERVER\Desktop\GameCheat_Restored && mkdir build && cd build && cmake .. && cmake --build . --config Release"
    }
} else {
    Write-Host "Visual Studio не найдена. Пожалуйста, установите Visual Studio Build Tools."
    Write-Host "Скачать можно: https://visualstudio.microsoft.com/downloads/"
}
```

## Быстрый старт (если есть Visual Studio)

Если у вас установлена Visual Studio, используйте "Developer Command Prompt for VS":

1. Нажмите `Win`, введите "Developer Command Prompt for VS"
2. Запустите его
3. Выполните:
```bash
cd C:\Users\SERVER\Desktop\GameCheat_Restored
cl /EHsc /std:c++17 main.cpp src/*.cpp /link d3d11.lib d3dcompiler.lib wininet.lib ws2_32.lib psapi.lib
```

## Проверка установки

После установки проверьте доступность инструментов:

```bash
# Проверка CMake
cmake --version

# Проверка MSVC
cl

# Проверка g++ (если установлен MinGW)
g++ --version
```

## Если ничего не помогло

Создайте упрощенную версию без DirectX и сложных зависимостей, которая будет работать только с памятью и сетью.
