package com.wheel;
import java.io.*;
import java.util.*;
import java.text.*;
import android.os.*;
import android.graphics.*;
import android.opengl.*;
import android.content.*;
import android.util.Log;
import android.os.PowerManager;
import android.app.NativeActivity;
import android.view.View;
import android.view.WindowManager;

public class WheelActivity extends NativeActivity
{
	PowerManager.WakeLock wakeLock;

	Runnable setLowProfile = new Runnable()
	{
		public void run()
		{
			try { getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LOW_PROFILE); }
			catch(Exception e) { Log.e("wheel", e.getMessage()); }
		}
	};

	void moveToBack() { moveTaskToBack(true); }
	void fullscreen() { try { runOnUiThread(setLowProfile); } catch(Exception e) { Log.e("wheel", e.getMessage()); } }
	int screenOrientation() { return ((WindowManager)getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay().getRotation(); }
}
