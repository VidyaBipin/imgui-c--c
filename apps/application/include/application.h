#pragma once
#include <imgui.h>
#include <string>
#include <vector>
#if defined(__APPLE__)
#define FULLSCREEN_OFFSET_X  0
#define FULLSCREEN_OFFSET_Y  0
#define FULLSCREEN_WIDTH_ADJ  0
#define FULLSCREEN_HEIGHT_ADJ  0
#elif defined(__linux__)
#define FULLSCREEN_OFFSET_X  72
#define FULLSCREEN_OFFSET_Y  0
#define FULLSCREEN_WIDTH_ADJ  64
#define FULLSCREEN_HEIGHT_ADJ  0
#else
#define FULLSCREEN_OFFSET_X  0
#define FULLSCREEN_OFFSET_Y  0
#define FULLSCREEN_WIDTH_ADJ  0
#define FULLSCREEN_HEIGHT_ADJ  32
#endif

typedef void (*SetupContext)(ImGuiContext* ctx);
typedef void (*Initialize)(void** handle);
typedef void (*Finalize)(void** handle);
typedef bool (*SplashScreen)(void* handle, bool app_will_quit);
typedef bool (*Frame)(void* handle, bool app_will_quit);
typedef void (*DropFromSystem)(std::vector<std::string>& drops);

typedef struct ApplicationFrameworks
{
    SetupContext        Application_SetupContext    {nullptr};
    Initialize          Application_Initialize      {nullptr};
    Finalize            Application_Finalize        {nullptr};
    SplashScreen        Application_SplashScreen    {nullptr};
    Frame               Application_Frame           {nullptr};
    DropFromSystem      Application_DropFromSystem  {nullptr};
} ApplicationFrameworks;

typedef struct ApplicationWindowProperty
{
    ApplicationWindowProperty() {}
    ApplicationWindowProperty(int _argc, char** _argv) { argc = _argc; argv = _argv; }
    std::string name;
    int pos_x       {0};
    int pos_y       {0};
    int width       {1440};
    int height      {960};
    float font_scale{1.0};
    float fps       {30.f};
    bool resizable  {true};
    bool docking    {true};
    bool viewport   {true};
    bool auto_merge {true};
    bool center     {true};
    bool power_save {true};
    bool full_screen{false};
    bool full_size  {false};
    bool using_setting_path {true};
    bool internationalize {false};
    bool top_most   {false};
    bool window_border {true};
    std::string language_path;
    std::string icon_path;
    int splash_screen_width {0};
    int splash_screen_height {0};
    void* handle    {nullptr};
    ApplicationFrameworks application;
    int argc    {0};
    char ** argv {nullptr};
} ApplicationWindowProperty;

void Application_Setup(ApplicationWindowProperty& property);
void Application_FullScreen(bool on);