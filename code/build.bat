@echo off

set CodeDir=..\code
set OutputDir=..\build_win32

set CompilerFlags=-O2 -Zi -nologo
set LinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib

If NOT EXIST %OutputDir% mkdir %OutputDir%

pushd %OutputDir%

del *.pdb > NUL 2> NUL

call cl %CompilerFlags% %CodeDir%\win32_graphics.cpp -Fmwin32_graphics.map /link %LinkerFlags%

popd