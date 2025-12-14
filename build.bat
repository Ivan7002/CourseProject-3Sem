@echo off
REM Batch file to compile RIA language source code and assemble the generated assembly
REM Usage: build.bat [input_file]
REM If no input_file specified, uses RIA-2025\in.txt by default
REM
REM IMPORTANT:
REM - If you changed RIA COMPILER source code (.cpp files), run rebuild_compiler.bat first!
REM - If you changed ASSEMBLER source code, rebuild Assembler project in Visual Studio!
REM
REM Current limitation: Assembler uses hardcoded out.asm file and needs manual rebuild

if "%~1"=="" (
    set INPUT_FILE=RIA-2025\in.txt
    set INPUT_NAME=in
    echo Using default input file: RIA-2025\in.txt
) else (
    set INPUT_FILE=%~1
    set INPUT_NAME=%~n1
    echo Using input file: %INPUT_FILE%
)

echo.
echo ========================================
echo Compiling %INPUT_FILE%...
echo ========================================

REM Run the RIA-2025 compiler
RIA-2025\Debug\RIA-2025.exe -in:%INPUT_FILE%

if errorlevel 1 (
    echo.
    echo Compilation failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Assembling %INPUT_NAME%.txt.asm...
echo ========================================

REM Copy the generated asm file to the assembler's source file
copy "RIA-2025\%INPUT_NAME%.txt.asm" "Assembler\out.asm" >nul

echo Assembler source file updated. Rebuilding assembler automatically...
echo.

REM Auto-rebuild assembler with new ASM code
echo Looking for MSBuild...
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
    echo Found MSBuild 2022. Rebuilding assembler...
    "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" "Assembler\Assembler.vcxproj" /t:Clean /p:Configuration=Debug /p:Platform=x86 /verbosity:quiet
    "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" "Assembler\Assembler.vcxproj" /p:Configuration=Debug /p:Platform=x86 /verbosity:minimal
    if errorlevel 1 (
        echo ERROR: Assembler rebuild failed!
        echo Please rebuild manually in Visual Studio.
    ) else (
        echo Assembler rebuilt successfully.
    )
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" (
    echo Found MSBuild 2019. Rebuilding assembler...
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" "Assembler\Assembler.vcxproj" /t:Clean /p:Configuration=Debug /p:Platform=x86 /verbosity:quiet
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" "Assembler\Assembler.vcxproj" /p:Configuration=Debug /p:Platform=x86 /verbosity:minimal
    if errorlevel 1 (
        echo ERROR: Assembler rebuild failed!
        echo Please rebuild manually in Visual Studio.
    ) else (
        echo Assembler rebuilt successfully.
    )
) else (
    echo WARNING: MSBuild not found in standard locations.
    echo Looking for MSBuild in PATH...
    where msbuild >nul 2>&1
    if errorlevel 1 (
        echo ERROR: MSBuild not found in PATH either!
        echo Please add MSBuild to PATH or rebuild assembler manually in Visual Studio.
        echo Project file: Assembler\Assembler\Assembler.vcxproj
    ) else (
        echo Found MSBuild in PATH. Rebuilding assembler...
        msbuild "Assembler\Assembler.vcxproj" /t:Clean /p:Configuration=Debug /p:Platform=x86 /verbosity:quiet
        msbuild "Assembler\Assembler.vcxproj" /p:Configuration=Debug /p:Platform=x86 /verbosity:minimal
        if errorlevel 1 (
            echo ERROR: Assembler rebuild failed!
        ) else (
            echo Assembler rebuilt successfully.
        )
    )
)

echo.

REM Run the assembler with updated code
Assembler\Debug\Assembler.exe

if errorlevel 1 (
    echo.
    echo Assembly failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo RIA program executed successfully!
echo ========================================
pause
