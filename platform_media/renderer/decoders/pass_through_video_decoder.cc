// -*- Mode: c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
//
// Copyright (c) 2018 Vivaldi Technologies AS. All rights reserved.
// Copyright (C) 2014 Opera Software ASA.  All rights reserved.
//
// This file is an original work developed by Opera Software ASA

#include "platform_media/renderer/decoders/pass_through_video_decoder.h"

namespace media {

PassThroughVideoDecoder::PassThroughVideoDecoder(
    const scoped_refptr<base::SingleThreadTaskRunner>& task_runner)
    : impl_(task_runner) {
}

PassThroughVideoDecoder::~PassThroughVideoDecoder() {
}

void PassThroughVideoDecoder::Initialize(const VideoDecoderConfig& config,
                                         bool low_delay,
                                         CdmContext* cdm_context,
                                         InitCB init_cb,
                                         const OutputCB& output_cb,
                                         const WaitingCB& waiting_for_decryption_key_cb) {
  impl_.Initialize(config, std::move(init_cb), output_cb);
}

void PassThroughVideoDecoder::Decode(
    scoped_refptr<DecoderBuffer> buffer,
    DecodeCB decode_cb) {
  impl_.Decode(buffer, std::move(decode_cb));
}

void PassThroughVideoDecoder::Reset(base::OnceClosure closure) {
  impl_.Reset(std::move(closure));
}

std::string PassThroughVideoDecoder::GetDisplayName() const {
  return "PassThroughVideoDecoder";
}

VideoDecoderType PassThroughVideoDecoder::GetDecoderType() const {
  return VideoDecoderType::kDecrypting;
}

}  // namespace media
