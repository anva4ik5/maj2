@echo off
echo Installing vcpkg...
git clone https://github.com/Microsoft/vcpkg
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install curl:x64-windows
.\vcpkg install nlohmann-json:x64-windows
.\vcpkg integrate install
cd ..
echo Done!
