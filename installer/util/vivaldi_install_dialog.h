// Copyright (c) 2020 Vivaldi Technologies AS. All rights reserved.

#ifndef INSTALLER_UTIL_VIVALDI_INSTALL_DIALOG_H_
#define INSTALLER_UTIL_VIVALDI_INSTALL_DIALOG_H_

#include <windows.h>

#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/optional.h"
#include "chrome/installer/util/l10n_string_util.h"
#include "chrome/installer/util/util_constants.h"

#include "installer/util/vivaldi_install_util.h"

namespace installer {

struct VivaldiInstallUIOptions {
  VivaldiInstallUIOptions();
  ~VivaldiInstallUIOptions();

  // This is a move-only struct.
  VivaldiInstallUIOptions(VivaldiInstallUIOptions&&);
  VivaldiInstallUIOptions& operator=(VivaldiInstallUIOptions&&);

  base::FilePath install_dir;
  vivaldi::InstallType install_type = vivaldi::InstallType::kForCurrentUser;

  // On Windows 8 and later this applies only to a standalone installation and
  // skips the registration with Windows as a browser application. Under Windows
  // 7 for a non-standalone install this makes the browser the default
  // automatically after always regisring it. For standalone installs on Windows
  // 7 this both registers the browser and sets it as the default automatically.
  // The case of just registering a standalone browser without making it the
  // default on Windows 7 is not supported.
  bool register_browser = false;

  // Flags indicating that the values above are explicitly set via a command
  // option and any value from the registry should be ignored.
  bool given_install_type = false;
  bool given_register_browser = false;
};

VivaldiInstallUIOptions ReadRegistryPreferences();

class VivaldiInstallDialog {
 public:
  enum DlgResult {
    INSTALL_DLG_ERROR = -1,   // Dialog could not be shown.
    INSTALL_DLG_CANCEL = 0,   // The user cancelled install.
    INSTALL_DLG_INSTALL = 1,  // The user clicked the install button.
  };

  enum Scaling {
    DPI_NORMAL,  // 100%
    DPI_MEDIUM,  // 125%
    DPI_LARGE,   // 150%
    DPI_XL,      // 200%
    DPI_XXL      // 250%
  };

  VivaldiInstallDialog(HINSTANCE instance, VivaldiInstallUIOptions options);

  virtual ~VivaldiInstallDialog();

  DlgResult ShowModal();

  VivaldiInstallUIOptions ExtractOptions() { return std::move(options_); }

 private:
  void InitDialog();
  void TranslateDialog();
  void ShowBrowseFolderDialog();
  void DoDialog();
  void ReadLastInstallValues();
  void SaveInstallValues();
  bool InternalSelectLanguage();

  void OnInstallTypeSelection();
  void OnLanguageSelection();
  bool IsInstallPathValid(const base::FilePath& path);
  installer::InstallStatus ShowEULADialog();
  std::wstring GetInnerFrameEULAResource();

  void ShowOptions();
  void UpdateTypeSpecificOptionsVisibility();
  void EnableAndShowControl(int id, bool show);
  void ShowControl(int id, bool show);

  void InitBkgnd();
  void InitCtlBrushes();
  void ClearAll();
  void Resize();
  void Center();

  BOOL OnEraseBkgnd(HDC hdc);
  HBRUSH OnCtlColor(HWND hwnd_ctl, HDC hdc);

  HBRUSH CreateDIBrush(int x, int y, int cx, int cy);
  HBRUSH GetCtlBrush(int id_dlg_item);

  static INT_PTR CALLBACK DlgProc(HWND hdlg,
                                  UINT msg,
                                  WPARAM wparam,
                                  LPARAM lparam);

 private:
  VivaldiInstallUIOptions options_;
  bool disable_standalone_autoupdates_ = false;

  std::wstring txt_tos_accept_install_str_;
  std::wstring btn_tos_accept_install_str_;
  std::wstring txt_tos_accept_update_str_;
  std::wstring btn_tos_accept_update_str_;
  std::wstring btn_simple_mode_str_;
  std::wstring btn_advanced_mode_str_;

  base::FilePath last_standalone_folder_;
  bool is_upgrade_ = false;
  bool dialog_ended_ = false;
  bool advanced_mode_ = false;
  HWND hdlg_ = NULL;
  HINSTANCE instance_ = NULL;
  DlgResult dlg_result_ = INSTALL_DLG_ERROR;
  Scaling dpi_scale_ = DPI_NORMAL;
  HBITMAP hbitmap_bkgnd_ = NULL;
  HBITMAP back_bmp_ = NULL;
  LPVOID back_bits_ = NULL;
  LONG back_bmp_width_ = 0;
  LONG back_bmp_height_ = 0;
  std::vector<std::pair<int, HBRUSH>> brushes_;
  std::vector<HGLOBAL> dibs_;
  bool changed_language_ = false;

  static VivaldiInstallDialog* this_;

  DISALLOW_COPY_AND_ASSIGN(VivaldiInstallDialog);
};

}  // namespace installer
#endif  // INSTALLER_UTIL_VIVALDI_INSTALL_DIALOG_H_
