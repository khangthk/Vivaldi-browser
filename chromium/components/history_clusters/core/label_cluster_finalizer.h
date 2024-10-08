// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_HISTORY_CLUSTERS_CORE_LABEL_CLUSTER_FINALIZER_H_
#define COMPONENTS_HISTORY_CLUSTERS_CORE_LABEL_CLUSTER_FINALIZER_H_

#include "components/history_clusters/core/cluster_finalizer.h"

namespace history_clusters {

// A cluster finalizer that determines the label for a given cluster.
class LabelClusterFinalizer : public ClusterFinalizer {
 public:
  LabelClusterFinalizer();
  ~LabelClusterFinalizer() override;

  // ClusterFinalizer:
  void FinalizeCluster(history::Cluster& cluster) override;
};

}  // namespace history_clusters

#endif  // COMPONENTS_HISTORY_CLUSTERS_CORE_LABEL_CLUSTER_FINALIZER_H_
