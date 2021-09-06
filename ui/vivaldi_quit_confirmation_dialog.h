// Copyright (c) 2018 Vivaldi Technologies AS. All rights reserved

#ifndef UI_QUIT_CONFIRMATION_DIALOG_H_
#define UI_QUIT_CONFIRMATION_DIALOG_H_

#include <string>
#include "base/callback_forward.h"
#include "ui/views/window/dialog_delegate.h"

namespace views {
class Label;
class Checkbox;
}  // namespace views

namespace vivaldi {

class VivaldiDialogDelegate {
 public:
  virtual ~VivaldiDialogDelegate() = default;

  virtual base::string16 GetWindowTitle() = 0;
  virtual base::string16 GetBodyText() = 0;
  virtual base::string16 GetCheckboxText() = 0;
};

class VivaldiDialogQuitDelegate : public VivaldiDialogDelegate {
 public:
  base::string16 GetWindowTitle() override;
  base::string16 GetBodyText() override;
  base::string16 GetCheckboxText() override;
};

class VivaldiDialogCloseWindowDelegate : public VivaldiDialogDelegate {
 public:
  base::string16 GetWindowTitle() override;
  base::string16 GetBodyText() override;
  base::string16 GetCheckboxText() override;
};

// Dialog class for prompting users to quit
class VivaldiQuitConfirmationDialog : public views::DialogDelegateView {
 public:
  // If stop_asking is true the user should not be asked for
  // a confirmation again.
  using QuitCallback = base::OnceCallback<void(bool close, bool stop_asking)>;

  VivaldiQuitConfirmationDialog(QuitCallback quit_callback,
                                gfx::NativeWindow window,
                                gfx::NativeView view,
                                VivaldiDialogDelegate* delegate);
  ~VivaldiQuitConfirmationDialog() override;

  // views::DialogDelegateView:
  bool Accept() override;
  bool Cancel() override;

  // views::WidgetDelegate:
  ui::ModalType GetModalType() const override;
  base::string16 GetWindowTitle() const override;
  bool ShouldShowCloseButton() const override;

  // views::View:
  gfx::Size CalculatePreferredSize() const override;

 private:
  void RunCallback(bool close);

  std::unique_ptr<views::Checkbox> CreateExtraView();

  QuitCallback quit_callback_;

  views::Label* label_;
  views::Checkbox* checkbox_ = nullptr;

  // The dialog takes ownership of the delegate
  std::unique_ptr<VivaldiDialogDelegate> delegate_;

  DISALLOW_COPY_AND_ASSIGN(VivaldiQuitConfirmationDialog);
};

}  // namespace vivaldi

#endif  // UI_QUIT_CONFIRMATION_DIALOG_H_
