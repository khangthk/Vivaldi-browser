// -*- Mode: c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
//
// Copyright (c) 2018 Vivaldi Technologies AS. All rights reserved.
// Copyright (C) 2014 Opera Software ASA.  All rights reserved.
//
// This file is an original work developed by Opera Software ASA

#ifndef PLATFORM_MEDIA_RENDERER_DECODERS_IPC_DEMUXER_H_
#define PLATFORM_MEDIA_RENDERER_DECODERS_IPC_DEMUXER_H_

#include "platform_media/common/feature_toggles.h"

#include <string>

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/single_thread_task_runner.h"
#include "media/base/demuxer.h"
#include "media/base/media_export.h"

class GURL;

namespace media {

class IPCDemuxerStream;
class IPCMediaPipelineHost;
class MediaLog;
struct PlatformAudioConfig;
struct PlatformMediaTimeInfo;
struct PlatformVideoConfig;

// An implementation of the demuxer interface. On its creation it should create
// the media IPC. It is wrapping all of the demuxer functionality, so that the
// IPC part is transparent. It is also responsible for providing the data
// source for the IPCMediaPipelineHost.
class MEDIA_EXPORT IPCDemuxer : public Demuxer {
 public:
  IPCDemuxer(std::unique_ptr<IPCMediaPipelineHost> ipc_media_pipeline_host,
             MediaLog* media_log);
  ~IPCDemuxer() override;

  // Checks if the content is supported by this demuxer. Return an empty string
  // if this is not possible or adjusted mime type.
  static std::string CanPlayType(const std::string& content_type, const GURL& url);
  static bool CanPlayType(const std::string& content_type);

  // Demuxer implementation.
  //
  // All Demuxer functions are expected to be called on |task_runner_|'s
  // thread.
  std::string GetDisplayName() const override;
  void Initialize(DemuxerHost* host,
                  PipelineStatusCallback status_cb) override;
  void Seek(base::TimeDelta time,
            PipelineStatusCallback status_cb) override;
  void Stop() override;
  void AbortPendingReads() override;
  std::vector<DemuxerStream*> GetAllStreams() override;
  IPCDemuxerStream* GetStream(DemuxerStream::Type type);
  base::TimeDelta GetStartTime() const override;
  base::Time GetTimelineOffset() const override;
  int64_t GetMemoryUsage() const override;
  void OnEnabledAudioTracksChanged(const std::vector<MediaTrack::Id>& track_ids,
                                   base::TimeDelta currTime,
                                   TrackChangeCB change_completed_cb) override;
  void OnSelectedVideoTrackChanged(const std::vector<MediaTrack::Id>& track_ids,
                                   base::TimeDelta currTime,
                                   TrackChangeCB change_completed_cb) override;
  absl::optional<container_names::MediaContainerName> GetContainerForMetrics()
      const override;

  // Used to tell the demuxer that a seek request is about to arrive on the
  // media thread.  This lets the demuxer drop everything it was doing and
  // become ready to handle the seek request quickly.
  //
  // This function is called from the main thread.
  void StartWaitingForSeek(base::TimeDelta seek_time) override;
  void CancelPendingSeek(base::TimeDelta seek_time) override;

 protected:
  void StartWaitingForSeekOnMediaThread();

  std::unique_ptr<IPCMediaPipelineHost> ipc_media_pipeline_host_;
  std::unique_ptr<IPCDemuxerStream> audio_stream_;
  std::unique_ptr<IPCDemuxerStream> video_stream_;

  MediaLog* media_log_ = nullptr;

  base::WeakPtrFactory<IPCDemuxer> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(IPCDemuxer);
};

}  // namespace media

#endif  // PLATFORM_MEDIA_RENDERER_DECODERS_IPC_DEMUXER_H_
