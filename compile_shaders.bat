:: Bat file location is here because call command path can't be long
@echo Compiling shaders...
@echo off

set shaderDir=%CD%\src\shaders
set vkCompilerDir=%CD%\lib\vulkan\1.3.216.0\Bin\glslc.exe
cd %shaderDir%

@echo on

for %%i in ("%shaderDir%\*.vert")do %vkCompilerDir% "%%~i" -o "%%~i.spv"
for %%i in ("%shaderDir%\*.frag")do %vkCompilerDir% "%%~i" -o "%%~i.spv"

@echo Finished compiling shaders.
@exit 0