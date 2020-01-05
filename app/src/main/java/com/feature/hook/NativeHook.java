package com.feature.hook;

/**
 * @anchor: handy
 * @date: 2019-11-24
 * @description:
 */
public class NativeHook {

    static {
        System.loadLibrary("native-write");
        System.loadLibrary("native-hook");
        System.loadLibrary("hook-simple");
    }

    public native void writeText(String filePath, String text);

    public native void hookWrite(String soPath);

    public native void hookSimple(String soName);

}
