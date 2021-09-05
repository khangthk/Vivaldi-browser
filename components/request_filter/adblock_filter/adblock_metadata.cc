// Copyright (c) 2019 Vivaldi Technologies AS. All rights reserved

#include "components/request_filter/adblock_filter/adblock_metadata.h"

#include "base/hash/hash.h"

namespace adblock_filter {
AdBlockMetadata::AdBlockMetadata() = default;
AdBlockMetadata::~AdBlockMetadata() = default;
AdBlockMetadata::AdBlockMetadata(const AdBlockMetadata&) = default;

bool AdBlockMetadata::operator==(const AdBlockMetadata& other) const {
  return homepage == other.homepage && title == other.title &&
         expires == other.expires && license == other.license &&
         redirect == other.redirect && version == other.version;
}

RuleSourceBase::RuleSourceBase() = default;
RuleSourceBase::RuleSourceBase(const GURL& source_url, RuleGroup group)
    : source_url(source_url),
      is_from_url(true),
      group(group),
      id(base::PersistentHash(source_url.spec())) {}
RuleSourceBase::RuleSourceBase(const base::FilePath& source_file,
                               RuleGroup group)
    : source_file(source_file),
      is_from_url(false),
      group(group),
      id(base::PersistentHash(source_file.AsUTF8Unsafe())) {}
RuleSourceBase::~RuleSourceBase() = default;
RuleSourceBase::RuleSourceBase(const RuleSourceBase&) = default;

RuleSource::RuleSource(const GURL& source_url, RuleGroup group)
    : RuleSourceBase(source_url, group) {}
RuleSource::RuleSource(const base::FilePath& source_file, RuleGroup group)
    : RuleSourceBase(source_file, group) {}
RuleSource::~RuleSource() = default;
RuleSource::RuleSource(const RuleSource&) = default;
}  // namespace adblock_filter
