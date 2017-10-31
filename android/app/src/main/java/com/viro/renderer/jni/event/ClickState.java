package com.viro.renderer.jni.event;

import java.util.HashMap;
import java.util.Map;

/**
 * Indicates the status of a click event, for use with the {@link ClickListener}.
 */
public enum ClickState {
    /**
     * The button has gone down.
     */
    CLICK_DOWN(1),
    /**
     * The button has gone up.
     */
    CLICK_UP(2),
    /**
     * Indicates a fast down/up (click) has completed.
     */
    CLICKED(3);

    private final int mTypeId;
    ClickState(int id){
        mTypeId = id;
    }

    private static Map<Integer, ClickState> map = new HashMap<Integer, ClickState>();
    static {
        for (ClickState status : ClickState.values()) {
            map.put(status.mTypeId, status);
        }
    }
    public static ClickState valueOf(int id) {
        return map.get(id);
    }
    public int getTypeId() { return mTypeId; }
}