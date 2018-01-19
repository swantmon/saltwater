
package de.tuilmenau.saltwater;

import android.app.NativeActivity;

import android.content.Context;

import android.graphics.Point;

import android.os.Bundle;

public class GameActivity extends NativeActivity
{
    private static final String LOG_TAG = "GameActivity";

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

        nativeInitializeInterface(getApplicationContext());
    }

    // -----------------------------------------------------------------------------

    @Override
    public void onStart()
    {
        super.onStart();
    }

    // -----------------------------------------------------------------------------

    @Override
    public void onResume()
    {
        super.onResume();
    }

    // -----------------------------------------------------------------------------

    @Override
    protected void onPause()
    {
        super.onPause();
    }

    // -----------------------------------------------------------------------------

    @Override
    public void onStop()
    {
        super.onStop();
    }

    // -----------------------------------------------------------------------------

    @Override
    public void onDestroy()
    {
        super.onDestroy();
    }

    // -----------------------------------------------------------------------------
    // Device
    // -----------------------------------------------------------------------------
    public int GetDeviceOrientation()
    {
        return getResources().getConfiguration().orientation;
    }

    // -----------------------------------------------------------------------------

    public int GetDeviceRotation()
    {
        return getWindowManager().getDefaultDisplay().getRotation();
    }

    // -----------------------------------------------------------------------------

    public int GetDeviceDimensionWidth()
    {
        Point Size = new Point();

        getWindowManager().getDefaultDisplay().getSize(Size);

        return Size.x;
    }

    // -----------------------------------------------------------------------------

    public int GetDeviceDimensionHeight()
    {
        Point Size = new Point();

        getWindowManager().getDefaultDisplay().getSize(Size);

        return Size.y;
    }

    // -----------------------------------------------------------------------------
    // Native functions
    // -----------------------------------------------------------------------------
    public native void nativeInitializeInterface(Context context);

    // -----------------------------------------------------------------------------
    // Load libraries. E.g. app with native JNI interface
    // -----------------------------------------------------------------------------
    static
    {
        System.loadLibrary("app_droid");
    }
}
