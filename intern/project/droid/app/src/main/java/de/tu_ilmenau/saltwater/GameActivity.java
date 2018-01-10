
package de.tu_ilmenau.saltwater;

import android.app.NativeActivity;

import android.os.Bundle;

public class GameActivity extends NativeActivity
{
    static GameActivity s_Instance;

    public static GameActivity GetInstance()
    {
        return s_Instance;
    }

    @Override
    public void onStart()
    {
        super.onStart();

        System.out.println("onStart");
    }

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        s_Instance = this;
    }

    @Override
    public void onResume()
    {
        super.onResume();
    }

    @Override
    public void onPause ()
    {
        super.onPause();
    }

    @Override
    public void onStop()
    {
        super.onStop();
    }

    public String GetHello()
    {
        return ("Hello CPP engine");
    }
}
