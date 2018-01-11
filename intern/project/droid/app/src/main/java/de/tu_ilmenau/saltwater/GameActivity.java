
package de.tu_ilmenau.saltwater;

import android.os.Bundle;

public class GameActivity
{
    static GameActivity s_Instance;

    public static GameActivity GetInstance()
    {
        return s_Instance;
    }

    public GameActivity()
    {
        int a = 42;

        s_Instance = this;
    }

    public int GetHello()
    {
        return 42;
    }
}
