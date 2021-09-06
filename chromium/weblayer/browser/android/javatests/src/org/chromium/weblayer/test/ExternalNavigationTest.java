// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.weblayer.test;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.test.filters.SmallTest;
import android.support.v4.app.Fragment;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.chromium.base.test.BaseJUnit4ClassRunner;
import org.chromium.base.test.util.CallbackHelper;
import org.chromium.content_public.browser.test.util.TestThreadUtils;
import org.chromium.weblayer.Browser;
import org.chromium.weblayer.Tab;
import org.chromium.weblayer.shell.InstrumentationActivity;

/**
 * Tests handling of external intents.
 */
@RunWith(BaseJUnit4ClassRunner.class)
public class ExternalNavigationTest {
    @Rule
    public InstrumentationActivityTestRule mActivityTestRule =
            new InstrumentationActivityTestRule();

    private static final String ABOUT_BLANK_URL = "about:blank";
    private static final String INTENT_TO_CHROME_DATA_CONTENT =
            "play.google.com/store/apps/details?id=com.facebook.katana/";
    private static final String INTENT_TO_CHROME_SCHEME = "https";
    private static final String INTENT_TO_CHROME_DATA_STRING =
            INTENT_TO_CHROME_SCHEME + "://" + INTENT_TO_CHROME_DATA_CONTENT;
    private static final String INTENT_TO_CHROME_ACTION = "android.intent.action.VIEW";
    private static final String INTENT_TO_CHROME_PACKAGE = "com.android.chrome";

    // An intent that opens Chrome to view a specified URL. Note that the "end" is left off to allow
    // appending extras when constructing URLs.
    private static final String INTENT_TO_CHROME = "intent://" + INTENT_TO_CHROME_DATA_CONTENT
            + "#Intent;scheme=" + INTENT_TO_CHROME_SCHEME + ";action=" + INTENT_TO_CHROME_ACTION
            + ";package=" + INTENT_TO_CHROME_PACKAGE + ";";
    private static final String INTENT_TO_CHROME_URL = INTENT_TO_CHROME + "end";

    // An intent URL that gets rejected as malformed.
    private static final String MALFORMED_INTENT_URL = "intent://garbage;end";

    // An intent that is properly formed but wishes to open an app that is not present on the
    // device. Note that the "end" is left off to allow appending extras when constructing URLs.
    private static final String NON_RESOLVABLE_INTENT =
            "intent://dummy.com/#Intent;scheme=https;action=android.intent.action.VIEW;package=com.missing.app;";

    private static final String LINK_WITH_INTENT_TO_CHROME_IN_SAME_TAB_FILE =
            "link_with_intent_to_chrome_in_same_tab.html";
    private static final String LINK_WITH_INTENT_TO_CHROME_IN_NEW_TAB_FILE =
            "link_with_intent_to_chrome_in_new_tab.html";
    private static final String PAGE_THAT_INTENTS_TO_CHROME_ON_LOAD_FILE =
            "page_that_intents_to_chrome_on_load.html";

    // The test server handles "echo" with a response containing "Echo" :).
    private final String mTestServerSiteUrl = mActivityTestRule.getTestServer().getURL("/echo");

    private final String mTestServerSiteFallbackUrlExtra =
            "S.browser_fallback_url=" + android.net.Uri.encode(mTestServerSiteUrl) + ";";
    private final String mIntentToChromeWithFallbackUrl =
            INTENT_TO_CHROME + mTestServerSiteFallbackUrlExtra + "end";
    private final String mNonResolvableIntentWithFallbackUrl =
            NON_RESOLVABLE_INTENT + mTestServerSiteFallbackUrlExtra + "end";

    private final String mRedirectToIntentToChromeURL =
            mActivityTestRule.getTestServer().getURL("/server-redirect?" + INTENT_TO_CHROME_URL);
    private final String mNonResolvableIntentWithFallbackUrlThatLaunchesIntent =
            NON_RESOLVABLE_INTENT + "S.browser_fallback_url="
            + android.net.Uri.encode(mRedirectToIntentToChromeURL) + ";end";

    private class IntentInterceptor implements InstrumentationActivity.IntentInterceptor {
        public Intent mLastIntent;
        private CallbackHelper mCallbackHelper = new CallbackHelper();

        @Override
        public void interceptIntent(
                Fragment fragment, Intent intent, int requestCode, Bundle options) {
            mLastIntent = intent;
            mCallbackHelper.notifyCalled();
        }

        public void waitForIntent() {
            try {
                mCallbackHelper.waitForFirst();
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }
    }

    /**
     * Verifies that for a navigation to a URI that WebLayer can handle internally, there
     * is no external intent triggered.
     */
    @Test
    @SmallTest
    public void testBrowserNavigation() throws Throwable {
        InstrumentationActivity activity = mActivityTestRule.launchShellWithUrl(ABOUT_BLANK_URL);
        IntentInterceptor intentInterceptor = new IntentInterceptor();
        activity.setIntentInterceptor(intentInterceptor);

        mActivityTestRule.navigateAndWait(mTestServerSiteUrl);

        Assert.assertNull(intentInterceptor.mLastIntent);
        Assert.assertEquals(mTestServerSiteUrl, mActivityTestRule.getCurrentDisplayUrl());
    }

    /**
     * Tests that a direct navigation to an external intent is blocked, resulting in a failed
     * browser navigation.
     */
    @Test
    @SmallTest
    public void testExternalIntentWithNoRedirectBlocked() throws Throwable {
        InstrumentationActivity activity = mActivityTestRule.launchShellWithUrl(ABOUT_BLANK_URL);
        IntentInterceptor intentInterceptor = new IntentInterceptor();
        activity.setIntentInterceptor(intentInterceptor);

        Tab tab = mActivityTestRule.getActivity().getTab();

        // Note that this navigation will not result in a paint.
        mActivityTestRule.navigateAndWaitForFailure(
                tab, INTENT_TO_CHROME_URL, /*waitForPaint=*/false);

        Assert.assertNull(intentInterceptor.mLastIntent);

        // The current URL should not have changed.
        Assert.assertEquals(ABOUT_BLANK_URL, mActivityTestRule.getCurrentDisplayUrl());
    }

    /**
     * Tests that a navigation that redirects to an external intent results in the external intent
     * being launched.
     */
    @Test
    @SmallTest
    public void testExternalIntentAfterRedirectLaunched() throws Throwable {
        InstrumentationActivity activity = mActivityTestRule.launchShellWithUrl(ABOUT_BLANK_URL);
        IntentInterceptor intentInterceptor = new IntentInterceptor();
        activity.setIntentInterceptor(intentInterceptor);

        Tab tab = mActivityTestRule.getActivity().getTab();
        TestThreadUtils.runOnUiThreadBlocking(() -> {
            tab.getNavigationController().navigate(Uri.parse(mRedirectToIntentToChromeURL));
        });

        intentInterceptor.waitForIntent();

        // The current URL should not have changed, and the intent should have been launched.
        Assert.assertEquals(ABOUT_BLANK_URL, mActivityTestRule.getCurrentDisplayUrl());
        Intent intent = intentInterceptor.mLastIntent;
        Assert.assertNotNull(intent);
        Assert.assertEquals(INTENT_TO_CHROME_PACKAGE, intent.getPackage());
        Assert.assertEquals(INTENT_TO_CHROME_ACTION, intent.getAction());
        Assert.assertEquals(INTENT_TO_CHROME_DATA_STRING, intent.getDataString());
    }

    /**
     * Tests that clicking on a link that goes to an external intent in the same tab results in the
     * external intent being launched.
     */
    @Test
    @SmallTest
    public void testExternalIntentInSameTabLaunchedOnLinkClick() throws Throwable {
        InstrumentationActivity activity = mActivityTestRule.launchShellWithUrl(ABOUT_BLANK_URL);
        IntentInterceptor intentInterceptor = new IntentInterceptor();
        activity.setIntentInterceptor(intentInterceptor);

        String url = mActivityTestRule.getTestDataURL(LINK_WITH_INTENT_TO_CHROME_IN_SAME_TAB_FILE);

        mActivityTestRule.navigateAndWait(url);

        mActivityTestRule.executeScriptSync(
                "document.onclick = function() {document.getElementById('link').click()}",
                true /* useSeparateIsolate */);
        EventUtils.simulateTouchCenterOfView(
                mActivityTestRule.getActivity().getWindow().getDecorView());

        intentInterceptor.waitForIntent();

        // The current URL should not have changed, and the intent should have been launched.
        Assert.assertEquals(url, mActivityTestRule.getCurrentDisplayUrl());
        Intent intent = intentInterceptor.mLastIntent;
        Assert.assertNotNull(intent);
        Assert.assertEquals(INTENT_TO_CHROME_PACKAGE, intent.getPackage());
        Assert.assertEquals(INTENT_TO_CHROME_ACTION, intent.getAction());
        Assert.assertEquals(INTENT_TO_CHROME_DATA_STRING, intent.getDataString());
    }

    /**
     * Tests that clicking on a link that goes to an external intent in a new tab results in
     * a new tab being opened whose URL is that of the intent and the intent being launched.
     */
    @Test
    @SmallTest
    public void testExternalIntentInNewTabLaunchedOnLinkClick() throws Throwable {
        InstrumentationActivity activity = mActivityTestRule.launchShellWithUrl(ABOUT_BLANK_URL);
        IntentInterceptor intentInterceptor = new IntentInterceptor();
        activity.setIntentInterceptor(intentInterceptor);

        String url = mActivityTestRule.getTestDataURL(LINK_WITH_INTENT_TO_CHROME_IN_NEW_TAB_FILE);

        mActivityTestRule.navigateAndWait(url);

        // Grab the existing tab before causing a new one to be opened.
        Tab tab = mActivityTestRule.getActivity().getTab();

        mActivityTestRule.executeScriptSync(
                "document.onclick = function() {document.getElementById('link').click()}",
                true /* useSeparateIsolate */);
        EventUtils.simulateTouchCenterOfView(
                mActivityTestRule.getActivity().getWindow().getDecorView());

        intentInterceptor.waitForIntent();

        // The current URL should not have changed in the existing tab, and the intent should have
        // been launched.
        Assert.assertEquals(url, mActivityTestRule.getLastCommittedUrlInTab(tab));
        Intent intent = intentInterceptor.mLastIntent;
        Assert.assertNotNull(intent);
        Assert.assertEquals(INTENT_TO_CHROME_PACKAGE, intent.getPackage());
        Assert.assertEquals(INTENT_TO_CHROME_ACTION, intent.getAction());
        Assert.assertEquals(INTENT_TO_CHROME_DATA_STRING, intent.getDataString());

        // A new tab should have been created whose URL is that of the intent.
        Browser browser = mActivityTestRule.getActivity().getBrowser();
        int numTabs =
                TestThreadUtils.runOnUiThreadBlocking(() -> { return browser.getTabs().size(); });
        Assert.assertEquals(2, numTabs);
        Assert.assertEquals(INTENT_TO_CHROME_URL, mActivityTestRule.getCurrentDisplayUrl());
    }

    /**
     * Tests that a navigation that redirects to an external intent with a fallback URL results in
     * the external intent being launched.
     */
    @Test
    @SmallTest
    public void testExternalIntentWithFallbackUrlAfterRedirectLaunched() throws Throwable {
        InstrumentationActivity activity = mActivityTestRule.launchShellWithUrl(ABOUT_BLANK_URL);
        IntentInterceptor intentInterceptor = new IntentInterceptor();
        activity.setIntentInterceptor(intentInterceptor);

        String url = mActivityTestRule.getTestServer().getURL(
                "/server-redirect?" + mIntentToChromeWithFallbackUrl);

        Tab tab = mActivityTestRule.getActivity().getTab();
        TestThreadUtils.runOnUiThreadBlocking(
                () -> { tab.getNavigationController().navigate(Uri.parse(url)); });

        intentInterceptor.waitForIntent();

        // The current URL should not have changed, and the intent should have been launched.
        Assert.assertEquals(ABOUT_BLANK_URL, mActivityTestRule.getCurrentDisplayUrl());
        Intent intent = intentInterceptor.mLastIntent;
        Assert.assertNotNull(intent);
        Assert.assertEquals(INTENT_TO_CHROME_PACKAGE, intent.getPackage());
        Assert.assertEquals(INTENT_TO_CHROME_ACTION, intent.getAction());
        Assert.assertEquals(INTENT_TO_CHROME_DATA_STRING, intent.getDataString());
    }

    /**
     * Tests that a navigation that redirects to an external intent that can't be handled results in
     * a failed navigation.
     */
    @Test
    @SmallTest
    public void testNonHandledExternalIntentAfterRedirectBlocked() throws Throwable {
        InstrumentationActivity activity = mActivityTestRule.launchShellWithUrl(ABOUT_BLANK_URL);
        IntentInterceptor intentInterceptor = new IntentInterceptor();
        activity.setIntentInterceptor(intentInterceptor);

        String url = mActivityTestRule.getTestServer().getURL(
                "/server-redirect?" + MALFORMED_INTENT_URL);

        Tab tab = mActivityTestRule.getActivity().getTab();

        // Note that this navigation will not result in a paint.
        NavigationWaiter waiter = new NavigationWaiter(
                MALFORMED_INTENT_URL, tab, /*expectFailure=*/true, /*waitForPaint=*/false);

        TestThreadUtils.runOnUiThreadBlocking(
                () -> { tab.getNavigationController().navigate(Uri.parse(url)); });
        waiter.waitForNavigation();

        Assert.assertNull(intentInterceptor.mLastIntent);

        // The current URL should not have changed.
        Assert.assertEquals(ABOUT_BLANK_URL, mActivityTestRule.getCurrentDisplayUrl());
    }

    /**
     * Tests that a navigation that redirects to an external intent that can't be handled but has a
     * fallback URL results in a navigation to the fallback URL.
     */
    @Test
    @SmallTest
    public void testNonHandledExternalIntentWithFallbackUrlAfterRedirectGoesToFallbackUrl()
            throws Throwable {
        InstrumentationActivity activity = mActivityTestRule.launchShellWithUrl(ABOUT_BLANK_URL);
        IntentInterceptor intentInterceptor = new IntentInterceptor();
        activity.setIntentInterceptor(intentInterceptor);

        String url = mActivityTestRule.getTestServer().getURL(
                "/server-redirect?" + mNonResolvableIntentWithFallbackUrl);

        Tab tab = mActivityTestRule.getActivity().getTab();

        NavigationWaiter waiter = new NavigationWaiter(
                mTestServerSiteUrl, tab, /*expectFailure=*/false, /*waitForPaint=*/true);

        TestThreadUtils.runOnUiThreadBlocking(
                () -> { tab.getNavigationController().navigate(Uri.parse(url)); });
        waiter.waitForNavigation();

        Assert.assertNull(intentInterceptor.mLastIntent);

        // The current URL should now be the fallback URL.
        Assert.assertEquals(mTestServerSiteUrl, mActivityTestRule.getCurrentDisplayUrl());
    }

    /**
     * Tests that a navigation that redirects to an external intent that can't be handled but has a
     * fallback URL that launches an intent that *can* be handled results in the launching of the
     * second intent.
     * |url| is a URL that redirects to an unhandleable intent but has a fallback URL that redirects
     * to a handleable intent.
     * Tests that a navigation to |url| launches the handleable intent.
     * TODO(crbug.com/1031465): Disallow such fallback intent launches by sharing Chrome's
     * RedirectHandler impl, at which point this should fail and be updated to verify that the
     * intent is blocked.
     */
    @Test
    @SmallTest
    public void
    testNonHandledExternalIntentWithFallbackUrlThatLaunchesIntentAfterRedirectLaunchesFallbackIntent()
            throws Throwable {
        InstrumentationActivity activity = mActivityTestRule.launchShellWithUrl(ABOUT_BLANK_URL);
        IntentInterceptor intentInterceptor = new IntentInterceptor();
        activity.setIntentInterceptor(intentInterceptor);

        String url = mActivityTestRule.getTestServer().getURL(
                "/server-redirect?" + mNonResolvableIntentWithFallbackUrlThatLaunchesIntent);

        Tab tab = mActivityTestRule.getActivity().getTab();

        TestThreadUtils.runOnUiThreadBlocking(
                () -> { tab.getNavigationController().navigate(Uri.parse(url)); });

        intentInterceptor.waitForIntent();

        // The current URL should not have changed, and the intent should have been launched.
        Assert.assertEquals(ABOUT_BLANK_URL, mActivityTestRule.getCurrentDisplayUrl());
        Intent intent = intentInterceptor.mLastIntent;
        Assert.assertNotNull(intent);
        Assert.assertEquals(INTENT_TO_CHROME_PACKAGE, intent.getPackage());
        Assert.assertEquals(INTENT_TO_CHROME_ACTION, intent.getAction());
        Assert.assertEquals(INTENT_TO_CHROME_DATA_STRING, intent.getDataString());
    }

    /**
     * Tests that going to a page that loads an intent that can be handled in onload() results in
     * the external intent being launched.
     * TODO(crbug.com/1031465): Disallow such intent launches by sharing Chrome's RedirectHandler
     * impl, at which point this should fail and be updated to verify that the intent is blocked.
     */
    @Test
    @SmallTest
    public void testExternalIntentLaunchedViaOnLoad() throws Throwable {
        InstrumentationActivity activity = mActivityTestRule.launchShellWithUrl(ABOUT_BLANK_URL);
        IntentInterceptor intentInterceptor = new IntentInterceptor();
        activity.setIntentInterceptor(intentInterceptor);

        String url = mActivityTestRule.getTestDataURL(PAGE_THAT_INTENTS_TO_CHROME_ON_LOAD_FILE);

        mActivityTestRule.navigateAndWait(url);

        intentInterceptor.waitForIntent();

        // The current URL should not have changed, and the intent should have been launched.
        Assert.assertEquals(url, mActivityTestRule.getCurrentDisplayUrl());
        Intent intent = intentInterceptor.mLastIntent;
        Assert.assertNotNull(intent);
        Assert.assertEquals(INTENT_TO_CHROME_PACKAGE, intent.getPackage());
        Assert.assertEquals(INTENT_TO_CHROME_ACTION, intent.getAction());
        Assert.assertEquals(INTENT_TO_CHROME_DATA_STRING, intent.getDataString());
    }
}
