// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.net.impl;

import static org.chromium.net.impl.HttpEngineNativeProvider.EXT_API_LEVEL;
import static org.chromium.net.impl.HttpEngineNativeProvider.EXT_VERSION;

import androidx.annotation.RequiresExtension;

import java.nio.ByteBuffer;

@RequiresExtension(extension = EXT_API_LEVEL, version = EXT_VERSION)
class AndroidUrlRequestWrapper extends org.chromium.net.ExperimentalUrlRequest {
    private final android.net.http.UrlRequest mBackend;

    AndroidUrlRequestWrapper(android.net.http.UrlRequest backend) {
        this.mBackend = backend;
    }

    @Override
    public void start() {
        mBackend.start();
    }

    @Override
    public void followRedirect() {
        mBackend.followRedirect();
    }

    @Override
    public void read(ByteBuffer buffer) {
        mBackend.read(buffer);
    }

    @Override
    public void cancel() {
        mBackend.cancel();
    }

    @Override
    public boolean isDone() {
        return mBackend.isDone();
    }

    @Override
    public void getStatus(StatusListener listener) {
        mBackend.getStatus(new AndroidUrlRequestStatusListenerWrapper(listener));
    }

    /**
     * Creates an {@link AndroidUrlRequestWrapper} that is recorded on the callback.
     *
     * @param backend the http UrlRequest
     * @param callback the stream's callback
     * @return the wrapped request
     */
    static AndroidUrlRequestWrapper createAndAddToCallback(
            android.net.http.UrlRequest backend, AndroidUrlRequestCallbackWrapper callback) {
        AndroidUrlRequestWrapper wrappedRequest = new AndroidUrlRequestWrapper(backend);
        callback.setRequest(wrappedRequest);
        return wrappedRequest;
    }
}
