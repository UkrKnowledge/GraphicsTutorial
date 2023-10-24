@echo off

set CodeDir=..\code
set DataDir=..\data
set OutputDir=..\build_win32
set DxcDir=..\libs\dxc\bin\x64

set CompilerFlags=-Od -Zi -nologo -I ..\libs
set LinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib ..\libs\assimp\libs\assimp-vc142-mt.lib dxgi.lib d3d12.lib

If NOT EXIST %OutputDir% mkdir %OutputDir%

pushd %DataDir%

call %DxcDir%\dxc.exe -E ModelVsMain -Fo ModelVsMain.shader -T vs_6_0 -Zi -Zpc -Qembed_debug %CodeDir%\model_shaders.cpp
call %DxcDir%\dxc.exe -E ModelPsMain -Fo ModelPsMain.shader -T ps_6_0 -Zi -Zpc -Qembed_debug %CodeDir%\model_shaders.cpp

popd %DataDir%

pushd %OutputDir%

del *.pdb > NUL 2> NUL

call cl %CompilerFlags% %CodeDir%\win32_graphics.cpp -Fmwin32_graphics.map /link %LinkerFlags%

popd