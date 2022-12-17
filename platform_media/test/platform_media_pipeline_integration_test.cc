// -*- Mode: c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
//
// Copyright (c) 2018 Vivaldi Technologies AS. All rights reserved.
// Copyright (C) 2014 Opera Software ASA.  All rights reserved.
//
// This file is an original work developed by Opera Software ASA

#include "media/test/pipeline_integration_test_base.h"

#include "base/command_line.h"
#include "base/path_service.h"
#include "base/vivaldi_paths.h"
#include "media/base/audio_decoder_config.h"
#include "media/base/media_log.h"
#include "media/base/media_switches.h"
#include "media/base/test_data_util.h"
#include "media/base/video_decoder_config.h"
#include "media/filters/file_data_source.h"
#include "media/test/test_media_source.h"

#include "platform_media/renderer/decoders/ipc_demuxer.h"
#include "platform_media/test/ipc_pipeline_test_setup.h"
#if defined(OS_MAC)
#include "base/mac/mac_util.h"
#endif

#if defined(OS_WIN)
#include "base/win/windows_version.h"
#endif

namespace media {

namespace {

#if defined(OS_WIN)
DemuxerStream* GetStream(std::unique_ptr<Demuxer>& demuxer,
                         DemuxerStream::Type type) {
  std::vector<DemuxerStream*> streams = demuxer->GetAllStreams();
  for (auto* stream : streams) {
    if (stream->type() == type)
      return stream;
  }
  return nullptr;
}
#endif  // defined(OS_WIN)

const base::FilePath::CharType kPlatformMediaTestDataPath[] =
    FILE_PATH_LITERAL("platform_media");

base::FilePath GetPlatformMediaTestDataPath() {
  return base::FilePath(kPlatformMediaTestDataPath);
}

}  // namespace

base::FilePath GetVivaldiTestDataFilePath(const std::string& name) {
  base::FilePath file_path;
  CHECK(base::PathService::Get(vivaldi::DIR_VIVALDI_TEST_DATA, &file_path));
  return file_path.Append(GetPlatformMediaTestDataPath()).AppendASCII(name);
}

class PlatformMediaMockMediaSource : public TestMediaSource {
 public:
  PlatformMediaMockMediaSource(const std::string& filename,
                               const std::string& mimetype,
                               size_t initial_append_size,
                               bool initial_sequence_mode = false)
      : TestMediaSource(filename,
                        mimetype,
                        initial_append_size,
                        initial_sequence_mode,
                        GetVivaldiTestDataFilePath(filename)) {}
};

class PlatformMediaPipelineIntegrationTest
    : public testing::Test,
      public PipelineIntegrationTestBase {
 public:
  ~PlatformMediaPipelineIntegrationTest() override {
    // Make sure the demuxer is detroyed before ipc_pipeline_test_setup_ as that
    // waits for all IPC to finish.
    if (pipeline_->IsRunning())
      Stop();

    demuxer_.reset();
  }

  void SetUp() override {
    static bool registered = false;
    if (!registered) {
      vivaldi::RegisterVivaldiPaths();
      registered = true;
    }
  }
  PipelineStatus StartVivaldiWithFile(
      const std::string& filename,
      CdmContext* cdm_context,
      uint8_t test_type,
      CreateVideoDecodersCB prepend_video_decoders_cb = CreateVideoDecodersCB(),
      CreateAudioDecodersCB prepend_audio_decoders_cb =
          CreateAudioDecodersCB()) {
    std::unique_ptr<FileDataSource> file_data_source(new FileDataSource());
    base::FilePath file_path(GetVivaldiTestDataFilePath(filename));
    CHECK(file_data_source->Initialize(file_path))
        << "Is " << file_path.value() << " missing?";
#if defined(USE_SYSTEM_PROPRIETARY_CODECS)
    filepath_ = file_path;
#endif
    return StartInternal(std::move(file_data_source), cdm_context, test_type,
                         prepend_video_decoders_cb, prepend_audio_decoders_cb);
  }

  PipelineStatus StartVivaldi(const std::string& filename) {
    return StartVivaldiWithFile(filename, nullptr, kNormal);
  }

  PipelineStatus StartVivaldi(const std::string& filename,
                              CdmContext* cdm_context) {
    return StartVivaldiWithFile(filename, cdm_context, kNormal);
  }
  PipelineStatus StartVivaldi(
      const std::string& filename,
      uint8_t test_type,
      CreateVideoDecodersCB prepend_video_decoders_cb = CreateVideoDecodersCB(),
      CreateAudioDecodersCB prepend_audio_decoders_cb =
          CreateAudioDecodersCB()) {
    return StartVivaldiWithFile(filename, nullptr, test_type,
                                prepend_video_decoders_cb,
                                prepend_audio_decoders_cb);
  }
  IPCPipelineTestSetup ipc_pipeline_test_setup_;
};

#if defined(OS_MAC) || defined(OS_WIN)
TEST_F(PlatformMediaPipelineIntegrationTest, BasicPlayback) {
  ASSERT_EQ(PIPELINE_OK, Start("bear.mp4", kHashed));

  Play();

  ASSERT_TRUE(WaitUntilOnEnded());

#if defined(OS_MAC)
  EXPECT_EQ("bd1d880e4934bf76c0bb34450cd0f173", GetVideoHash());
  EXPECT_EQ("-0.51,0.54,1.03,0.85,-0.08,-0.22,", GetAudioHash());
#elif defined(OS_WIN)
  EXPECT_EQ("eb228dfe6882747111161156164dcab0", GetVideoHash());
  EXPECT_EQ("-0.52,0.26,0.16,0.24,-0.00,0.26,", GetAudioHash());
#endif
  EXPECT_TRUE(demuxer_->GetTimelineOffset().is_null());
}

TEST_F(PlatformMediaPipelineIntegrationTest, BasicPlayback_16x9_Aspect) {
  ASSERT_EQ(PIPELINE_OK,
            StartVivaldi("vivaldi-bear-320x240-16x9-aspect.mp4", kHashed));

  Play();

  ASSERT_TRUE(WaitUntilOnEnded());

#if defined(OS_MAC)
  EXPECT_EQ("e9a2e53ef2c16757962cc58d37de69e7", GetVideoHash());
  EXPECT_EQ("-3.66,-2.08,0.22,2.09,0.64,-0.90,", GetAudioHash());
#elif defined(OS_WIN)
  EXPECT_EQ("e9a2e53ef2c16757962cc58d37de69e7", GetVideoHash());
  EXPECT_EQ("-3.60,-1.82,0.28,1.90,0.34,-1.09,", GetAudioHash());
#endif
}

TEST_F(PlatformMediaPipelineIntegrationTest, BasicPlayback_VideoOnly) {
  ASSERT_EQ(PIPELINE_OK, Start("bear_silent.mp4", kHashed));

  Play();

  ASSERT_TRUE(WaitUntilOnEnded());

#if defined(OS_MAC)
  // On OS X, the expected hashes can be different, because our solution
  // doesn't necessarily process frames one by one, see AVFMediaDecoder.
  EXPECT_EQ("e7832270a91e8de7945b5724eec2cbcb", GetVideoHash());
#elif defined(OS_WIN)
  EXPECT_EQ("eb228dfe6882747111161156164dcab0", GetVideoHash());
#endif
}

TEST_F(PlatformMediaPipelineIntegrationTest, BasicPlayback_M4A) {
  ASSERT_EQ(PIPELINE_OK, Start("sfx.m4a", kHashed));

  Play();

  ASSERT_TRUE(WaitUntilOnEnded());

#if defined(OS_MAC)
  EXPECT_EQ("-5.29,-5.47,-5.05,-4.33,-2.99,-3.79,", GetAudioHash());
#elif defined(OS_WIN)
  EXPECT_EQ("0.46,1.72,4.26,4.57,3.39,1.54,", GetAudioHash());
#endif
}

TEST_F(PlatformMediaPipelineIntegrationTest, SeekWhilePaused) {
  ASSERT_EQ(PIPELINE_OK, Start("bear.mp4"));

  base::TimeDelta duration(pipeline_->GetMediaDuration());
  base::TimeDelta start_seek_time(duration / 4);
  base::TimeDelta seek_time(duration * 3 / 4);

  Play();
  ASSERT_TRUE(WaitUntilCurrentTimeIsAfter(start_seek_time));
  Pause();
  ASSERT_TRUE(Seek(seek_time));
  EXPECT_EQ(pipeline_->GetMediaTime(), seek_time);
  Play();
  ASSERT_TRUE(WaitUntilOnEnded());

  // Make sure seeking after reaching the end works as expected.
  Pause();
  ASSERT_TRUE(Seek(seek_time));
  EXPECT_EQ(pipeline_->GetMediaTime(), seek_time);
  Play();
  ASSERT_TRUE(WaitUntilOnEnded());
}

TEST_F(PlatformMediaPipelineIntegrationTest, SeekWhilePlaying) {
  ASSERT_EQ(PIPELINE_OK, Start("bear.mp4"));

  base::TimeDelta duration(pipeline_->GetMediaDuration());
  base::TimeDelta start_seek_time(duration / 4);
  base::TimeDelta seek_time(duration * 3 / 4);

  Play();
  ASSERT_TRUE(WaitUntilCurrentTimeIsAfter(start_seek_time));
  ASSERT_TRUE(Seek(seek_time));
  EXPECT_GE(pipeline_->GetMediaTime(), seek_time);
  ASSERT_TRUE(WaitUntilOnEnded());

  // Make sure seeking after reaching the end works as expected.
  ASSERT_TRUE(Seek(seek_time));
  EXPECT_GE(pipeline_->GetMediaTime(), seek_time);
  ASSERT_TRUE(WaitUntilOnEnded());
}

TEST_F(PlatformMediaPipelineIntegrationTest, Seek_VideoOnly) {
  ASSERT_EQ(PIPELINE_OK, Start("bear_silent.mp4", kHashed));

  Play();
  ASSERT_TRUE(Seek(pipeline_->GetMediaDuration() / 2));

  ASSERT_TRUE(WaitUntilOnEnded());
}

TEST_F(PlatformMediaPipelineIntegrationTest, PlayInLoop) {
  ASSERT_EQ(PIPELINE_OK, Start("bear.mp4"));

  const base::TimeDelta duration = pipeline_->GetMediaDuration();
  const base::TimeDelta play_time = duration / 4;

  Play();
  ASSERT_TRUE(WaitUntilCurrentTimeIsAfter(play_time));
  ASSERT_TRUE(Seek(duration));
  ASSERT_TRUE(WaitUntilOnEnded());

  ASSERT_TRUE(Seek(base::TimeDelta()));
  ASSERT_LT(pipeline_->GetMediaTime(), play_time);
  ASSERT_TRUE(WaitUntilCurrentTimeIsAfter(play_time));
}

TEST_F(PlatformMediaPipelineIntegrationTest, TruncatedMedia) {
  ASSERT_EQ(PIPELINE_OK,
            StartVivaldi("vivaldi-bear_truncated.mp4"));

  Play();
  WaitUntilCurrentTimeIsAfter(base::Microseconds(1066666));
  ASSERT_TRUE(ended_ || pipeline_status_ != PIPELINE_OK);
}

TEST_F(PlatformMediaPipelineIntegrationTest, DecodingError) {
  // TODO(wdzierzanowski): WMFMediaPipeline (Windows) doesn't detect the error?
  // (DNA-30324).
#if !defined(OS_WIN)
  ASSERT_EQ(PIPELINE_OK, StartVivaldi("bear_corrupt.mp4"));
  Play();
  EXPECT_EQ(PIPELINE_ERROR_DECODE, WaitUntilEndedOrError());
#endif
}

TEST_F(PlatformMediaPipelineIntegrationTest, Rotated_Metadata_0) {
  // This is known not to work on Windows systems older than 8.
#if defined(OS_WIN)
  if (base::win::GetVersion() < base::win::Version::WIN8)
    return;
#endif

  ASSERT_EQ(PIPELINE_OK, Start("bear_rotate_0.mp4"));
  ASSERT_EQ(VideoRotation::VIDEO_ROTATION_0,
            metadata_.video_decoder_config.video_transformation());
}

TEST_F(PlatformMediaPipelineIntegrationTest, Rotated_Metadata_90) {
  // This is known not to work on Windows systems older than 8.
#if defined(OS_WIN)
  if (base::win::GetVersion() < base::win::Version::WIN8)
    return;
#endif

  ASSERT_EQ(PIPELINE_OK, Start("bear_rotate_90.mp4"));
  ASSERT_EQ(VideoRotation::VIDEO_ROTATION_90,
            metadata_.video_decoder_config.video_transformation());
}

TEST_F(PlatformMediaPipelineIntegrationTest, Rotated_Metadata_180) {
  // This is known not to work on Windows systems older than 8.
#if defined(OS_WIN)
  if (base::win::GetVersion() < base::win::Version::WIN8)
    return;
#endif

  ASSERT_EQ(PIPELINE_OK, Start("bear_rotate_180.mp4"));
  ASSERT_EQ(VideoRotation::VIDEO_ROTATION_180,
            metadata_.video_decoder_config.video_transformation());
}

TEST_F(PlatformMediaPipelineIntegrationTest, Rotated_Metadata_270) {
  // This is known not to work on Windows systems older than 8.
#if defined(OS_WIN)
  if (base::win::GetVersion() < base::win::Version::WIN8)
    return;
#endif

  ASSERT_EQ(PIPELINE_OK, Start("bear_rotate_270.mp4"));
  ASSERT_EQ(VideoRotation::VIDEO_ROTATION_270,
            metadata_.video_decoder_config.video_transformation());
}

// Configuration change happens only on Windows.
#if defined(OS_WIN)
TEST_F(PlatformMediaPipelineIntegrationTest, AudioConfigChange) {
  ASSERT_EQ(PIPELINE_OK,
            StartVivaldi("vivaldi-config_change_audio.mp4"));

  Play();

  AudioDecoderConfig audio_config =
      GetStream(demuxer_, DemuxerStream::AUDIO)->audio_decoder_config();

  EXPECT_EQ(audio_config.samples_per_second(), 24000);

  ASSERT_TRUE(WaitUntilOnEnded());

  audio_config =
      GetStream(demuxer_, DemuxerStream::AUDIO)->audio_decoder_config();
  EXPECT_EQ(audio_config.samples_per_second(), 48000);
}

TEST_F(PlatformMediaPipelineIntegrationTest, VideoConfigChange) {
  ASSERT_EQ(PIPELINE_OK,
            StartVivaldi("vivaldi-config_change_video.mp4"));

  Play();

  VideoDecoderConfig video_config =
      GetStream(demuxer_, DemuxerStream::VIDEO)->video_decoder_config();
  EXPECT_EQ(video_config.coded_size().height(), 270);

  ASSERT_TRUE(WaitUntilOnEnded());

  video_config =
      GetStream(demuxer_, DemuxerStream::VIDEO)->video_decoder_config();
  EXPECT_EQ(video_config.coded_size().height(), 272);
}
#endif  // defined(OS_WIN)

TEST_F(PlatformMediaPipelineIntegrationTest, BasicPlaybackPositiveStartTime) {
  ASSERT_EQ(PIPELINE_OK,
            StartVivaldi("vivaldi-nonzero-start-time.mp4"));
  Play();
  ASSERT_TRUE(WaitUntilOnEnded());
  ASSERT_EQ(base::Microseconds(390000),
            demuxer_->GetStartTime());
}

#endif  // defined(OS_MAC) || defined(OS_WIN)

TEST_F(PlatformMediaPipelineIntegrationTest,
       BasicPlayback_MediaSource_MP4_AudioOnly) {
  PlatformMediaMockMediaSource source("what-does-the-fox-say.mp4",
                                      "audio/mp4; codecs=\"mp4a.40.5\"",
                                      kAppendWholeFile);
  StartPipelineWithMediaSource(&source);
  source.EndOfStream();

  EXPECT_EQ(1u, pipeline_->GetBufferedTimeRanges().size());
  EXPECT_EQ(0, pipeline_->GetBufferedTimeRanges().start(0).InMilliseconds());
  EXPECT_EQ(1493, pipeline_->GetBufferedTimeRanges().end(0).InMilliseconds());

  Play();

  ASSERT_TRUE(WaitUntilOnEnded());
  source.Shutdown();
  Stop();
}

}  // namespace media
