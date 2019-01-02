@echo off

:: this is where you set your SDL2 path based on your env
:: "SDL2_INCLUDE" where SDL2\include folder is located (fullpath)
:: "SDL2_LIB_PATH" where all the SDL2 libs are located (fullpath)

:: ---- CUSTOM SETTINGS ----
set SDL2_INCLUDE="C:\Dev\include"
set SDL2_LIB_PATH="C:\Dev\lib"
:: ---- CUSTOM SETTINGS ----







set SCRIPT_FOLDER=%~dp0

set WARNING_LEVEL=/W3
set OPTIMIZATION=/O2 /MD
set MACRO_DEF=/D_CRT_SECURE_NO_WARNINGS /DWINDOWS_OS /DX64_BUILD
set INCLUDE_PATH=/I%SDL2_INCLUDE% /I"%SCRIPT_FOLDER%src"
set COMPILE_COMMAND=/nologo %MACRO_DEF% %INCLUDE_PATH% %OPTIMIZATION% %WARNING_LEVEL% /c

set SDL2_LIBS=SDL2.lib SDL2main.lib SDL2_mixer.lib
set LINK_COMMAND=/NOLOGO /WX /INCREMENTAL:NO /DEBUG:NONE /MACHINE:X64 /LIBPATH:%SDL2_LIB_PATH% /SUBSYSTEM:CONSOLE %SDL2_LIBS%


mkdir win32_build 2>nul
mkdir win32_build\nothing_test 2>nul
mkdir win32_build\repl 2>nul


pushd win32_build
	call :clean
	
	cl %COMPILE_COMMAND% ..\src\color.c
    cl %COMPILE_COMMAND% ..\src\ebisp\builtins.c
    cl %COMPILE_COMMAND% ..\src\ebisp\expr.c
    cl %COMPILE_COMMAND% ..\src\ebisp\gc.c
    cl %COMPILE_COMMAND% ..\src\ebisp\interpreter.c
    cl %COMPILE_COMMAND% ..\src\ebisp\parser.c
    cl %COMPILE_COMMAND% ..\src\ebisp\scope.c
    cl %COMPILE_COMMAND% ..\src\ebisp\std.c
    cl %COMPILE_COMMAND% ..\src\ebisp\tokenizer.c
    cl %COMPILE_COMMAND% ..\src\game.c
    cl %COMPILE_COMMAND% ..\src\game\camera.c
    cl %COMPILE_COMMAND% ..\src\game\level.c
    cl %COMPILE_COMMAND% ..\src\game\level\background.c
    cl %COMPILE_COMMAND% ..\src\game\level\boxes.c
    cl %COMPILE_COMMAND% ..\src\game\level\goals.c
    cl %COMPILE_COMMAND% ..\src\game\level\labels.c
    cl %COMPILE_COMMAND% ..\src\game\level\lava.c
    cl %COMPILE_COMMAND% ..\src\game\level\lava\wavy_rect.c
    cl %COMPILE_COMMAND% ..\src\game\level\physical_world.c
    cl %COMPILE_COMMAND% ..\src\game\level\physical_world.c
    cl %COMPILE_COMMAND% ..\src\game\level\platforms.c
    cl %COMPILE_COMMAND% ..\src\game\level\player.c
    cl %COMPILE_COMMAND% ..\src\game\level\player\dying_rect.c
    cl %COMPILE_COMMAND% ..\src\game\level\player\rigid_rect.c
    cl %COMPILE_COMMAND% ..\src\game\level\regions.c
    cl %COMPILE_COMMAND% ..\src\game\level\script.c
    cl %COMPILE_COMMAND% ..\src\game\level\solid.c
    cl %COMPILE_COMMAND% ..\src\game\level_picker.c
    cl %COMPILE_COMMAND% ..\src\game\level_script.c
    cl %COMPILE_COMMAND% ..\src\game\sound_samples.c
    cl %COMPILE_COMMAND% ..\src\game\sprite_font.c
    cl %COMPILE_COMMAND% ..\src\math\mat3x3.c
    cl %COMPILE_COMMAND% ..\src\math\point.c
    cl %COMPILE_COMMAND% ..\src\math\rand.c
    cl %COMPILE_COMMAND% ..\src\math\rect.c
    cl %COMPILE_COMMAND% ..\src\math\triangle.c
    cl %COMPILE_COMMAND% ..\src\sdl\renderer.c
    cl %COMPILE_COMMAND% ..\src\str.c
    cl %COMPILE_COMMAND% ..\src\system\line_stream.c
    cl %COMPILE_COMMAND% ..\src\system\log.c
    cl %COMPILE_COMMAND% ..\src\system\log_script.c
    cl %COMPILE_COMMAND% ..\src\system\lt.c
    cl %COMPILE_COMMAND% ..\src\system\lt\lt_adapters.c
    cl %COMPILE_COMMAND% ..\src\system\lt\lt_slot.c
    cl %COMPILE_COMMAND% ..\src\system\nth_alloc.c
    cl %COMPILE_COMMAND% ..\src\system\stacktrace.c
    cl %COMPILE_COMMAND% ..\src\ui\console.c
    cl %COMPILE_COMMAND% ..\src\ui\console_log.c
    cl %COMPILE_COMMAND% ..\src\ui\edit_field.c
    cl %COMPILE_COMMAND% ..\src\ui\history.c
    cl %COMPILE_COMMAND% ..\src\main.c
	
	link /OUT:"nothing.exe" %LINK_COMMAND% *.obj
	
	if %errorlevel% neq 0 (
		echo Build of Nothing Failed :^(
		popd
		goto :eof
	)
popd


pushd win32_build\nothing_test
	call :clean
	
	cl %COMPILE_COMMAND% ..\..\src\ebisp\builtins.c
	cl %COMPILE_COMMAND% ..\..\src\ebisp\expr.c
	cl %COMPILE_COMMAND% ..\..\src\ebisp\gc.c
	cl %COMPILE_COMMAND% ..\..\src\ebisp\interpreter.c
	cl %COMPILE_COMMAND% ..\..\src\ebisp\parser.c
	cl %COMPILE_COMMAND% ..\..\src\ebisp\scope.c
	cl %COMPILE_COMMAND% ..\..\src\ebisp\tokenizer.c
	cl %COMPILE_COMMAND% ..\..\src\str.c
	cl %COMPILE_COMMAND% ..\..\src\system\log.c
	cl %COMPILE_COMMAND% ..\..\src\system\lt.c
	cl %COMPILE_COMMAND% ..\..\src\system\lt\lt_adapters.c
	cl %COMPILE_COMMAND% ..\..\src\system\lt\lt_slot.c
	cl %COMPILE_COMMAND% ..\..\src\system\nth_alloc.c
	cl %COMPILE_COMMAND% ..\..\src\system\stacktrace.c
	cl %COMPILE_COMMAND% ..\..\test\main.c
		
	link /OUT:"nothing_test.exe" %LINK_COMMAND% *.obj
	
	if %errorlevel% neq 0 (
		echo Build of Test Failed :^(
		popd
		goto :eof
	)
popd


echo Building Repl
pushd win32_build\repl
	call :clean
	
	cl %COMPILE_COMMAND% ..\..\src\ebisp\builtins.c
	cl %COMPILE_COMMAND% ..\..\src\ebisp\expr.c
	cl %COMPILE_COMMAND% ..\..\src\ebisp\gc.c
	cl %COMPILE_COMMAND% ..\..\src\ebisp\interpreter.c
	cl %COMPILE_COMMAND% ..\..\src\ebisp\parser.c
	cl %COMPILE_COMMAND% ..\..\src\ebisp\repl_runtime.c
	cl %COMPILE_COMMAND% ..\..\src\ebisp\scope.c
	cl %COMPILE_COMMAND% ..\..\src\ebisp\std.c
	cl %COMPILE_COMMAND% ..\..\src\ebisp\tokenizer.c
	cl %COMPILE_COMMAND% ..\..\src\str.c
	cl %COMPILE_COMMAND% ..\..\src\system\log.c
	cl %COMPILE_COMMAND% ..\..\src\system\lt.c
	cl %COMPILE_COMMAND% ..\..\src\system\lt\lt_adapters.c
	cl %COMPILE_COMMAND% ..\..\src\system\lt\lt_slot.c
	cl %COMPILE_COMMAND% ..\..\src\system\nth_alloc.c
	cl %COMPILE_COMMAND% ..\..\src\system\stacktrace.c
	cl %COMPILE_COMMAND% ..\..\src\ebisp\repl.c
	
	link /OUT:"repl.exe" %LINK_COMMAND% *.obj
	
	if %errorlevel% neq 0 (
		echo Build of Repl Failed :^(
		popd
		goto :eof
	)
popd


echo To start the game type: win32_build\nothing.exe levels\level-01.txt


goto :eof


:clean
	del /f /q *.obj *.exe 2>nul
	exit /b
