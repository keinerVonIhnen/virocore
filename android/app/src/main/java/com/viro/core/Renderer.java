/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */
/*
 * This class is a convenience wrapper around:
 *
 * Cpp JNI wrapper      : VRORenderer_JNI.cpp
 * Cpp Object           : VROSceneRendererGVR.cpp
 */
package com.viro.core;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.view.Surface;

import com.google.ar.core.Session;
import com.viro.core.internal.PlatformUtil;

/**
 * @hide
 */
public class Renderer {

    protected long mNativeRef;

    /* ----------     Scene view only methods    ---------- */

    public Renderer(ClassLoader appClassLoader, Context context, ViroViewScene view, AssetManager assets,
                    PlatformUtil platformUtil) {
        mNativeRef = nativeCreateRendererSceneView(appClassLoader, context, view, assets, platformUtil);
    }

    /* ----------     GVR only methods    ---------- */

    public Renderer(ClassLoader appClassLoader, Context context,
                    AssetManager assets, PlatformUtil platformUtil, long nativeGvrContext) {
        mNativeRef = nativeCreateRendererGVR(appClassLoader, context, assets, platformUtil, nativeGvrContext);
    }

    public void drawFrame() {
        nativeDrawFrame(mNativeRef);
    }
    public void setVRModeEnabled(boolean enabled) { nativeSetVRModeEnabled(mNativeRef, enabled); }

    /* ----------     OVR only methods    ---------- */
    public Renderer(ClassLoader appClassLoader, Context context,
                    ViroViewOVR view, Activity activity, AssetManager assets, PlatformUtil platformUtil) {
        mNativeRef = nativeCreateRendererOVR(appClassLoader, context, view, activity, assets, platformUtil);
    }

    public void onSurfaceDestroyed(Surface surface) { nativeOnSurfaceDestroyed(mNativeRef); }

    public void recenterTracking() { nativeRecenterTracking(mNativeRef); }

    /* ----------     ARCore only methods    ---------- */

    public Renderer(ClassLoader appClassLoader, Context context,
                    ViroViewARCore view, Session session, AssetManager assets, PlatformUtil platformUtil) {
        mNativeRef = nativeCreateRendererARCore(appClassLoader, context, view, session, assets, platformUtil);
    }

    public void performARHitTestWithRay(float[] ray, ARHitTestListener callback) {
        nativePerformARHitTestWithRay(mNativeRef, ray, callback);
    }

    public void performARHitTestWithPosition(float[] position, ARHitTestListener callback) {
        nativePerformARHitTestWithPosition(mNativeRef, position, callback);
    }

    public void performARHitTestWithPoint(float x, float y, ARHitTestListener callback) {
        nativePerformARHitTestWithPoint(mNativeRef, x, y, callback);
    }

    /* ----------     Common lifecycle methods    ---------- */

    public void destroy() { nativeDestroyRenderer(mNativeRef); }
    public void initalizeGl() { nativeInitializeGl(mNativeRef); }

    public void onStart() { nativeOnStart(mNativeRef); }
    public void onPause() {
        nativeOnPause(mNativeRef);
    }
    public void onResume() {
        nativeOnResume(mNativeRef);
    }
    public void onStop() { nativeOnStop(mNativeRef); }
    public void onSurfaceCreated(Surface surface) { nativeOnSurfaceCreated(surface, mNativeRef); }
    public void onSurfaceChanged(Surface surface, int width, int height) { nativeOnSurfaceChanged(surface, width, height, mNativeRef); }

    /* ----------     Common other methods   ---------- */

    public void onKeyEvent(int keyCode, int action) {
        nativeOnKeyEvent(mNativeRef, keyCode, action);
    }
    public void onTouchEvent(int onTouchAction, float touchPosX, float touchPosY) {
        nativeOnTouchEvent(mNativeRef, onTouchAction, touchPosX, touchPosY);
    }

    public void onPinchEvent(int pinchState, float scaleFactor, float viewportX, float viewportY) {
        nativeOnPinchEvent(mNativeRef, pinchState, scaleFactor, viewportX, viewportY);
    }

    public void onRotateEvent(int rotateState, float rotateRadians, float viewportX, float viewportY) {
        nativeOnRotateEvent(mNativeRef, rotateState, rotateRadians, viewportX, viewportY);
    }

    public void setSceneController(long nativeSceneControllerRef) {
        nativeSetSceneController(mNativeRef, nativeSceneControllerRef);
    }
    public void setSceneController(long nativeSceneControllerRef, float duration) {
        nativeSetSceneControllerWithAnimation(mNativeRef, nativeSceneControllerRef, duration);
    };
    public void setPointOfView(Node node) {
        if (node != null) {
            nativeSetPointOfView(mNativeRef, node.mNativeRef);
        }
        else {
            nativeSetPointOfView(mNativeRef, 0);
        }
    }
    public String getHeadset() {
        return nativeGetHeadset(mNativeRef);
    }
    public String getController() {
        return nativeGetController(mNativeRef);
    }
    public void setSuspended(boolean suspend) {
       nativeSetSuspended(mNativeRef, suspend);
    }
    public void setDebugHUDEnabled(boolean enabled) { nativeSetDebugHUDEnabled(mNativeRef, enabled); }

    public void addFrameListener(FrameListener frameListener) {
        nativeAddFrameListener(mNativeRef, frameListener.mNativeRef);
    }

    public void removeFrameListener(FrameListener frameListener) {
        nativeRemoveFrameListener(mNativeRef, frameListener.mNativeRef);
    }

    /* ----------     Native methods    ---------- */

    private native long nativeCreateRendererGVR(ClassLoader appClassLoader, Context context,
                                                AssetManager assets, PlatformUtil platformUtil, long nativeGvrContext);
    private native long nativeCreateRendererOVR(ClassLoader appClassLoader, Context context,
                                                ViroViewOVR view, Activity activity, AssetManager assets, PlatformUtil platformUtil);
    private native long nativeCreateRendererARCore(ClassLoader appClassLoader, Context context,
                                                   ViroViewARCore view, Session session, AssetManager assets, PlatformUtil platformUtil);
    private native long nativeCreateRendererSceneView(ClassLoader appClassLoader, Context context,
                                                       ViroViewScene view, AssetManager assets, PlatformUtil platformUtil);
    private native void nativeDestroyRenderer(long nativeRenderer);
    private native void nativeInitializeGl(long nativeRenderer);
    private native void nativeSetVRModeEnabled(long nativeRenderer, boolean enabled);
    private native long nativeDrawFrame(long nativeRenderer);
    private native void nativeOnStart(long nativeRenderer);
    private native void nativeOnKeyEvent(long nativeRenderer, int keyCode, int action);
    private native void nativeOnTouchEvent(long nativeRenderer, int onTouchAction, float touchPosX, float touchPosY);
    private native void nativeOnPinchEvent(long nativeRenderer, int pinchState, float scaleFactor, float viewportX, float viewportY);
    private native void nativeOnRotateEvent(long nativeRenderer, int rotateState, float rotateRadians, float viewportX, float viewportY);
    private native void nativeOnPause(long nativeRenderer);
    private native void nativeOnResume(long nativeRenderer);
    private native void nativeOnStop(long nativeRenderer);
    private native void nativeOnSurfaceCreated(Surface surface, long nativeRenderer);
    private native void nativeOnSurfaceChanged(Surface surface, int width, int height, long nativeRenderer);
    private native void nativeOnSurfaceDestroyed(long nativeRenderer);
    private native void nativeSetSceneController(long nativeRenderer, long nativeScene);
    private native void nativeSetSceneControllerWithAnimation(long nativeRenderer, long nativeScene, float duration);
    private native void nativeSetPointOfView(long nativeRenderer, long nodeRef);
    private native String nativeGetHeadset(long nativeRenderer);
    private native String nativeGetController(long nativeRenderer);
    private native void nativeSetDebugHUDEnabled(long nativeRenderer, boolean enabled);
    private native void nativeSetSuspended(long nativeRenderer, boolean suspendRenderer);
    private native void nativeRecenterTracking(long nativeRenderer);
    private native void nativePerformARHitTestWithRay(long nativeRenderer, float[] ray, ARHitTestListener callback);
    private native void nativePerformARHitTestWithPosition(long nativeRenderer, float[] position, ARHitTestListener callback);
    private native void nativePerformARHitTestWithPoint(long nativeRenderer, float x, float y, ARHitTestListener callback);

    private native void nativeAddFrameListener(long nativeRenderer, long portalTraversalListener);
    private native void nativeRemoveFrameListener(long nativeRenderer, long portalTraversalListener);

}