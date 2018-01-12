
package de.tu_ilmenau.saltwater;

import android.app.NativeActivity;

import android.content.Context;

import android.os.Bundle;

public class GameActivity extends NativeActivity
{
    // -----------------------------------------------------------------------------
    // Testing area
    // -----------------------------------------------------------------------------
    public int GetNumber()
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
    public void onCreate(Bundle _SavedInstanceState)
    {
        super.onCreate(_SavedInstanceState);

        s_Instance = this;

        nativeSetActivityAndContext(getApplicationContext());
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
    // Native functions
    // -----------------------------------------------------------------------------
    public native void nativeSetActivityAndContext(Context context);

    // -----------------------------------------------------------------------------
    // Load libraries. E.g. app with native JNI interface
    // -----------------------------------------------------------------------------
    static
    {
        System.loadLibrary("app_droid");
    }
}
