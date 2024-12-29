@echo off

gcc -Os -g -o ./build/bullet_hell2.exe ./src/*.c -I./raylib/include -I./include ./build/raylib.dll ./build/lua54.dll 

IF NOT EXIST ".\build\scripts" MKDIR .\build\scripts
clua.bat