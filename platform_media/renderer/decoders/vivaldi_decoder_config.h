// Copyright (c) 2021 Vivaldi Technologies AS. All rights reserved.

#ifndef PLATFORM_MEDIA_RENDERER_DECODERS_VIVALDI_DECODER_CONFIG_H_
#define PLATFORM_MEDIA_RENDERER_DECODERS_VIVALDI_DECODER_CONFIG_H_

#include <memory>
#include <vector>

#include "base/memory/scoped_refptr.h"
#include "base/task/sequenced_task_runner.h"

namespace media {

class AudioDecoder;
class MediaLog;
class VideoDecoder;

class VivaldiDecoderConfig {
 public:
  static void AddAudioDecoders(
      const scoped_refptr<base::SequencedTaskRunner>& task_runner,
      MediaLog* media_log,
      std::vector<std::unique_ptr<AudioDecoder>>& decoders);

  static void AddVideoDecoders(
      scoped_refptr<base::SequencedTaskRunner> task_runner,
      MediaLog* media_log,
      std::vector<std::unique_ptr<VideoDecoder>>& decoders);
};

}  // namespace media

#endif  // PLATFORM_MEDIA_RENDERER_DECODERS_VIVALDI_DECODER_CONFIG_H_
