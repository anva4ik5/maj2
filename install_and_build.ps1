# Скрипт автоматической установки и сборки
# Требует прав администратора

Write-Host "=== GameCheat - Установка и сборка ===" -ForegroundColor Green

# Проверка наличия Visual Studio Build Tools
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsInstalled = $false
$vsPath = ""

if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -property installationPath
    if ($vsPath) {
        Write-Host "✓ Visual Studio найдена: $vsPath" -ForegroundColor Green
        $vsInstalled = $true
    }
}

if (-not $vsInstalled) {
    Write-Host "✗ Visual Studio не найдена" -ForegroundColor Red
    Write-Host "Пожалуйста, установите Visual Studio Build Tools:" -ForegroundColor Yellow
    Write-Host "https://visualstudio.microsoft.com/downloads/" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Выберите 'Desktop development with C++' при установке" -ForegroundColor Yellow
    Read-Host "Нажмите Enter после установки Visual Studio"
    
    # Повторная проверка
    if (Test-Path $vsWhere) {
        $vsPath = & $vsWhere -latest -property installationPath
        if ($vsPath) {
            $vsInstalled = $true
        }
    }
}

if ($vsInstalled) {
    Write-Host ""
    Write-Host "=== Сборка проекта ===" -ForegroundColor Green
    
    $projectDir = "C:\Users\SERVER\Desktop\GameCheat_Restored"
    $buildDir = "$projectDir\build"
    
    # Создание директории build
    if (-not (Test-Path $buildDir)) {
        New-Item -ItemType Directory -Path $buildDir -Force
        Write-Host "✓ Создана директория build" -ForegroundColor Green
    }
    
    # Проверка наличия CMake
    $cmakeInstalled = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmakeInstalled) {
        Write-Host "✓ CMake найден" -ForegroundColor Green
        
        # Сборка с CMake
        Write-Host "Запуск CMake..." -ForegroundColor Yellow
        Push-Location $buildDir
        cmake ..
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✓ CMake конфигурация успешна" -ForegroundColor Green
            cmake --build . --config Release
            if ($LASTEXITCODE -eq 0) {
                Write-Host "✓ Сборка успешна!" -ForegroundColor Green
                Write-Host "Исполняемый файл: $buildDir\bin\Release\GameCheat_Restored.exe" -ForegroundColor Cyan
            } else {
                Write-Host "✗ Ошибка сборки" -ForegroundColor Red
            }
        } else {
            Write-Host "✗ Ошибка CMake конфигурации" -ForegroundColor Red
        }
        Pop-Location
    } else {
        Write-Host "✗ CMake не найден" -ForegroundColor Yellow
        Write-Host "Попытка сборки напрямую с cl.exe..." -ForegroundColor Yellow
        
        # Прямая сборка с cl.exe
        $devCmd = "$vsPath\VC\Auxiliary\Build\vcvars64.bat"
        
        $batchFile = "$projectDir\build_temp.bat"
        "@echo off" | Out-File $batchFile
        "call `"$devCmd`"" | Out-File $batchFile -Append
        "cd /d $projectDir" | Out-File $batchFile -Append
        "cl /EHsc /std:c++17 main.cpp src/*.cpp /link d3d11.lib d3dcompiler.lib wininet.lib ws2_32.lib psapi.lib /OUT:GameCheat_Restored.exe" | Out-File $batchFile -Append
        
        Write-Host "Запуск сборки..." -ForegroundColor Yellow
        & cmd /c $batchFile
        
        if (Test-Path "$projectDir\GameCheat_Restored.exe") {
            Write-Host "✓ Сборка успешна!" -ForegroundColor Green
            Write-Host "Исполняемый файл: $projectDir\GameCheat_Restored.exe" -ForegroundColor Cyan
        } else {
            Write-Host "✗ Ошибка сборки" -ForegroundColor Red
        }
        
        Remove-Item $batchFile -Force
    }
}

Write-Host ""
Write-Host "=== Готово ===" -ForegroundColor Green
Write-Host "Для запуска перейдите в директорию проекта и запустите GameCheat_Restored.exe" -ForegroundColor Cyan
Write-Host "Не забудьте настроить config.ini перед запуском" -ForegroundColor Yellow
