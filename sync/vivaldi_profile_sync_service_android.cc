// Copyright (c) 2019 Vivaldi Technologies AS. All rights reserved

#include "sync/vivaldi_profile_sync_service_android.h"

#include "app/vivaldi_apptools.h"
#include "base/android/jni_string.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/android/chrome_jni_headers/VivaldiProfileSyncService_jni.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "components/sync/driver/sync_token_status.h"
#include "sync/vivaldi_profile_sync_service.h"
#include "sync/vivaldi_profile_sync_service_factory.h"
#include "sync/vivaldi_sync_ui_helper.h"

static jlong JNI_VivaldiProfileSyncService_Init(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj) {
  VivaldiProfileSyncServiceAndroid* vivaldi_account_manager_android =
      new VivaldiProfileSyncServiceAndroid(env, obj);
  if (!vivaldi_account_manager_android->Init()) {
    delete vivaldi_account_manager_android;
    return 0;
  }
  return reinterpret_cast<intptr_t>(vivaldi_account_manager_android);
}

VivaldiProfileSyncServiceAndroid::VivaldiProfileSyncServiceAndroid(JNIEnv* env,
                                                                   jobject obj)
    : weak_java_ref_(env, obj) {
  profile_ = ProfileManager::GetActiveUserProfile();
  DCHECK(profile_);
  sync_service_ =
      vivaldi::VivaldiProfileSyncServiceFactory::GetForProfileVivaldi(profile_);

  SendCycleData();
}

VivaldiProfileSyncServiceAndroid::~VivaldiProfileSyncServiceAndroid() {
  sync_service_->RemoveObserver(this);
}

bool VivaldiProfileSyncServiceAndroid::Init() {
  if (sync_service_) {
    sync_service_->AddObserver(this);
    return true;
  }
  return false;
}

jboolean VivaldiProfileSyncServiceAndroid::SetEncryptionPassword(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj,
    const base::android::JavaParamRef<jstring>& password) {
  return sync_service_->ui_helper()->SetEncryptionPassword(
      base::android::ConvertJavaStringToUTF8(env, password));
}

void VivaldiProfileSyncServiceAndroid::ClearServerData(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj) {
  if (vivaldi::IsVivaldiRunning())
    sync_service_->ClearSyncData();
}

void VivaldiProfileSyncServiceAndroid::StopAndClear(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj) {
  sync_service_->StopAndClear();
}

jboolean VivaldiProfileSyncServiceAndroid::HasServerError(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj) {
  return sync_service_->GetSyncTokenStatusForDebugging().connection_status ==
         syncer::CONNECTION_SERVER_ERROR;
}

jboolean VivaldiProfileSyncServiceAndroid::IsSetupInProgress(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj) {
  return sync_service_->IsSetupInProgress();
}

base::android::ScopedJavaLocalRef<jstring>
VivaldiProfileSyncServiceAndroid::GetBackupEncryptionToken(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj) {
  return base::android::ConvertUTF8ToJavaString(
      env, sync_service_->ui_helper()->GetBackupEncryptionToken());
}

jboolean VivaldiProfileSyncServiceAndroid::RestoreEncryptionToken(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& obj,
    const base::android::JavaParamRef<jstring>& token) {
  return sync_service_->ui_helper()->RestoreEncryptionToken(
      base::android::ConvertJavaStringToUTF8(env, token));
}

void VivaldiProfileSyncServiceAndroid::SendCycleData() {
  JNIEnv* env = base::android::AttachCurrentThread();
  base::android::ScopedJavaLocalRef<jobject> obj = weak_java_ref_.get(env);

  vivaldi::VivaldiSyncUIHelper::CycleData cycle_data =
      sync_service_->ui_helper()->GetCycleData();

  Java_VivaldiProfileSyncService_onCycleData(
      env, obj, cycle_data.download_updates_status, cycle_data.commit_status,
      cycle_data.cycle_start_time.ToJavaTime(),
      cycle_data.next_retry_time.ToJavaTime());
}

void VivaldiProfileSyncServiceAndroid::OnSyncCycleCompleted(
    syncer::SyncService* sync) {
  SendCycleData();
}