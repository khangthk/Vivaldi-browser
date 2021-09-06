// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/cert_provisioning/cert_provisioning_worker.h"

#include "base/no_destructor.h"
#include "chrome/browser/chromeos/platform_keys/platform_keys_service.h"
#include "chrome/browser/chromeos/platform_keys/platform_keys_service_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "components/policy/core/common/cloud/cloud_policy_client.h"
#include "components/policy/core/common/cloud/device_management_service.h"
#include "content/public/browser/browser_context.h"

namespace em = enterprise_management;

namespace chromeos {
namespace cert_provisioning {

namespace {

const base::TimeDelta kMinumumTryAgainLaterDelay =
    base::TimeDelta::FromSeconds(10);

const net::BackoffEntry::Policy kBackoffPolicy{
    /*num_errors_to_ignore=*/0,
    /*initial_delay_ms=*/30 * 1000 /* (30 seconds) */,
    /*multiply_factor=*/2.0,
    /*jitter_factor=*/0.15,
    /*maximum_backoff_ms=*/12 * 60 * 60 * 1000 /* (12 hours) */,
    /*entry_lifetime_ms=*/-1,
    /*always_use_initial_delay=*/false};

std::string CertScopeToString(CertScope cert_scope) {
  switch (cert_scope) {
    case CertScope::kUser:
      return "google/chromeos/user";
    case CertScope::kDevice:
      return "google/chromeos/device";
  }
  NOTREACHED();
}

bool ConvertHashingAlgorithm(
    em::HashingAlgorithm input_algo,
    base::Optional<chromeos::platform_keys::HashAlgorithm>* output_algo) {
  switch (input_algo) {
    case em::HashingAlgorithm::SHA1:
      *output_algo =
          chromeos::platform_keys::HashAlgorithm::HASH_ALGORITHM_SHA1;
      return true;
    case em::HashingAlgorithm::SHA256:
      *output_algo =
          chromeos::platform_keys::HashAlgorithm::HASH_ALGORITHM_SHA256;
      return true;
    case em::HashingAlgorithm::HASHING_ALGORITHM_UNSPECIFIED:
      return false;
  }
}

// States are used in serialization and cannot be reordered. Therefore, their
// order should not be defined by their underlying values.
int GetStateOrderedIndex(CertProvisioningWorkerState state) {
  int res = 0;
  switch (state) {
    case CertProvisioningWorkerState::kInitState:
      res -= 1;
      FALLTHROUGH;
    case CertProvisioningWorkerState::kKeypairGenerated:
      res -= 1;
      FALLTHROUGH;
    case CertProvisioningWorkerState::kStartCsrResponseReceived:
      res -= 1;
      FALLTHROUGH;
    case CertProvisioningWorkerState::kVaChallengeFinished:
      res -= 1;
      FALLTHROUGH;
    case CertProvisioningWorkerState::kKeyRegistered:
      res -= 1;
      FALLTHROUGH;
    case CertProvisioningWorkerState::kSignCsrFinished:
      res -= 1;
      FALLTHROUGH;
    case CertProvisioningWorkerState::kFinishCsrResponseReceived:
      res -= 1;
      FALLTHROUGH;
    case CertProvisioningWorkerState::kDownloadCertResponseReceived:
      res -= 1;
      FALLTHROUGH;
    case CertProvisioningWorkerState::kSucceed:
    case CertProvisioningWorkerState::kFailed:
      res -= 1;
  }
  return res;
}

}  // namespace

// ============= CertProvisioningWorkerFactory =================================

CertProvisioningWorkerFactory* CertProvisioningWorkerFactory::test_factory_ =
    nullptr;

// static
CertProvisioningWorkerFactory* CertProvisioningWorkerFactory::Get() {
  if (UNLIKELY(test_factory_)) {
    return test_factory_;
  }

  static base::NoDestructor<CertProvisioningWorkerFactory> factory;
  return factory.get();
}

std::unique_ptr<CertProvisioningWorker> CertProvisioningWorkerFactory::Create(
    CertScope cert_scope,
    Profile* profile,
    PrefService* pref_service,
    const CertProfile& cert_profile,
    policy::CloudPolicyClient* cloud_policy_client,
    CertProvisioningWorkerCallback callback) {
  return std::make_unique<CertProvisioningWorkerImpl>(
      cert_scope, profile, pref_service, cert_profile, cloud_policy_client,
      std::move(callback));
}

// static
void CertProvisioningWorkerFactory::SetFactoryForTesting(
    CertProvisioningWorkerFactory* test_factory) {
  test_factory_ = test_factory;
}

// ============= CertProvisioningWorkerImpl ====================================

CertProvisioningWorkerImpl::CertProvisioningWorkerImpl(
    CertScope cert_scope,
    Profile* profile,
    PrefService* pref_service,
    const CertProfile& cert_profile,
    policy::CloudPolicyClient* cloud_policy_client,
    CertProvisioningWorkerCallback callback)
    : cert_scope_(cert_scope),
      profile_(profile),
      pref_service_(pref_service),
      cert_profile_(cert_profile),
      callback_(std::move(callback)),
      request_backoff_(&kBackoffPolicy),
      cloud_policy_client_(cloud_policy_client) {
  CHECK(profile);
  platform_keys_service_ =
      platform_keys::PlatformKeysServiceFactory::GetForBrowserContext(profile);
  CHECK(platform_keys_service_);

  CHECK(pref_service);
  CHECK(cloud_policy_client_);
}

CertProvisioningWorkerImpl::~CertProvisioningWorkerImpl() = default;

void CertProvisioningWorkerImpl::DoStep() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  CancelScheduledTasks();
  is_waiting_ = false;

  switch (state_) {
    case CertProvisioningWorkerState::kInitState:
      GenerateKey();
      return;
    case CertProvisioningWorkerState::kKeypairGenerated:
      StartCsr();
      return;
    case CertProvisioningWorkerState::kStartCsrResponseReceived:
      BuildVaChallengeResponse();
      return;
    case CertProvisioningWorkerState::kVaChallengeFinished:
      RegisterKey();
      return;
    case CertProvisioningWorkerState::kKeyRegistered:
      SignCsr();
      return;
    case CertProvisioningWorkerState::kSignCsrFinished:
      FinishCsr();
      return;
    case CertProvisioningWorkerState::kFinishCsrResponseReceived:
      DownloadCert();
      return;
    case CertProvisioningWorkerState::kDownloadCertResponseReceived:
      ImportCert();
      return;
    case CertProvisioningWorkerState::kSucceed:
    case CertProvisioningWorkerState::kFailed:
      DCHECK(false);
      return;
  }
  NOTREACHED() << " " << static_cast<uint>(state_);
}

void CertProvisioningWorkerImpl::UpdateState(
    CertProvisioningWorkerState new_state) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  DCHECK(GetStateOrderedIndex(state_) < GetStateOrderedIndex(new_state));

  state_ = new_state;

  if (IsFinished()) {
    std::move(callback_).Run(state_ == CertProvisioningWorkerState::kSucceed);
  }
}

void CertProvisioningWorkerImpl::GenerateKey() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  tpm_challenge_key_subtle_impl_ =
      attestation::TpmChallengeKeySubtleFactory::Create();
  tpm_challenge_key_subtle_impl_->StartPrepareKeyStep(
      GetVaKeyType(cert_scope_),
      GetVaKeyName(cert_scope_, cert_profile_.profile_id), profile_,
      GetVaKeyNameForSpkac(cert_scope_, cert_profile_.profile_id),
      base::BindOnce(&CertProvisioningWorkerImpl::OnGenerateKeyDone,
                     weak_factory_.GetWeakPtr()));
}

void CertProvisioningWorkerImpl::OnGenerateKeyDone(
    const attestation::TpmChallengeKeyResult& result) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!result.IsSuccess() || result.public_key.empty()) {
    LOG(ERROR) << "Failed to prepare key: " << result.GetErrorMessage();
    UpdateState(CertProvisioningWorkerState::kFailed);
    return;
  }

  public_key_ = result.public_key;
  UpdateState(CertProvisioningWorkerState::kKeypairGenerated);
  DoStep();
}

void CertProvisioningWorkerImpl::StartCsr() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  cloud_policy_client_->ClientCertProvisioningStartCsr(
      CertScopeToString(cert_scope_), cert_profile_.profile_id, public_key_,
      base::BindOnce(&CertProvisioningWorkerImpl::OnStartCsrDone,
                     weak_factory_.GetWeakPtr()));
}

void CertProvisioningWorkerImpl::OnStartCsrDone(
    policy::DeviceManagementStatus status,
    base::Optional<CertProvisioningResponseErrorType> error,
    base::Optional<int64_t> try_later,
    const std::string& invalidation_topic,
    const std::string& va_challenge,
    enterprise_management::HashingAlgorithm hashing_algorithm,
    const std::string& data_to_sign) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!ProcessResponseErrors(status, error, try_later)) {
    return;
  }

  if (!ConvertHashingAlgorithm(hashing_algorithm, &hashing_algorithm_)) {
    LOG(ERROR) << "Failed to parse hashing algorithm";
    UpdateState(CertProvisioningWorkerState::kFailed);
    return;
  }

  csr_ = data_to_sign;
  invalidation_topic_ = invalidation_topic;
  va_challenge_ = va_challenge;
  UpdateState(CertProvisioningWorkerState::kStartCsrResponseReceived);

  RegisterForInvalidationTopic(invalidation_topic);
  DoStep();
}

void CertProvisioningWorkerImpl::BuildVaChallengeResponse() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (va_challenge_.empty()) {
    UpdateState(CertProvisioningWorkerState::kVaChallengeFinished);
    DoStep();
    return;
  }

  tpm_challenge_key_subtle_impl_->StartSignChallengeStep(
      va_challenge_, /*include_signed_public_key=*/true,
      base::BindOnce(
          &CertProvisioningWorkerImpl::OnBuildVaChallengeResponseDone,
          weak_factory_.GetWeakPtr()));
}

void CertProvisioningWorkerImpl::OnBuildVaChallengeResponseDone(
    const attestation::TpmChallengeKeyResult& result) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!result.IsSuccess()) {
    LOG(ERROR) << "Failed to build challenge response: "
               << result.GetErrorMessage();
    UpdateState(CertProvisioningWorkerState::kFailed);
    return;
  }

  if (result.challenge_response.empty()) {
    LOG(ERROR) << "Challenge response is empty";
    UpdateState(CertProvisioningWorkerState::kFailed);
    return;
  }

  va_challenge_response_ = result.challenge_response;
  UpdateState(CertProvisioningWorkerState::kVaChallengeFinished);
  DoStep();
}

void CertProvisioningWorkerImpl::RegisterKey() {
  tpm_challenge_key_subtle_impl_->StartRegisterKeyStep(
      base::BindOnce(&CertProvisioningWorkerImpl::OnRegisterKeyDone,
                     weak_factory_.GetWeakPtr()));
}

void CertProvisioningWorkerImpl::OnRegisterKeyDone(
    const attestation::TpmChallengeKeyResult& result) {
  if (!result.IsSuccess()) {
    LOG(ERROR) << "Failed to register key: " << result.GetErrorMessage();
    UpdateState(CertProvisioningWorkerState::kFailed);
    return;
  }

  UpdateState(CertProvisioningWorkerState::kKeyRegistered);
  DoStep();
}

void CertProvisioningWorkerImpl::SignCsr() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!hashing_algorithm_.has_value()) {
    LOG(ERROR) << "Hashing algorithm is empty";
    UpdateState(CertProvisioningWorkerState::kFailed);
    return;
  }

  platform_keys_service_->SignRSAPKCS1Digest(
      GetPlatformKeysTokenId(cert_scope_), csr_, public_key_,
      hashing_algorithm_.value(),
      base::BindRepeating(&CertProvisioningWorkerImpl::OnSignCsrDone,
                          weak_factory_.GetWeakPtr()));
}

void CertProvisioningWorkerImpl::OnSignCsrDone(
    const std::string& signature,
    const std::string& error_message) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!error_message.empty()) {
    LOG(ERROR) << "Failed to sign CSR: " << error_message;
    UpdateState(CertProvisioningWorkerState::kFailed);
    return;
  }

  signature_ = signature;
  UpdateState(CertProvisioningWorkerState::kSignCsrFinished);
  DoStep();
}

void CertProvisioningWorkerImpl::FinishCsr() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  cloud_policy_client_->ClientCertProvisioningFinishCsr(
      CertScopeToString(cert_scope_), cert_profile_.profile_id, public_key_,
      va_challenge_response_, signature_,
      base::BindOnce(&CertProvisioningWorkerImpl::OnFinishCsrDone,
                     weak_factory_.GetWeakPtr()));
}

void CertProvisioningWorkerImpl::OnFinishCsrDone(
    policy::DeviceManagementStatus status,
    base::Optional<CertProvisioningResponseErrorType> error,
    base::Optional<int64_t> try_later) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!ProcessResponseErrors(status, error, try_later)) {
    return;
  }

  UpdateState(CertProvisioningWorkerState::kFinishCsrResponseReceived);
  DoStep();
}

void CertProvisioningWorkerImpl::DownloadCert() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  cloud_policy_client_->ClientCertProvisioningDownloadCert(
      CertScopeToString(cert_scope_), cert_profile_.profile_id, public_key_,
      base::BindOnce(&CertProvisioningWorkerImpl::OnDownloadCertDone,
                     weak_factory_.GetWeakPtr()));
}

void CertProvisioningWorkerImpl::OnDownloadCertDone(
    policy::DeviceManagementStatus status,
    base::Optional<CertProvisioningResponseErrorType> error,
    base::Optional<int64_t> try_later,
    const std::string& pem_encoded_certificate) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!ProcessResponseErrors(status, error, try_later)) {
    return;
  }

  pem_encoded_certificate_ = pem_encoded_certificate;
  UpdateState(CertProvisioningWorkerState::kDownloadCertResponseReceived);

  DoStep();
}

void CertProvisioningWorkerImpl::ImportCert() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  net::CertificateList cert_list =
      net::X509Certificate::CreateCertificateListFromBytes(
          pem_encoded_certificate_.data(), pem_encoded_certificate_.size(),
          net::X509Certificate::FORMAT_AUTO);

  if (cert_list.size() != 1) {
    LOG(ERROR) << "Unexpected certificate content: size " << cert_list.size();
    UpdateState(CertProvisioningWorkerState::kFailed);
    return;
  }

  platform_keys_service_->ImportCertificate(
      GetPlatformKeysTokenId(cert_scope_), cert_list.front(),
      base::BindRepeating(&CertProvisioningWorkerImpl::OnImportCertDone,
                          weak_factory_.GetWeakPtr()));
}

void CertProvisioningWorkerImpl::OnImportCertDone(
    const std::string& error_message) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!error_message.empty()) {
    LOG(ERROR) << "Failed to import certificate: " << error_message;
    UpdateState(CertProvisioningWorkerState::kFailed);
    return;
  }

  UpdateState(CertProvisioningWorkerState::kSucceed);
}

bool CertProvisioningWorkerImpl::IsFinished() const {
  switch (state_) {
    case CertProvisioningWorkerState::kSucceed:
    case CertProvisioningWorkerState::kFailed:
      return true;
    default:
      return false;
  }
}

bool CertProvisioningWorkerImpl::IsWaiting() const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return is_waiting_;
}

CertProvisioningWorkerState CertProvisioningWorkerImpl::GetState() const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return state_;
}

bool CertProvisioningWorkerImpl::ProcessResponseErrors(
    policy::DeviceManagementStatus status,
    base::Optional<CertProvisioningResponseErrorType> error,
    base::Optional<int64_t> try_later) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (status ==
      policy::DeviceManagementStatus::DM_STATUS_TEMPORARY_UNAVAILABLE) {
    LOG(WARNING) << "DM Server is temporary unavailable";
    request_backoff_.InformOfRequest(false);
    ScheduleNextStep(request_backoff_.GetTimeUntilRelease());
    return false;
  }

  if (status != policy::DeviceManagementStatus::DM_STATUS_SUCCESS) {
    LOG(ERROR) << "DM Server returned error: " << status;
    UpdateState(CertProvisioningWorkerState::kFailed);
    return false;
  }

  request_backoff_.InformOfRequest(true);

  if (error.has_value()) {
    LOG(ERROR) << "Server response contains error: " << error.value();
    UpdateState(CertProvisioningWorkerState::kFailed);
    return false;
  }

  if (try_later.has_value()) {
    ScheduleNextStep(base::TimeDelta::FromMilliseconds(try_later.value()));
    return false;
  }

  return true;
}

void CertProvisioningWorkerImpl::ScheduleNextStep(base::TimeDelta delay) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  delay = std::max(delay, kMinumumTryAgainLaterDelay);

  base::SequencedTaskRunnerHandle::Get()->PostDelayedTask(
      FROM_HERE,
      base::Bind(&CertProvisioningWorkerImpl::DoStep,
                 weak_factory_.GetWeakPtr()),
      delay);

  is_waiting_ = true;
  VLOG(0) << "Next step scheduled in " << delay;
}

void CertProvisioningWorkerImpl::CancelScheduledTasks() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  weak_factory_.InvalidateWeakPtrs();
}

}  // namespace cert_provisioning
}  // namespace chromeos
