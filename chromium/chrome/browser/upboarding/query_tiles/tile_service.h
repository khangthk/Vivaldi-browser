// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UPBOARDING_QUERY_TILES_TILE_SERVICE_H_
#define CHROME_BROWSER_UPBOARDING_QUERY_TILES_TILE_SERVICE_H_

#include <string>
#include <vector>

#include "base/callback.h"
#include "base/supports_user_data.h"
#include "chrome/browser/upboarding/query_tiles/query_tile_entry.h"
#include "components/keyed_service/core/keyed_service.h"

namespace gfx {
class Image;
}  // namespace gfx

namespace upboarding {

using GetTilesCallback =
    base::OnceCallback<void(const std::vector<QueryTileEntry*>&)>;
using VisualsCallback = base::OnceCallback<void(const gfx::Image&)>;

// The central class on chrome client responsible for fetching, storing,
// managing, and displaying query tiles in chrome.
class TileService : public KeyedService, public base::SupportsUserData {
 public:
  // Called to retrieve all the tiles.
  virtual void GetQueryTiles(GetTilesCallback callback) = 0;

  // Called to retrieve thumbnail for the given tile id.
  virtual void GetVisuals(const std::string& tile_id,
                          VisualsCallback callback) = 0;

  TileService() = default;
  ~TileService() override = default;

  TileService(const TileService&) = delete;
  TileService& operator=(const TileService&) = delete;
};

}  // namespace upboarding

#endif  // CHROME_BROWSER_UPBOARDING_QUERY_TILES_TILE_SERVICE_H_
