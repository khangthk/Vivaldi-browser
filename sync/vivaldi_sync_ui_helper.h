// Copyright (c) 2019 Vivaldi Technologies AS. All rights reserved

#ifndef SYNC_VIVALDI_SYNC_UI_HELPER_H_
#define SYNC_VIVALDI_SYNC_UI_HELPER_H_

#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/time/time.h"
#include "chrome/browser/profiles/profile.h"
#include "components/sync/driver/sync_service_observer.h"

namespace vivaldi {

class VivaldiProfileSyncService;

class VivaldiSyncUIHelper : public syncer::SyncServiceObserver {
 public:
  using ResultCallback = base::OnceCallback<void(bool)>;

  enum CycleStatus {
    NOT_SYNCED = 0,
    SUCCESS,
    IN_PROGRESS,
    AUTH_ERROR,
    SERVER_ERROR,
    NETWORK_ERROR,
    CLIENT_ERROR,
    CONFLICT,
    THROTTLED,
    OTHER_ERROR
  };

  struct CycleData {
    CycleStatus download_updates_status;
    CycleStatus commit_status;
    base::Time cycle_start_time;
    base::Time next_retry_time;
  };

  VivaldiSyncUIHelper(Profile* profile,
                      VivaldiProfileSyncService* sync_manager);
  ~VivaldiSyncUIHelper() override;

  bool SetEncryptionPassword(const std::string& password);

  std::string GetBackupEncryptionToken();
  bool RestoreEncryptionToken(const base::StringPiece& token);

  CycleData GetCycleData();

  // syncer::SyncServiceObserver implementation.
  void OnStateChanged(syncer::SyncService* sync) override;
  void OnSyncShutdown(syncer::SyncService* sync) override;

 private:
  Profile* profile_;
  VivaldiProfileSyncService* sync_manager_;

  bool tried_decrypt_ = false;
};
}  // namespace vivaldi

#endif  // SYNC_VIVALDI_SYNC_UI_HELPER_H_