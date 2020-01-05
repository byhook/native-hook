package com.feature.hook;

import android.os.Bundle;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import java.io.File;

import dalvik.system.PathClassLoader;

public class MainActivity extends AppCompatActivity {

    private NativeHook mNativeHook = new NativeHook();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    public void onHookClick(View view) {
        PathClassLoader pathClassLoader = (PathClassLoader) getClassLoader();
        String nativeWritePath = pathClassLoader.findLibrary("native-write");
        mNativeHook.hookWrite(nativeWritePath);
    }

    public void onWriteClick(View view) {
        File textFile = new File(getExternalFilesDir(null), "open.txt");
        mNativeHook.writeText(textFile.getAbsolutePath(), "world");
    }

    public void onSimpleHookClick(View view) {
        mNativeHook.hookSimple("libnative-write.so");
    }
}
