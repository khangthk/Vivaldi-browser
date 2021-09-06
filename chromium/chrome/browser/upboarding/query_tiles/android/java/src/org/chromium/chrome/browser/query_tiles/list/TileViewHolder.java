// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.query_tiles.list;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.recyclerview.widget.RecyclerView.ViewHolder;

import org.chromium.chrome.browser.query_tiles.Tile;
import org.chromium.chrome.query_tiles.R;
import org.chromium.ui.modelutil.PropertyModel;

/**
 * A {@link ViewHolder} responsible for building and setting properties on the query tile views.
 */
class TileViewHolder extends ViewHolder {
    /** Creates an instance of a {@link TileViewHolder}. */
    protected TileViewHolder(View itemView) {
        super(itemView);
    }

    /**
     * Used as a method reference for ViewHolderFactory.
     * @see RecyclerViewAdapter.ViewHolderFactory#createViewHolder
     */
    public static TileViewHolder create(ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext())
                            .inflate(R.layout.query_tile_view, parent, false);
        return new TileViewHolder(view);
    }

    /**
     * Binds the currently held {@link View} to {@code item}.
     * @param properties The shared {@link PropertyModel} all items can access.
     * @param tile       The {@link ListItem} to visually represent in this {@link ViewHolder}.
     */
    public void bind(PropertyModel properties, Tile tile) {
        TextView title = itemView.findViewById(R.id.title);
        title.setText(tile.displayTitle);
        itemView.setOnClickListener(
                v -> { properties.get(TileListProperties.CLICK_CALLBACK).onResult(tile); });

        final ImageView thumbnail = itemView.findViewById(R.id.thumbnail);
        properties.get(TileListProperties.VISUALS_CALLBACK).getVisuals(tile, bitmaps -> {
            thumbnail.setImageBitmap(
                    (bitmaps == null || bitmaps.isEmpty()) ? null : bitmaps.get(0));
        });
    }

    /**
     * Gives subclasses a chance to free up expensive resources when this {@link ViewHolder} is no
     * longer attached to the parent {@link RecyclerView}.
     */
    public void recycle() {}
}
