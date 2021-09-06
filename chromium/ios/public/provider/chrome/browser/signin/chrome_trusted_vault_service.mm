// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/public/provider/chrome/browser/signin/chrome_trusted_vault_service.h"

#import "base/logging.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace ios {

ChromeTrustedVaultService::ChromeTrustedVaultService() {}

ChromeTrustedVaultService::~ChromeTrustedVaultService() {}

void ChromeTrustedVaultService::FetchKeys(
    ChromeIdentity* chrome_identity,
    base::OnceCallback<void(const TrustedVaultSharedKeyList&)> callback) {
  NOTREACHED();
}

}  // namespace ios
