// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.feed.v2;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.annotations.NativeMethods;
import org.chromium.chrome.browser.ChromeActivity;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.browser.tab.TabLaunchType;
import org.chromium.chrome.browser.tabmodel.TabModelSelector;
import org.chromium.components.feed.proto.FeedUiProto.StreamUpdate;
import org.chromium.content_public.browser.LoadUrlParams;
import org.chromium.ui.base.PageTransition;

import java.util.ArrayList;
import java.util.List;

/**
 * Bridge class that lets Android code access native code for feed related functionalities.
 *
 * Created once for each StreamSurfaceMediator corresponding to each NTP/start surface.
 */
@JNINamespace("feed")
public class FeedStreamSurface implements FeedActionHandler {
    private final long mNativeFeedStreamSurface;
    private final ChromeActivity mActivity;

    /**
     * Creates a {@link FeedStreamSurface} for creating native side bridge to access native feed
     * client implementation.
     */
    public FeedStreamSurface(ChromeActivity activity) {
        mNativeFeedStreamSurface = FeedStreamSurfaceJni.get().init(FeedStreamSurface.this);
        mActivity = activity;
    }

    /**
     * Called when the stream update content is available. The content will get passed to UI
     */
    @CalledByNative
    void onStreamUpdated(byte[] data) {
        try {
            StreamUpdate streamUpdate = StreamUpdate.parseFrom(data);
        } catch (com.google.protobuf.InvalidProtocolBufferException e) {
            // Consume exception for now, ignoring unparsable events.
        }
    }

    @Override
    public void navigate(String url) {
        LoadUrlParams loadUrlParams = new LoadUrlParams(url);
        loadUrlParams.setTransitionType(PageTransition.AUTO_BOOKMARK);
        mActivity.getActivityTabProvider().get().loadUrl(loadUrlParams);
        FeedStreamSurfaceJni.get().navigationStarted(
                mNativeFeedStreamSurface, FeedStreamSurface.this, url, false /*inNewTab*/);
    }

    @Override
    public void navigateInNewTab(String url) {
        TabModelSelector tabModelSelector = mActivity.getTabModelSelector();
        Tab tab = mActivity.getActivityTabProvider().get();
        tabModelSelector.openNewTab(
                new LoadUrlParams(url), TabLaunchType.FROM_CHROME_UI, tab, tab.isIncognito());
        FeedStreamSurfaceJni.get().navigationStarted(
                mNativeFeedStreamSurface, FeedStreamSurface.this, url, true /*inNewTab*/);
    }

    @Override
    public void loadMore() {
        FeedStreamSurfaceJni.get().loadMore(mNativeFeedStreamSurface, FeedStreamSurface.this);
    }

    @Override
    public int requestDismissal() {
        // TODO(jianli): may need to pass parameters from UI.
        List<byte[]> serializedDataOperations = new ArrayList<byte[]>();
        // TODO(jianli): append data to serializedDataOperations.
        return FeedStreamSurfaceJni.get().executeEphemeralChange(
                mNativeFeedStreamSurface, FeedStreamSurface.this, serializedDataOperations);
    }

    @Override
    public void commitDismissal(int changeId) {
        FeedStreamSurfaceJni.get().commitEphemeralChange(
                mNativeFeedStreamSurface, FeedStreamSurface.this, changeId);
    }

    @Override
    public void discardDismissal(int changeId) {
        FeedStreamSurfaceJni.get().discardEphemeralChange(
                mNativeFeedStreamSurface, FeedStreamSurface.this, changeId);
    }

    /**
     * Handles uploading data for ThereAndBackAgain.
     */
    public void processThereAndBackAgain(byte[] data) {
        FeedStreamSurfaceJni.get().processThereAndBackAgain(
                mNativeFeedStreamSurface, FeedStreamSurface.this, data);
    }

    /**
     * Informs that the surface is opened. We can request the initial set of content now. Once
     * the content is available, onStreamUpdated will be called.
     */
    public void surfaceOpened() {
        FeedStreamSurfaceJni.get().surfaceOpened(mNativeFeedStreamSurface, FeedStreamSurface.this);
    }

    /**
     * Informs that the surface is closed. Any cleanup can be performed now.
     */
    public void surfaceClosed() {
        FeedStreamSurfaceJni.get().surfaceClosed(mNativeFeedStreamSurface, FeedStreamSurface.this);
    }

    @NativeMethods
    interface Natives {
        long init(FeedStreamSurface caller);
        void navigationStarted(long nativeFeedStreamSurface, FeedStreamSurface caller, String url,
                boolean inNewTab);
        void navigationDone(long nativeFeedStreamSurface, FeedStreamSurface caller, String url,
                boolean inNewTab);
        void loadMore(long nativeFeedStreamSurface, FeedStreamSurface caller);
        void processThereAndBackAgain(
                long nativeFeedStreamSurface, FeedStreamSurface caller, byte[] data);
        int executeEphemeralChange(long nativeFeedStreamSurface, FeedStreamSurface caller,
                List<byte[]> serializedDataOperations);
        void commitEphemeralChange(
                long nativeFeedStreamSurface, FeedStreamSurface caller, int changeId);
        void discardEphemeralChange(
                long nativeFeedStreamSurface, FeedStreamSurface caller, int changeId);
        void surfaceOpened(long nativeFeedStreamSurface, FeedStreamSurface caller);
        void surfaceClosed(long nativeFeedStreamSurface, FeedStreamSurface caller);
    }
}
