if "%DevEnvDir%" == "" call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
devenv vs2019\morph.sln /build "Debug|x64" /Project morph
devenv vs2019\morph.sln /build "Release|x64" /Project morph
