// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// #import {TestBrowserProxy} from 'chrome://test/test_browser_proxy.m.js';

/** @implements {settings.HatsBrowserProxy} */
/* #export */ class TestHatsBrowserProxy extends TestBrowserProxy {
  constructor() {
    super([
      'tryShowSurvey',
    ]);
  }

  /** @override*/
  tryShowSurvey() {
    this.methodCalled('tryShowSurvey');
  }
}
