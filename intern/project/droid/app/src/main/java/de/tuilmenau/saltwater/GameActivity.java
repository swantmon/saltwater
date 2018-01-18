
package de.tuilmenau.saltwater;

import android.app.NativeActivity;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;

import android.Manifest;

import android.net.Uri;

import android.provider.Settings;

import android.os.Bundle;

import android.widget.Toast;

public class GameActivity extends NativeActivity
{
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
    public void onStart() {
        super.onStart();
    }

    // -----------------------------------------------------------------------------

    @Override
    public void onResume()
    {
        super.onResume();

        if (!HasCameraPermission())
        {
            RequestCameraPermission();

            return;
        }

        RequestCameraPermission();
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

    @Override
    public void onRequestPermissionsResult(int _RequestCode, String[] _Permissions, int[] _Results)
    {
        if (!HasCameraPermission())
        {
            Toast.makeText(this, "Camera permission is needed to run this application", Toast.LENGTH_LONG).show();

            if (!ShouldShowRequestPermissionRationale())
            {
                // Permission denied with checking "Do not ask again".
                LaunchPermissionSettings();
            }

            finish();
        }
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
    // Camera permissions
    // -----------------------------------------------------------------------------
    public boolean HasCameraPermission()
    {
        return this.checkSelfPermission(Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED;
    }

    // -----------------------------------------------------------------------------

    public void RequestCameraPermission()
    {
        this.requestPermissions(new String[] {Manifest.permission.CAMERA}, 0);
    }

    // -----------------------------------------------------------------------------

    public boolean ShouldShowRequestPermissionRationale()
    {
        return this.shouldShowRequestPermissionRationale(Manifest.permission.CAMERA);
    }

    // -----------------------------------------------------------------------------

    public void LaunchPermissionSettings()
    {
        Intent intent = new Intent();
        intent.setAction(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
        intent.setData(Uri.fromParts("package", this.getPackageName(), null));

        this.startActivity(intent);
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
