// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// clang-format off
// #import 'chrome://settings/lazy_load.js';
// #import {TestBrowserProxy} from 'chrome://test/test_browser_proxy.m.js';
// #import {AccountManagerBrowserProxyImpl, ProfileInfoBrowserProxyImpl, pageVisibility, Router, SyncBrowserProxyImpl} from 'chrome://settings/settings.js';
// #import {flush} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
// #import {TestSyncBrowserProxy} from 'chrome://test/settings/test_sync_browser_proxy.m.js';
// #import {TestProfileInfoBrowserProxy} from 'chrome://test/settings/test_profile_info_browser_proxy.m.js';
// #import {simulateSyncStatus} from 'chrome://test/settings/sync_test_util.m.js';
// clang-format on

cr.define('settings_people_page', function() {
  /** @implements {settings.AccountManagerBrowserProxy} */
  class TestAccountManagerBrowserProxy extends TestBrowserProxy {
    constructor() {
      super([
        'getAccounts',
        'addAccount',
        'reauthenticateAccount',
        'removeAccount',
        'showWelcomeDialogIfRequired',
      ]);
    }

    /** @override */
    getAccounts() {
      this.methodCalled('getAccounts');
      return Promise.resolve([{
        id: '123',
        accountType: 1,
        isDeviceAccount: false,
        isSignedIn: true,
        unmigrated: false,
        fullName: 'Primary Account',
        email: 'user@gmail.com',
        pic: 'data:image/png;base64,primaryAccountPicData',
      }]);
    }

    /** @override */
    addAccount() {
      this.methodCalled('addAccount');
    }

    /** @override */
    reauthenticateAccount(account_email) {
      this.methodCalled('reauthenticateAccount', account_email);
    }

    /** @override */
    removeAccount(account) {
      this.methodCalled('removeAccount', account);
    }

    /** @override */
    showWelcomeDialogIfRequired() {
      this.methodCalled('showWelcomeDialogIfRequired');
    }
  }

  /** @type {?settings.AccountManagerBrowserProxy} */
  let accountManagerBrowserProxy = null;

  // Preferences should exist for embedded 'personalization_options.html'.
  // We don't perform tests on them.
  const DEFAULT_PREFS = {
    profile: {password_manager_leak_detection: {value: true}},
    signin: {
      allowed_on_next_startup:
          {type: chrome.settingsPrivate.PrefType.BOOLEAN, value: true}
    },
    safebrowsing:
        {enabled: {value: true}, scout_reporting_enabled: {value: true}},
  };

  /** @type {?SettingsPeoplePageElement} */
  let peoplePage = null;

  /** @type {?settings.ProfileInfoBrowserProxy} */
  let profileInfoBrowserProxy = null;

  /** @type {?settings.SyncBrowserProxy} */
  let syncBrowserProxy = null;

  suite('Chrome OS', function() {
    suiteSetup(function() {
      loadTimeData.overrideValues({
        // Simulate SplitSettings (OS settings in their own surface).
        showOSSettings: false,
        // Simulate ChromeOSAccountManager (Google Accounts support).
        isAccountManagerEnabled: true,
      });
    });

    setup(async function() {
      syncBrowserProxy = new TestSyncBrowserProxy();
      settings.SyncBrowserProxyImpl.instance_ = syncBrowserProxy;

      profileInfoBrowserProxy = new TestProfileInfoBrowserProxy();
      settings.ProfileInfoBrowserProxyImpl.instance_ = profileInfoBrowserProxy;

      accountManagerBrowserProxy = new TestAccountManagerBrowserProxy();
      settings.AccountManagerBrowserProxyImpl.instance_ =
          accountManagerBrowserProxy;

      PolymerTest.clearBody();
      peoplePage = document.createElement('settings-people-page');
      peoplePage.prefs = DEFAULT_PREFS;
      peoplePage.pageVisibility = settings.pageVisibility;
      document.body.appendChild(peoplePage);

      await accountManagerBrowserProxy.whenCalled('getAccounts');
      await syncBrowserProxy.whenCalled('getSyncStatus');
      Polymer.dom.flush();
    });

    teardown(function() {
      peoplePage.remove();
    });

    test('GAIA name and picture', async () => {
      chai.assert.include(
          peoplePage.$$('#profile-icon').style.backgroundImage,
          'data:image/png;base64,primaryAccountPicData');
      assertEquals(
          'Primary Account', peoplePage.$$('#profile-name').textContent.trim());
    });

    test('profile row is actionable', () => {
      // Simulate a signed-in user.
      sync_test_util.simulateSyncStatus({
        signedIn: true,
      });

      // Profile row opens account manager, so the row is actionable.
      const profileIcon = peoplePage.$$('#profile-icon');
      assertTrue(!!profileIcon);
      assertTrue(profileIcon.hasAttribute('actionable'));
      const profileRow = peoplePage.$$('#profile-row');
      assertTrue(!!profileRow);
      assertTrue(profileRow.hasAttribute('actionable'));
      const subpageArrow = peoplePage.$$('#profile-subpage-arrow');
      assertTrue(!!subpageArrow);
      assertFalse(subpageArrow.hidden);
    });
  });

  suite('Chrome OS with account manager disabled', function() {
    suiteSetup(function() {
      loadTimeData.overrideValues({
        // Simulate SplitSettings (OS settings in their own surface).
        showOSSettings: false,
        // Disable ChromeOSAccountManager (Google Accounts support).
        isAccountManagerEnabled: false,
      });
    });

    setup(async function() {
      syncBrowserProxy = new TestSyncBrowserProxy();
      settings.SyncBrowserProxyImpl.instance_ = syncBrowserProxy;

      profileInfoBrowserProxy = new TestProfileInfoBrowserProxy();
      settings.ProfileInfoBrowserProxyImpl.instance_ = profileInfoBrowserProxy;

      PolymerTest.clearBody();
      peoplePage = document.createElement('settings-people-page');
      peoplePage.prefs = DEFAULT_PREFS;
      peoplePage.pageVisibility = settings.pageVisibility;
      document.body.appendChild(peoplePage);

      await syncBrowserProxy.whenCalled('getSyncStatus');
      Polymer.dom.flush();
    });

    teardown(function() {
      peoplePage.remove();
    });

    test('profile row is not actionable', () => {
      // Simulate a signed-in user.
      sync_test_util.simulateSyncStatus({
        signedIn: true,
      });

      // Account manager isn't available, so the row isn't actionable.
      const profileIcon = peoplePage.$$('#profile-icon');
      assertTrue(!!profileIcon);
      assertFalse(profileIcon.hasAttribute('actionable'));
      const profileRow = peoplePage.$$('#profile-row');
      assertTrue(!!profileRow);
      assertFalse(profileRow.hasAttribute('actionable'));
      const subpageArrow = peoplePage.$$('#profile-subpage-arrow');
      assertTrue(!!subpageArrow);
      assertTrue(subpageArrow.hidden);

      // Clicking on profile icon doesn't navigate to a new route.
      const oldRoute = settings.Router.getInstance().getCurrentRoute();
      profileIcon.click();
      assertEquals(oldRoute, settings.Router.getInstance().getCurrentRoute());
    });
  });

  suite('Chrome OS with SplitSyncConsent', function() {
    suiteSetup(function() {
      loadTimeData.overrideValues({
        splitSettingsSyncEnabled: true,
        splitSyncConsent: true,
      });
    });

    setup(async function() {
      syncBrowserProxy = new TestSyncBrowserProxy();
      settings.SyncBrowserProxyImpl.instance_ = syncBrowserProxy;

      profileInfoBrowserProxy = new TestProfileInfoBrowserProxy();
      settings.ProfileInfoBrowserProxyImpl.instance_ = profileInfoBrowserProxy;

      PolymerTest.clearBody();
      peoplePage = document.createElement('settings-people-page');
      peoplePage.prefs = DEFAULT_PREFS;
      peoplePage.pageVisibility = settings.pageVisibility;
      document.body.appendChild(peoplePage);

      await syncBrowserProxy.whenCalled('getSyncStatus');
      Polymer.dom.flush();
    });

    teardown(function() {
      peoplePage.remove();
    });

    test('Sync account control is shown', () => {
      sync_test_util.simulateSyncStatus({
        syncSystemEnabled: true,
      });

      // Account control is visible.
      const accountControl = peoplePage.$$('settings-sync-account-control');
      assertNotEquals('none', window.getComputedStyle(accountControl).display);

      // Profile row items are not available.
      assertFalse(!!peoplePage.$$('#profile-icon'));
      assertFalse(!!peoplePage.$$('#profile-row'));
    });
  });
  // #cr_define_end
});
