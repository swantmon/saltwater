
package de.tuilmenau.saltwater;

import android.app.NativeActivity;

import android.content.Context;

import android.graphics.Point;

import android.os.Build;
import android.os.Bundle;

import android.view.View;
import android.view.WindowManager;

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

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        // -----------------------------------------------------------------------------
        // Version specific stuff
        // -----------------------------------------------------------------------------
        if (android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.P)
        {
            getWindow().getAttributes().layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }

        // -----------------------------------------------------------------------------
        // Layout
        // -----------------------------------------------------------------------------
        getWindow().getDecorView().setSystemUiVisibility(
                  View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_IMMERSIVE);  // NOT sticky.. will be set to sticky later!

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

        RestoreTransparentBars();

        getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(new View.OnSystemUiVisibilityChangeListener()
        {
            @Override
            public void onSystemUiVisibilityChange(int _Visibility)
            {
                RestoreTransparentBars();
            }
        });

        getWindow().getDecorView().setOnFocusChangeListener(new View.OnFocusChangeListener()
        {
            @Override
            public void onFocusChange(View _View, boolean _HasFocus)
            {
                RestoreTransparentBars();
            }
        });
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

    public void RestoreTransparentBars()
    {
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        getWindow().getDecorView().setSystemUiVisibility(
                  View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
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
