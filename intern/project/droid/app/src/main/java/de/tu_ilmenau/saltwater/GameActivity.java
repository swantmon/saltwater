
package de.tu_ilmenau.saltwater;

import android.app.NativeActivity;
import android.os.Bundle;

public class GameActivity extends NativeActivity
{
    // -----------------------------------------------------------------------------
    // Testing area
    // -----------------------------------------------------------------------------
    public static int GetTest() {return 42; }

    public int GetHello()
    {
        return 42;
    }

    // -----------------------------------------------------------------------------
    // Instance
    // -----------------------------------------------------------------------------
    static GameActivity s_Instance;

    public static GameActivity GetInstance()
    {
        return s_Instance;
    }

    // -----------------------------------------------------------------------------
    // App lifecycle
    // -----------------------------------------------------------------------------
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        s_Instance = this;
    }

    @Override
    public void onStart() {
        super.onStart();
    }

    @Override
    public void onResume()
    {
        super.onResume();
    }

    @Override
    protected void onPause()
    {
        super.onPause();
    }

    @Override
    public void onStop()
    {
        super.onStop();
    }

    @Override
    public void onDestroy()
    {
        super.onDestroy();
    }

    // -----------------------------------------------------------------------------
    // Load libraries. E.g. app with native JNI interface
    // -----------------------------------------------------------------------------
    static
    {
        System.loadLibrary("app_droid");
    }
}
