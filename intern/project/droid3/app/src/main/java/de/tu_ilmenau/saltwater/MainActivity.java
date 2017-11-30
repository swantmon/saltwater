package de.tu_ilmenau.saltwater;

import android.app.Activity;
import android.os.Bundle;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.SurfaceHolder;

public class MainActivity extends Activity implements SurfaceHolder.Callback
{
    static
    {
        System.loadLibrary("app_droid");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);

        SurfaceView surfaceView = (SurfaceView)findViewById(R.id.surfaceView);

        surfaceView.getHolder().addCallback(this);
    }

    @Override
    protected void onStart()
    {
        super.onStart();
        nativeOnStart();
    }

    @Override
    protected void onResume()
    {
        super.onResume();
        nativeOnResume();
    }

    @Override
    protected void onPause()
    {
        super.onPause();
        nativeOnPause();
    }

    @Override
    protected void onStop()
    {
        super.onStop();
        nativeOnStop();
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h)
    {
        nativeSetSurface(holder.getSurface());
    }

    public void surfaceCreated(SurfaceHolder holder)
    {
    }

    public void surfaceDestroyed(SurfaceHolder holder)
    {
        nativeSetSurface(null);
    }

    public static native void nativeOnStart();
    public static native void nativeOnResume();
    public static native void nativeOnPause();
    public static native void nativeOnStop();
    public static native void nativeSetSurface(Surface surface);
}
