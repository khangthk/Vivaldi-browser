// Copyright (c) 2016 Vivaldi. All rights reserved.

#include "browser/menus/vivaldi_menus.h"

#include <string>
#include <vector>

#include "app/vivaldi_apptools.h"
#include "app/vivaldi_resources.h"
#include "base/files/file_util.h"
#include "base/strings/utf_string_conversions.h"
#include "browser/menus/vivaldi_menu_enums.h"
#include "browser/vivaldi_webcontents_util.h"
#include "chrome/app/chrome_command_ids.h"
#include "chrome/browser/app_mode/app_mode_utils.h"
#include "chrome/browser/devtools/devtools_window.h"
#include "chrome/browser/media/router/media_router_feature.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/browser/ui/views/renderer_context_menu/render_view_context_menu_views.h"
#include "chrome/common/pref_names.h"
#include "components/datasource/vivaldi_data_source_api.h"
#include "components/search_engines/template_url.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "extensions/api/features/vivaldi_runtime_feature.h"
#include "extensions/browser/guest_view/mime_handler_view/mime_handler_view_guest.h"
#include "extensions/browser/guest_view/web_view/web_view_guest.h"
#include "net/base/escape.h"
#include "third_party/blink/public/common/context_menu_data/edit_flags.h"
#include "third_party/blink/public/web/web_context_menu_data.h"
#include "ui/base/accelerators/accelerator.h"
#include "ui/base/clipboard/clipboard.h"
#include "ui/events/keycodes/keyboard_codes.h"
#include "ui/vivaldi_ui_utils.h"
#include "vivaldi/prefs/vivaldi_gen_prefs.h"

using blink::WebContextMenuData;
using extensions::WebViewGuest;

namespace vivaldi {

// Keep a copy of selected text. Chrome code in render_view_context_menu.cc will
// for some code paths remove whitespaces (newlines) in the ContextMenuParams
// object when setting up the menu (see search menu). We need the unmodified
// string in some cases.
static base::string16 s_selection_text;

std::string GetModifierStringFromEventFlags(int event_flags) {
  std::string modifiers;
  if (event_flags & ui::EF_CONTROL_DOWN)
    modifiers.append("ctrl");
  if (event_flags & ui::EF_SHIFT_DOWN) {
    if (modifiers.length() > 0)
      modifiers.append(",");
    modifiers.append("shift");
  }
  if (event_flags & ui::EF_ALT_DOWN) {
    if (modifiers.length() > 0)
      modifiers.append(",");
    modifiers.append("alt");
  }
  if (event_flags & ui::EF_COMMAND_DOWN) {
    if (modifiers.length() > 0)
      modifiers.append(",");
    modifiers.append("cmd");
  }
  return modifiers;
}

void SendSimpleAction(WebContents* web_contents,
                      int event_flags,
                      base::StringPiece command,
                      base::StringPiece text = base::StringPiece(),
                      base::StringPiece url = base::StringPiece()) {
  WebContents* wc = web_contents;
  // NOTE(david@vivaldi): If we're viewing a MimeHandlerViewGuest, use its
  // embedder WebContents.
#if BUILDFLAG(ENABLE_EXTENSIONS)
  auto* guest_view = extensions::MimeHandlerViewGuest::FromWebContents(wc);
  if (guest_view) {
    wc = guest_view->embedder_web_contents();
  }
  DevToolsWindow* win = DevToolsWindow::AsDevToolsWindow(wc);
  if (win && win->GetInspectedWebContents() != wc) {
    // We're inspecting a page, make sure to direct the event to the
    // inspected web contents.
    wc = win->GetInspectedWebContents();
    // Action was triggered from devtools, change the url to point to
    // the inspected page instead.
    GURL gurl = wc ? wc->GetURL() : GURL();
    if (gurl.is_valid()) {
      url = gurl.spec();
    }
  }
#endif
  WebViewGuest* guestView = WebViewGuest::FromWebContents(wc);
  if (!guestView)
    return;

  std::vector<base::Value> args;
  args.emplace_back(command);
  args.emplace_back(text);
  args.emplace_back(url);
  args.emplace_back(GetModifierStringFromEventFlags(event_flags));
  guestView->SimpleAction(base::ListValue(std::move(args)));
}

void SendSimpleAction(WebContents* web_contents,
                      int event_flags,
                      base::StringPiece command,
                      base::StringPiece16 text,
                      base::StringPiece url) {
  SendSimpleAction(web_contents, event_flags, command, base::UTF16ToUTF8(text),
                   url);
}

bool IsVivaldiCommandId(int id) {
  return (id >= IDC_VIVALDI_MENU_ENUMS_START &&
          id < IDC_VIVALDI_MENU_ENUMS_END);
}

void VivaldiInitMenu(WebContents* web_contents,
                     const ContextMenuParams& params) {
  s_selection_text = params.selection_text;
}

const GURL& GetDocumentURL(const ContextMenuParams& params) {
  return params.frame_url.is_empty() ? params.page_url : params.frame_url;
}

void VivaldiAddLinkItems(SimpleMenuModel* menu,
                         WebContents* web_contents,
                         const ContextMenuParams& params) {
  if (IsVivaldiRunning() && !params.link_url.is_empty()) {
    Profile* profile =
        Profile::FromBrowserContext(web_contents->GetBrowserContext());
    int firstIndex =
        menu->GetIndexOfCommandId(IDC_CONTENT_CONTEXT_OPENLINKNEWTAB);
    DCHECK_GE(firstIndex, 0);
    menu->RemoveItemAt(firstIndex);
    int index =
        menu->GetIndexOfCommandId(IDC_CONTENT_CONTEXT_OPENLINKNEWWINDOW);
    if (index > -1) {
      // Might not be there for links in a PWA window.
      menu->RemoveItemAt(index);
    }
    index = menu->GetIndexOfCommandId(IDC_CONTENT_CONTEXT_OPENLINKOFFTHERECORD);
    DCHECK_GE(index, 0);
    menu->RemoveItemAt(index);

    index = firstIndex;
    menu->InsertItemWithStringIdAt(index, IDC_CONTENT_CONTEXT_OPENLINKNEWTAB,
                                   IDS_VIV_OPEN_LINK_NEW_FOREGROUND_TAB);
    if (!profile->GetPrefs()->GetBoolean(
          vivaldiprefs::kTabsOpenNewInBackground)) {
      menu->InsertItemWithStringIdAt(++index,
                                     IDC_CONTENT_CONTEXT_OPENLINKBACKGROUNDTAB,
                                     IDS_VIV_OPEN_LINK_NEW_BACKGROUND_TAB);
    }
    menu->InsertItemWithStringIdAt(++index, IDC_VIV_OPEN_LINK_CURRENT_TAB,
                                   IDS_VIV_OPEN_LINK_CURRENT_TAB);

    menu->InsertSeparatorAt(++index, ui::NORMAL_SEPARATOR);
    menu->InsertItemWithStringIdAt(++index,
                                   IDC_CONTENT_CONTEXT_OPENLINKNEWWINDOW,
                                   IDS_VIV_OPEN_LINK_NEW_WINDOW);
    if (!profile->IsGuestSession()) {
      menu->InsertItemWithStringIdAt(++index,
                                     IDC_CONTENT_CONTEXT_OPENLINKOFFTHERECORD,
                                     IDS_VIV_OPEN_LINK_NEW_PRIVATE_WINDOW);
    }
    if (!IsVivaldiWebPanel(web_contents)) {
      menu->InsertSeparatorAt(++index, ui::NORMAL_SEPARATOR);
      if (!profile->IsGuestSession()) {
        menu->InsertItemWithStringIdAt(++index, IDC_VIV_BOOKMARK_LINK,
                                       IDS_VIV_BOOKMARK_LINK);
      }
      menu->InsertItemWithStringIdAt(++index, IDC_VIV_ADD_LINK_TO_WEBPANEL,
                                     IDS_VIV_ADD_LINK_TO_WEBPANEL);
      if (vivaldi_runtime_feature::IsEnabled(profile, "calendar_mail_feeds")) {
        // Text under context menu is always auto selected on mac so we can not
        // test for selected text to determine behavior in that OS.
#if !defined(OS_MAC)
        if (params.selection_text.empty()) {
#endif
          menu->InsertItemWithStringIdAt(++index, IDC_VIV_SEND_LINK_BY_MAIL,
                                         IDS_VIV_SEND_LINK_BY_MAIL);
#if !defined(OS_MAC)
        }
#endif
      }
    }
  }
}

void VivaldiAddImageItems(SimpleMenuModel* menu,
                          WebContents* web_contents,
                          const ContextMenuParams& params) {
  if (IsVivaldiRunning()) {
    Profile* profile =
        Profile::FromBrowserContext(web_contents->GetBrowserContext());
    int index = menu->GetIndexOfCommandId(IDC_CONTENT_CONTEXT_OPENIMAGENEWTAB);
    DCHECK_GE(index, 0);
    menu->RemoveItemAt(index);

    menu->InsertItemWithStringIdAt(index, IDC_VIV_OPEN_IMAGE_NEW_FOREGROUND_TAB,
                                   IDS_VIV_OPEN_IMAGE_NEW_FOREGROUND_TAB);
    if (!profile->GetPrefs()->GetBoolean(
          vivaldiprefs::kTabsOpenNewInBackground)) {
      menu->InsertItemWithStringIdAt(++index,
                                     IDC_VIV_OPEN_IMAGE_NEW_BACKGROUND_TAB,
                                     IDS_VIV_OPEN_IMAGE_NEW_BACKGROUND_TAB);
    }
    menu->InsertItemWithStringIdAt(++index, IDC_VIV_OPEN_IMAGE_CURRENT_TAB,
                                   IDS_VIV_OPEN_IMAGE_CURRENT_TAB);
    menu->InsertSeparatorAt(++index, ui::NORMAL_SEPARATOR);
    menu->InsertItemWithStringIdAt(++index, IDC_VIV_OPEN_IMAGE_NEW_WINDOW,
                                   IDS_VIV_OPEN_IMAGE_NEW_WINDOW);
    if (!profile->IsGuestSession()) {
      menu->InsertItemWithStringIdAt(++index,
                                     IDC_VIV_OPEN_IMAGE_NEW_PRIVATE_WINDOW,
                                     IDS_VIV_OPEN_IMAGE_NEW_PRIVATE_WINDOW);
    }
    menu->InsertSeparatorAt(++index, ui::NORMAL_SEPARATOR);

    index = menu->GetIndexOfCommandId(IDC_CONTENT_CONTEXT_COPYIMAGELOCATION);
    DCHECK_GE(index, 0);
    menu->InsertSeparatorAt(++index, ui::NORMAL_SEPARATOR);
    if (!IsVivaldiWebPanel(web_contents)) {
      menu->InsertItemWithStringIdAt(++index, IDC_VIV_INSPECT_IMAGE,
                                     IDS_VIV_INSPECT_IMAGE);
      if (!profile->IsGuestSession()) {
        menu->InsertItemWithStringIdAt(++index, IDC_VIV_USE_IMAGE_AS_BACKGROUND,
                                      IDS_VIV_USE_IMAGE_AS_BACKGROUND);
      }
    }
    menu->InsertItemWithStringIdAt(++index, IDC_CONTENT_CONTEXT_RELOADIMAGE,
                                   IDS_CONTENT_CONTEXT_RELOADIMAGE);
  }
}

void VivaldiAddCopyItems(SimpleMenuModel* menu,
                         WebContents* web_contents,
                         const ContextMenuParams& params) {
  Profile* profile =
      Profile::FromBrowserContext(web_contents->GetBrowserContext());
  if (IsVivaldiRunning() && WebViewGuest::FromWebContents(web_contents) &&
      !IsVivaldiWebPanel(web_contents) && !profile->IsGuestSession()) {
    menu->AddItemWithStringId(IDC_VIV_COPY_TO_NOTE, IDS_VIV_COPY_TO_NOTE);
    if (vivaldi_runtime_feature::IsEnabled(profile, "calendar_mail_feeds")) {
      menu->AddItemWithStringId(IDC_VIV_SEND_SELECTION_BY_MAIL,
                                IDS_VIV_SEND_BY_MAIL);
      menu->AddItemWithStringId(IDC_VIV_ADD_AS_EVENT, IDS_VIV_ADD_AS_EVENT);
    }
  }
}

void VivaldiAddPageItems(SimpleMenuModel* menu,
                         WebContents* web_contents,
                         const ContextMenuParams& params) {
  Profile* profile =
      Profile::FromBrowserContext(web_contents->GetBrowserContext());
  if (IsVivaldiRunning() && WebViewGuest::FromWebContents(web_contents) &&
      !IsVivaldiWebPanel(web_contents) && !IsVivaldiMail(web_contents)) {
    int index = menu->GetIndexOfCommandId(IDC_SAVE_PAGE);
    DCHECK_GE(index, 0);
    if (!profile->IsGuestSession()) {
      menu->InsertItemWithStringIdAt(index, IDC_VIV_BOOKMARK_PAGE,
                                     IDS_VIV_BOOKMARK_PAGE);
    }
    menu->InsertItemWithStringIdAt(++index, IDC_VIV_ADD_PAGE_TO_WEBPANEL,
                                   IDS_VIV_ADD_PAGE_TO_WEBPANEL);
    if (vivaldi_runtime_feature::IsEnabled(profile, "calendar_mail_feeds")) {
      menu->InsertItemWithStringIdAt(++index, IDC_VIV_SEND_PAGE_BY_MAIL,
                                    IDS_VIV_SEND_BY_MAIL);
    }
    menu->InsertSeparatorAt(++index, ui::NORMAL_SEPARATOR);
    if (media_router::MediaRouterEnabled(web_contents->GetBrowserContext())) {
      index = menu->GetIndexOfCommandId(IDC_ROUTE_MEDIA);
    } else {
      index = menu->GetIndexOfCommandId(IDC_PRINT);
    }
    DCHECK_GE(index, 0);
    menu->InsertItemWithStringIdAt(++index, IDC_VIV_COPY_PAGE_ADDRESS,
                                   IDS_VIV_COPY_PAGE_ADDRESS);
  } else if (IsVivaldiWebPanel(web_contents)) {
    // Casting is only available from tab-contents.
    if (media_router::MediaRouterEnabled(web_contents->GetBrowserContext())) {
      int index = menu->GetIndexOfCommandId(IDC_ROUTE_MEDIA);
      DCHECK_GE(index, 0);
      menu->RemoveItemAt(index);
    }
  }
}

void VivaldiAddEditableItems(SimpleMenuModel* menu,
                             WebContents* web_contents,
                             const ContextMenuParams& params) {
  if (IsVivaldiRunning()) {
    const bool use_paste_and_match_style =
        params.input_field_type !=
            blink::ContextMenuDataInputFieldType::kPlainText &&
        params.input_field_type !=
            blink::ContextMenuDataInputFieldType::kPassword;

    int index = menu->GetIndexOfCommandId(IDC_CONTENT_CONTEXT_PASTE);
    DCHECK_GE(index, 0);

    if (params.vivaldi_input_type == "vivaldi-addressfield" ||
        params.vivaldi_input_type == "vivaldi-searchfield") {
      menu->InsertItemWithStringIdAt(++index, IDC_CONTENT_CONTEXT_PASTE_AND_GO,
                                     IDS_CONTENT_CONTEXT_PASTE_AND_GO);
    }

    index =
        menu->GetIndexOfCommandId(IDC_CONTENT_CONTEXT_PASTE_AND_MATCH_STYLE);
    if (use_paste_and_match_style) {
      DCHECK_GE(index, 0);
      menu->InsertSeparatorAt(++index, ui::NORMAL_SEPARATOR);
    } else if (params.misspelled_word.empty()) {
      DCHECK_GE(index, 0);
      menu->RemoveItemAt(index);
      menu->InsertSeparatorAt(index, ui::NORMAL_SEPARATOR);
    }

    if (!params.vivaldi_keyword_url.is_empty()) {
      if (params.misspelled_word.empty()) {
        index = menu->GetIndexOfCommandId(IDC_CONTENT_CONTEXT_SELECTALL);
      } else {
        index = menu->GetIndexOfCommandId(IDC_CONTENT_CONTEXT_PASTE);
      }

      DCHECK_GE(index, 0);

      Profile* profile =
        Profile::FromBrowserContext(web_contents->GetBrowserContext());
      if (!profile->IsGuestSession()) {
        menu->InsertSeparatorAt(++index, ui::NORMAL_SEPARATOR);
        menu->InsertItemWithStringIdAt(++index,
          IDC_VIV_CONTENT_CONTEXT_ADDSEARCHENGINE,
          IDS_VIV_CONTENT_CONTEXT_ADDSEARCHENGINE);
      }
    }
  }
}

void VivaldiAddDeveloperItems(SimpleMenuModel* menu,
                              WebContents* web_contents,
                              const ContextMenuParams& params) {
  if (IsVivaldiRunning() && WebViewGuest::FromWebContents(web_contents) &&
      !IsVivaldiWebPanel(web_contents) && !IsVivaldiMail(web_contents)) {
    if (params.src_url.is_empty() && params.link_url.is_empty()) {
      int index = menu->GetIndexOfCommandId(IDC_VIEW_SOURCE);
      if (index == -1) {
        index = menu->GetIndexOfCommandId(IDC_CONTENT_CONTEXT_INSPECTELEMENT);
      }
      DCHECK_GE(index, 0);
      menu->InsertItemWithStringIdAt(index, IDC_VIV_VALIDATE_PAGE,
                                     IDS_VIV_VALIDATE_PAGE);
    }
  }
}

void VivaldiAddFullscreenItems(SimpleMenuModel* menu,
                               WebContents* web_contents,
                               const ContextMenuParams& params) {
  if (IsVivaldiRunning() && WebViewGuest::FromWebContents(web_contents) &&
      !IsVivaldiWebPanel(web_contents) && !IsVivaldiMail(web_contents)) {
    menu->AddSeparator(ui::NORMAL_SEPARATOR);
    menu->AddItemWithStringId(IDC_VIV_FULLSCREEN, IDS_VIV_FULLSCREEN);
    menu->AddSeparator(ui::NORMAL_SEPARATOR);
  }
}

bool IsVivaldiCommandIdEnabled(const SimpleMenuModel& menu,
                               const ContextMenuParams& params,
                               int id,
                               bool* enabled) {
  switch (id) {
    default:
      return false;

    case IDC_VIV_OPEN_LINK_CURRENT_TAB:
    case IDC_CONTENT_CONTEXT_OPENLINKBACKGROUNDTAB:
      *enabled = params.link_url.is_valid();
      break;
    case IDC_CONTENT_CONTEXT_RELOADIMAGE:
    case IDC_VIV_OPEN_IMAGE_CURRENT_TAB:
    case IDC_VIV_OPEN_IMAGE_NEW_FOREGROUND_TAB:
    case IDC_VIV_OPEN_IMAGE_NEW_BACKGROUND_TAB:
    case IDC_VIV_OPEN_IMAGE_NEW_WINDOW:
    case IDC_VIV_OPEN_IMAGE_NEW_PRIVATE_WINDOW:
      *enabled = params.media_type ==
                 blink::ContextMenuDataMediaType::kImage;
      break;
    case IDC_CONTENT_CONTEXT_PASTE_AND_GO: {
      if (!(params.edit_flags & blink::ContextMenuDataEditFlags::kCanPaste)) {
        *enabled = false;
        break;
      }

      std::vector<base::string16> types;
      ui::Clipboard::GetForCurrentThread()->ReadAvailableTypes(
          ui::ClipboardBuffer::kCopyPaste, /* data_dst = */ nullptr, &types);
      *enabled = !types.empty();
      break;
    }

    case IDC_VIV_CONTENT_CONTEXT_ADDSEARCHENGINE:
      *enabled = !params.vivaldi_keyword_url.is_empty();
      break;

    case IDC_VIV_COPY_TO_NOTE:
      *enabled = !params.selection_text.empty();
      break;

    case IDC_VIV_ADD_AS_EVENT:
      *enabled = !params.selection_text.empty();
      break;

    case IDC_VIV_BOOKMARK_PAGE:
    case IDC_VIV_BOOKMARK_LINK:
    case IDC_VIV_ADD_PAGE_TO_WEBPANEL:
    case IDC_VIV_ADD_LINK_TO_WEBPANEL:
    case IDC_VIV_SEND_LINK_BY_MAIL:
    case IDC_VIV_SEND_PAGE_BY_MAIL:
    case IDC_VIV_SEND_SELECTION_BY_MAIL:
    case IDC_VIV_COPY_PAGE_ADDRESS:
    case IDC_VIV_INSPECT_IMAGE:
    case IDC_VIV_USE_IMAGE_AS_BACKGROUND:
    case IDC_VIV_VALIDATE_PAGE:
    case IDC_VIV_FULLSCREEN:
      *enabled = true;
      break;
  }
  return true;
}

void OnUseLocalImageAsBackground(content::WebContents* web_contents,
                                 int event_flags,
                                 const GURL& src_url) {
  // PathExists() triggers IO restriction.
  base::ThreadRestrictions::ScopedAllowIO allow_io;

  // Strip any url encoding.
  std::string filename = net::UnescapeURLComponent(
      src_url.GetContent(),
      net::UnescapeRule::NORMAL | net::UnescapeRule::SPACES |
          net::UnescapeRule::REPLACE_PLUS_WITH_SPACE |
          net::UnescapeRule::URL_SPECIAL_CHARS_EXCEPT_PATH_SEPARATORS);
#if defined(OS_POSIX)
  base::FilePath path(filename);
#elif defined(OS_WIN)
  base::FilePath path(base::UTF8ToWide(filename));
#endif
  if (!base::PathExists(path)) {
    if (filename[0] == '/') {
      // It might be in a format /D:/somefile.png so strip the first
      // slash.
      filename = filename.substr(1, std::string::npos);
#if defined(OS_POSIX)
      path = base::FilePath(filename);
#elif defined(OS_WIN)
      path = base::FilePath(base::UTF8ToWide(filename));
#endif
    }
  }
  // Call UpdateMapping and send a notification to JS to switch to use the
  // custom background image in parallel. In the very unlikely case when the
  // former will access the custom image preference before we set its URL in the
  // UpdateMapping callback the user may briefly see the older background.
  //
  // If we will need to ensure that useLocalImageAsBackground will always be
  // called after UpdateMapping calls our callback, we will need a weak pointer
  // for WebContents, which is messy until https://crbug.com/952390 is
  // resolved.
  VivaldiDataSourcesAPI::UpdateMapping(
      web_contents->GetBrowserContext(), 0,
      VivaldiDataSourcesAPI::kStartpageImagePathCustom_Index, std::move(path),
      base::DoNothing());
  SendSimpleAction(web_contents, event_flags, "useLocalImageAsBackground");
}

WindowOpenDisposition VivaldiGetNewTabDispostion(WebContents* web_contents) {
  Profile* profile =
      Profile::FromBrowserContext(web_contents->GetBrowserContext());
  bool open_in_background =
      profile->GetPrefs()->GetBoolean(vivaldiprefs::kTabsOpenNewInBackground);
  return open_in_background ? WindowOpenDisposition::NEW_BACKGROUND_TAB :
      WindowOpenDisposition::NEW_FOREGROUND_TAB;
}

bool VivaldiExecuteCommand(RenderViewContextMenu* context_menu,
                           const ContextMenuParams& params,
                           WebContents* source_web_contents,
                           int event_flags,
                           int id,
                           const OpenURLCall& openurl) {
  switch (id) {
    default:
      return false;

    case IDC_VIV_OPEN_LINK_CURRENT_TAB:
      openurl.Run(params.link_url, GetDocumentURL(params),
                  WindowOpenDisposition::CURRENT_TAB, ui::PAGE_TRANSITION_LINK);
      break;
    case IDC_CONTENT_CONTEXT_OPENLINKBACKGROUNDTAB:
      openurl.Run(params.link_url, GetDocumentURL(params),
                  WindowOpenDisposition::NEW_BACKGROUND_TAB,
                  ui::PAGE_TRANSITION_LINK);
      break;
    case IDC_CONTENT_CONTEXT_RELOADIMAGE: {
      // params.x and params.y position the context menu and are always in root
      // coordinates. Convert to content coordinates.
      gfx::PointF p = source_web_contents->GetRenderViewHost()
                          ->GetWidget()
                          ->GetView()
                          ->TransformRootPointToViewCoordSpace(
                              gfx::PointF(params.x, params.y));
      source_web_contents->GetRenderViewHost()->LoadImageAt(
          static_cast<int>(p.x()), static_cast<int>(p.y()));
      break;
    }
    case IDC_VIV_OPEN_IMAGE_CURRENT_TAB:
      openurl.Run(params.src_url, GetDocumentURL(params),
                  WindowOpenDisposition::CURRENT_TAB, ui::PAGE_TRANSITION_LINK);
      break;
    case IDC_VIV_OPEN_IMAGE_NEW_FOREGROUND_TAB:
      openurl.Run(params.src_url, GetDocumentURL(params),
                  VivaldiGetNewTabDispostion(source_web_contents),
                  ui::PAGE_TRANSITION_LINK);
      break;
    case IDC_VIV_OPEN_IMAGE_NEW_BACKGROUND_TAB:
      openurl.Run(params.src_url, GetDocumentURL(params),
                  WindowOpenDisposition::NEW_BACKGROUND_TAB,
                  ui::PAGE_TRANSITION_LINK);
      break;
    case IDC_VIV_OPEN_IMAGE_NEW_WINDOW:
      openurl.Run(params.src_url, GetDocumentURL(params),
                  WindowOpenDisposition::NEW_WINDOW, ui::PAGE_TRANSITION_LINK);
      break;
    case IDC_VIV_OPEN_IMAGE_NEW_PRIVATE_WINDOW:
      openurl.Run(params.src_url, GetDocumentURL(params),
                  WindowOpenDisposition::OFF_THE_RECORD,
                  ui::PAGE_TRANSITION_LINK);
      break;
    case IDC_CONTENT_CONTEXT_PASTE_AND_GO:
      if (IsVivaldiRunning()) {
        base::string16 text;
        ui::Clipboard::GetForCurrentThread()->ReadText(
            ui::ClipboardBuffer::kCopyPaste, /* data_dst = */ nullptr, &text);
        std::string target;
        if (params.vivaldi_input_type == "vivaldi-addressfield")
          target = "url";
        else if (params.vivaldi_input_type == "vivaldi-searchfield")
          target = "search";
        std::vector<base::Value> args;
        args.emplace_back(text);
        args.emplace_back(std::move(target));
        args.emplace_back(GetModifierStringFromEventFlags(event_flags));
        extensions::WebViewGuest* current_webviewguest =
            vivaldi::ui_tools::GetActiveWebViewGuest();
        if (current_webviewguest) {
          current_webviewguest->PasteAndGo(base::ListValue(std::move(args)));
        }
      }
      break;

    case IDC_VIV_CONTENT_CONTEXT_ADDSEARCHENGINE: {
      base::string16 keyword(TemplateURL::GenerateKeyword(params.page_url));
      WebViewGuest* vivGuestView =
          WebViewGuest::FromWebContents(source_web_contents);
      if (vivGuestView) {
        std::vector<base::Value> args;
        args.emplace_back(keyword);
        args.emplace_back(params.vivaldi_keyword_url.spec());
        vivGuestView->CreateSearch(base::ListValue(std::move(args)));
      }
    } break;

    case IDC_VIV_COPY_TO_NOTE:
      if (IsVivaldiRunning()) {
        SendSimpleAction(source_web_contents, event_flags, "copyToNote",
                         s_selection_text, params.page_url.spec());
      }
      break;

    case IDC_VIV_ADD_AS_EVENT:
      if (IsVivaldiRunning()) {
        SendSimpleAction(source_web_contents, event_flags, "addAsEvent",
                         s_selection_text, params.page_url.spec());
      }
      break;

    case IDC_VIV_BOOKMARK_PAGE:
      if (IsVivaldiRunning()) {
        SendSimpleAction(source_web_contents, event_flags,
                         "addActivePageToBookmarks");
      }
      break;

    case IDC_VIV_BOOKMARK_LINK:
      if (IsVivaldiRunning()) {
        SendSimpleAction(source_web_contents, event_flags, "addUrlToBookmarks",
                         params.link_text, params.link_url.spec());
      }
      break;

    case IDC_VIV_ADD_PAGE_TO_WEBPANEL:
    case IDC_VIV_ADD_LINK_TO_WEBPANEL:
      if (IsVivaldiRunning()) {
        SendSimpleAction(
            source_web_contents, event_flags, "addUrlToWebPanel", "",
            id == IDC_VIV_ADD_PAGE_TO_WEBPANEL ? params.page_url.spec()
                                               : params.link_url.spec());
      }
      break;

    case IDC_VIV_SEND_PAGE_BY_MAIL:
    case IDC_VIV_SEND_LINK_BY_MAIL:
      if (IsVivaldiRunning()) {
        SendSimpleAction(
            source_web_contents, event_flags, "sendLinkByMail",
            id == IDC_VIV_SEND_PAGE_BY_MAIL ? source_web_contents->GetTitle()
                                            : params.link_text,
            id == IDC_VIV_SEND_PAGE_BY_MAIL ? params.page_url.spec()
                                            : params.link_url.spec());
      }
      break;

    case IDC_VIV_SEND_SELECTION_BY_MAIL:
      if (IsVivaldiRunning()) {
        SendSimpleAction(source_web_contents, event_flags,
                         "sendSelectionByMail", params.selection_text,
                         base::UTF16ToUTF8(source_web_contents->GetTitle()));
      }
      break;

    case IDC_VIV_COPY_PAGE_ADDRESS:
      if (IsVivaldiRunning()) {
        SendSimpleAction(source_web_contents, event_flags, "copyUrlToClipboard",
                         "", params.page_url.spec());
      }
      break;

    case IDC_VIV_INSPECT_IMAGE:
      if (IsVivaldiRunning()) {
        SendSimpleAction(source_web_contents, event_flags, "inspectImageUrl",
                         "", params.src_url.spec());
      }
      break;

    case IDC_VIV_USE_IMAGE_AS_BACKGROUND:
      if (IsVivaldiRunning()) {
        // The app does not have access to file:// schemes, so handle it
        // differently.
        if (params.src_url.SchemeIs(url::kFileScheme)) {
          OnUseLocalImageAsBackground(source_web_contents, event_flags,
                                      params.src_url);
        } else {
          SendSimpleAction(source_web_contents, event_flags,
                           "useImageAsBackground", "",
                           params.src_url.spec());
        }
      }
      break;

    case IDC_VIV_VALIDATE_PAGE:
      if (IsVivaldiRunning()) {
        SendSimpleAction(source_web_contents, event_flags, "validateUrl",
                         "", params.page_url.spec());
      }
      break;

    case IDC_VIV_FULLSCREEN:
      SendSimpleAction(source_web_contents, event_flags, "fullscreen");
      break;

    case IDC_RELOAD:
      if (IsVivaldiRunning()) {
        extensions::WebViewGuest* web_view_guest =
          extensions::WebViewGuest::FromWebContents(source_web_contents);
        if (web_view_guest && web_view_guest->IsVivaldiWebPanel()) {
           web_view_guest->Reload();
           break;
        }
      }
      return false;
      break;
  }
  return true;
}

// Returns true if an accelerator has been asigned. The accelerator can be empty
// meaning we do not want an accelerator.
bool VivaldiGetAcceleratorForCommandId(const RenderViewContextMenuViews* view,
                                       int command_id,
                                       ui::Accelerator* accel) {
  // TODO(espen@vivaldi.com): We can not get away with hardcoded shortcuts in
  // in menus in chromium code as long as we do not have synchronous access to
  // our settings. We can either turn them off or let them be the same as as the
  // default values in vivaldi.
  if (!IsVivaldiRunning())
    return false;

  switch (command_id) {
    case IDC_BACK:
      *accel = ui::Accelerator(ui::VKEY_LEFT, ui::EF_ALT_DOWN);
      return true;
    case IDC_CONTENT_CONTEXT_UNDO:
      *accel = ui::Accelerator(ui::VKEY_Z, ui::EF_CONTROL_DOWN);
      return true;
    case IDC_CONTENT_CONTEXT_REDO:
      *accel = ui::Accelerator(ui::VKEY_Y, ui::EF_CONTROL_DOWN);
      return true;
    case IDC_CONTENT_CONTEXT_CUT:
      *accel = ui::Accelerator(ui::VKEY_X, ui::EF_CONTROL_DOWN);
      return true;
    case IDC_CONTENT_CONTEXT_COPY:
      *accel = ui::Accelerator(ui::VKEY_C, ui::EF_CONTROL_DOWN);
      return true;
    case IDC_CONTENT_CONTEXT_PASTE:
      *accel = ui::Accelerator(ui::VKEY_V, ui::EF_CONTROL_DOWN);
      return true;
    case IDC_CONTENT_CONTEXT_INSPECTELEMENT:
      *accel =
          ui::Accelerator(ui::VKEY_I, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN);
      return true;
    case IDC_CONTENT_CONTEXT_PASTE_AND_MATCH_STYLE:
      *accel = ui::Accelerator();
      return true;
    case IDC_CONTENT_CONTEXT_PASTE_AND_GO:
      *accel =
          ui::Accelerator(ui::VKEY_V, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN);
      return true;
    case IDC_CONTENT_CONTEXT_SELECTALL:
      *accel = ui::Accelerator(ui::VKEY_A, ui::EF_CONTROL_DOWN);
      return true;
    case IDC_CONTENT_CONTEXT_ROTATECCW:
      *accel = ui::Accelerator(ui::VKEY_OEM_4, ui::EF_CONTROL_DOWN);
      return true;
    case IDC_CONTENT_CONTEXT_ROTATECW:
      *accel = ui::Accelerator(ui::VKEY_OEM_6, ui::EF_CONTROL_DOWN);
      return true;
    case IDC_FORWARD:
      *accel = ui::Accelerator(ui::VKEY_RIGHT, ui::EF_ALT_DOWN);
      return true;
    case IDC_PRINT:
      *accel = ui::Accelerator(ui::VKEY_P, ui::EF_CONTROL_DOWN);
      return true;
    case IDC_RELOAD:
      *accel = ui::Accelerator(ui::VKEY_R, ui::EF_CONTROL_DOWN);
      return true;
    case IDC_CONTENT_CONTEXT_SAVEAVAS:
    case IDC_SAVE_PAGE:
      *accel = ui::Accelerator(ui::VKEY_S, ui::EF_CONTROL_DOWN);
      return true;
    case IDC_VIEW_SOURCE:
      *accel = ui::Accelerator(ui::VKEY_U, ui::EF_CONTROL_DOWN);
      return true;
    case IDC_VIV_COPY_TO_NOTE:
      *accel =
          ui::Accelerator(ui::VKEY_C, ui::EF_SHIFT_DOWN | ui::EF_CONTROL_DOWN);
      return true;
    case IDC_VIV_BOOKMARK_PAGE:
      *accel = ui::Accelerator(ui::VKEY_D, ui::EF_CONTROL_DOWN);
      return true;
    case IDC_VIV_FULLSCREEN:
      *accel = ui::Accelerator(ui::VKEY_F11, ui::EF_NONE);
      return true;

    default:
      return false;
  }
}

}  // namespace vivaldi
