#pragma once

extern CustomEngine::Application* CustomEngine::CreateApplication();

#ifdef CE_PLATFORM_WINDOWS

#ifdef CE_WINDOWS_NO_CONSOLE

#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    auto app = CustomEngine::CreateApplication();
    app->run();
    delete app;
}

#else

int main(int argc, char** argv)
{
    auto app = CustomEngine::CreateApplication();
    app->run();
    delete app;
}

#endif

#endif