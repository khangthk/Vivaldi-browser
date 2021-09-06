// Copyright (c) 2020 Vivaldi Technologies AS. All rights reserved

#ifndef UI_VIEWS_CONTROLS_MUTE_BUTTON_H_
#define UI_VIEWS_CONTROLS_MUTE_BUTTON_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/view.h"

namespace content {
class PictureInPictureWindowController;
}

namespace vivaldi {
////////////////////////////////////////////////////////////////////////////////
//
// MuteButton
//
// Used to mute/unmute current video playing in the PIP window.
//
////////////////////////////////////////////////////////////////////////////////

class VIEWS_EXPORT MuteButton : public views::ImageButton {
 public:
  enum class Mode { kMute = 0, kAudible };

  // The button's class name.
  static const char kViewClassName[];

  MuteButton(PressedCallback callback);
  ~MuteButton() override;

  // Set a specified button state.
  void ChangeMode(Mode mode, bool force);

  Mode muted_mode() { return muted_mode_; }

 private:
  Mode muted_mode_ = Mode::kAudible;

  DISALLOW_COPY_AND_ASSIGN(MuteButton);
};

}  // namespace vivaldi

#endif  // UI_VIEWS_CONTROLS_MUTE_BUTTON_H_
