
#include "app_droid/app_application.h"

void android_main(struct android_app* state) 
{
    App::Application::OnStart(state);

    App::Application::OnRun();

    App::Application::OnExit();
}
