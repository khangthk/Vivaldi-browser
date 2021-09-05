// Copyright (c) 2017 Vivaldi Technologies AS. All rights reserved

#include "extensions/api/window/window_private_api.h"

#include <memory>
#include <string>

#include "browser/vivaldi_browser_finder.h"
#include "chrome/browser/chrome_notification_types.h"
#include "chrome/browser/extensions/api/tabs/windows_util.h"
#include "chrome/browser/extensions/browser_extension_window_controller.h"
#include "chrome/browser/extensions/window_controller.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_commands.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_list.h"
#include "components/sessions/content/session_tab_helper.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_frame_host.h"
#include "extensions/api/extension_action_utils/extension_action_utils_api.h"
#include "extensions/api/tabs/tabs_private_api.h"
#include "extensions/api/zoom/zoom_api.h"
#include "extensions/tools/vivaldi_tools.h"
#include "ui/base/ui_base_types.h"
#include "ui/vivaldi_browser_window.h"
#include "ui/vivaldi_ui_utils.h"
#include "vivaldi/prefs/vivaldi_gen_prefs.h"

using extensions::vivaldi::window_private::WindowState;

namespace extensions {

VivaldiWindowsAPI::VivaldiWindowsAPI(content::BrowserContext* context)
    : browser_context_(context) {
  BrowserList::GetInstance()->AddObserver(this);

  registrar_.Add(this, chrome::NOTIFICATION_BROWSER_CLOSE_CANCELLED,
                 content::NotificationService::AllSources());
}

VivaldiWindowsAPI::~VivaldiWindowsAPI() {}

void VivaldiWindowsAPI::Shutdown() {
  BrowserList::GetInstance()->RemoveObserver(this);
}

static base::LazyInstance<BrowserContextKeyedAPIFactory<VivaldiWindowsAPI> >::
    DestructorAtExit g_factory_window = LAZY_INSTANCE_INITIALIZER;

// static
BrowserContextKeyedAPIFactory<VivaldiWindowsAPI>*
VivaldiWindowsAPI::GetFactoryInstance() {
  return g_factory_window.Pointer();
}

void VivaldiWindowsAPI::Observe(
    int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) {
  switch (type) {
    case chrome::NOTIFICATION_BROWSER_CLOSE_CANCELLED: {
      // Browser close was cancelled so notify the client about this
      // fact so it can re-enable chrome tab event.
      Browser* browser = content::Source<Browser>(source).ptr();
      DCHECK(browser);
      ::vivaldi::BroadcastEvent(
          vivaldi::window_private::OnWindowCloseCancelled::kEventName,
          vivaldi::window_private::OnWindowCloseCancelled::Create(
              browser->session_id().id()),
          browser_context_);
      break;
    }
    default: {
      NOTREACHED();
      break;
    }
  }
}

// static
void VivaldiWindowsAPI::WindowsForProfileClosing(Profile* profile) {
  if (profile->IsGuestSession()) {
    // We don't care about guest windows.
    return;
  }
  VivaldiWindowsAPI* api = GetFactoryInstance()->Get(profile);
  DCHECK(api);
  if (!api)
    return;

  for (auto* browser : *BrowserList::GetInstance()) {
    if (browser->profile()->GetOriginalProfile() ==
        profile->GetOriginalProfile())
      api->closing_windows_.push_back(browser);
  }
}

// static
bool VivaldiWindowsAPI::IsWindowClosingBecauseProfileClose(Browser* browser) {
  VivaldiWindowsAPI* api = GetFactoryInstance()->Get(browser->profile());
  DCHECK(api);
  if (!api)
    return false;
  const auto& v = api->closing_windows_;
  return std::find(v.begin(), v.end(), browser) != v.end();
}

void VivaldiWindowsAPI::OnBrowserAdded(Browser* browser) {
  // In Vivaldi we add the ExtensionActionUtil object as an tabstripobserver for
  // each browser. We fetch the correct browser for each update.
  extensions::ExtensionActionUtil* utils =
      extensions::ExtensionActionUtilFactory::GetForBrowserContext(
          browser_context_);

  browser->tab_strip_model()->AddObserver(utils);

  browser->tab_strip_model()->AddObserver(
      TabsPrivateAPI::FromBrowserContext(browser_context_));

  if (browser->is_vivaldi()) {
    ZoomAPI::AddZoomObserver(browser);
  }
  int id = browser->session_id().id();

  ::vivaldi::BroadcastEvent(
      extensions::vivaldi::window_private::OnWindowCreated::kEventName,
      extensions::vivaldi::window_private::OnWindowCreated::Create(id),
      browser_context_);
}

void VivaldiWindowsAPI::OnBrowserRemoved(Browser* browser) {
  // In Vivaldi we add the ExtensionActionUtil object as an tabstripobserver for
  // each browser. We fetch the correct browser for each update.
  extensions::ExtensionActionUtil* utils =
      extensions::ExtensionActionUtilFactory::GetForBrowserContext(
          browser_context_);

  browser->tab_strip_model()->RemoveObserver(utils);

  browser->tab_strip_model()->RemoveObserver(
      TabsPrivateAPI::FromBrowserContext(browser_context_));

  if (browser->is_vivaldi()) {
    ZoomAPI::RemoveZoomObserver(browser);
  }
  for (auto it = closing_windows_.begin(); it != closing_windows_.end(); ++it) {
    if ((*it) == browser) {
      closing_windows_.erase(it);
      break;
    }
  }
  int id = browser->session_id().id();

  ::vivaldi::BroadcastEvent(
      vivaldi::window_private::OnWindowClosed::kEventName,
      vivaldi::window_private::OnWindowClosed::Create(id),
      browser_context_);

  if (chrome::GetTotalBrowserCount() == 1) {
    BrowserList* browsers = BrowserList::GetInstance();
    for (BrowserList::const_iterator iter = browsers->begin();
         iter != browsers->end(); ++iter) {
      const Browser* browser = *iter;
      // If this is the last normal window, close the settings
      // window so shutdown can progress normally.
      if (browser->is_vivaldi() &&
          static_cast<VivaldiBrowserWindow*>(browser->window())->type() ==
              VivaldiBrowserWindow::WindowType::SETTINGS) {
        browser->window()->Close();
        break;
      }
    }
  }
}

ui::WindowShowState ConvertToWindowShowState(WindowState state) {
  switch (state) {
    case WindowState::WINDOW_STATE_NORMAL:
      return ui::SHOW_STATE_NORMAL;
    case WindowState::WINDOW_STATE_MINIMIZED:
      return ui::SHOW_STATE_MINIMIZED;
    case WindowState::WINDOW_STATE_MAXIMIZED:
      return ui::SHOW_STATE_MAXIMIZED;
    case WindowState::WINDOW_STATE_FULLSCREEN:
      return ui::SHOW_STATE_FULLSCREEN;
    case WindowState::WINDOW_STATE_NONE:
      return ui::SHOW_STATE_DEFAULT;
  }
  NOTREACHED();
  return ui::SHOW_STATE_DEFAULT;
}

VivaldiBrowserWindow::WindowType ConvertToVivaldiWindowType(
    vivaldi::window_private::WindowType type) {
  switch (type) {
    case vivaldi::window_private::WindowType::WINDOW_TYPE_NORMAL:
      return VivaldiBrowserWindow::WindowType::NORMAL;
    case vivaldi::window_private::WindowType::WINDOW_TYPE_SETTINGS:
      return VivaldiBrowserWindow::WindowType::SETTINGS;
    case vivaldi::window_private::WindowType::WINDOW_TYPE_NONE:
      return VivaldiBrowserWindow::WindowType::NORMAL;
    case vivaldi::window_private::WindowType::WINDOW_TYPE_POPUP:
      return VivaldiBrowserWindow::WindowType::NORMAL;
  }
  NOTREACHED();
  return VivaldiBrowserWindow::WindowType::NORMAL;
}

ExtensionFunction::ResponseAction WindowPrivateCreateFunction::Run() {
  using vivaldi::window_private::Create::Params;
  namespace Results = vivaldi::window_private::Create::Results;

  std::unique_ptr<Params> params = Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params.get());

  int top = 0;
  int left = 0;
  int min_width = 0;
  int min_height = 0;

  if (params->options.bounds.top.get()) {
    top = *params->options.bounds.top.get();
  }
  if (params->options.bounds.left.get()) {
    left = *params->options.bounds.left.get();
  }
  if (params->options.bounds.min_width.get()) {
    min_width = *params->options.bounds.min_width.get();
  }
  if (params->options.bounds.min_height.get()) {
    min_height= *params->options.bounds.min_height.get();
  }
  bool incognito = false;
  bool focused = true;
  std::string tab_url;
  std::string ext_data;

  if (params->options.incognito.get()) {
    incognito = *params->options.incognito.get();
  }
  if (params->options.focused.get()) {
    focused = *params->options.focused.get();
  }
  if (params->options.tab_url.get()) {
    tab_url = *params->options.tab_url.get();
  }
  if (params->options.ext_data.get()) {
    ext_data = *params->options.ext_data.get();
  }
  Profile* profile = Profile::FromBrowserContext(browser_context());
  if (incognito) {
    profile =
        profile->GetOffTheRecordProfile(Profile::OTRProfileID::PrimaryID());
  } else {
    profile = profile->GetOriginalProfile();
  }
  gfx::Rect bounds(left, top, params->options.bounds.width,
                   params->options.bounds.height);

  // App window specific parameters
  VivaldiBrowserWindowParams window_params;

  window_params.focused = focused;
  if (params->options.window_decoration.get()) {
    window_params.native_decorations = *params->options.window_decoration.get();
  } else {
    window_params.native_decorations = profile->GetPrefs()->GetBoolean(
        vivaldiprefs::kWindowsUseNativeDecoration);
  }
  window_params.content_bounds = bounds;
  window_params.minimum_size = gfx::Size(min_width, min_height);
  window_params.state = ui::SHOW_STATE_DEFAULT;

  if (profile->IsGuestSession() && !incognito) {
    // Opening a new window from a guest session is only allowed for
    // incognito windows.  It will crash on purpose otherwise.
    // See Browser::Browser() for the CHECKs.
    return RespondNow(
        Error("New guest window can only be opened from incognito window"));
  }

  VivaldiBrowserWindow* window = new VivaldiBrowserWindow();
  window->set_type(ConvertToVivaldiWindowType(params->type));

  Browser::CreateParams create_params(Browser::TYPE_POPUP, profile, false);
  create_params.initial_bounds = bounds;
  create_params.is_session_restore = false;
  create_params.is_vivaldi = true;
  create_params.window = window;
  create_params.ext_data = ext_data;
  window->SetBrowser(std::make_unique<Browser>(create_params));

  int window_id = window->browser()->session_id().id();

  window->CreateWebContents(window_params, render_frame_host());

  // This sets up the parameters used in the "create" binding that fills in the
  // contentWindow parameter used in the create-callback.
  content::RenderFrameHost* created_frame =
      window->web_contents()->GetMainFrame();
  std::unique_ptr<base::DictionaryValue> result(new base::DictionaryValue);
  result->SetInteger("frameId", created_frame->GetRoutingID());
  result->SetInteger("windowId", window_id);
  ResponseValue result_arg = OneArgument(std::move(result));
  // Delay sending the response until the newly created window has finished its
  // navigation or was closed during that process.
  window->AddOnDidFinishFirstNavigationCallback(
      base::BindOnce(&WindowPrivateCreateFunction::OnAppUILoaded, this,
                     std::move(result_arg)));

  window->LoadContents(params->url);

  if (!tab_url.empty()) {
    content::OpenURLParams urlparams(GURL(tab_url), content::Referrer(),
      WindowOpenDisposition::NEW_FOREGROUND_TAB,
      ui::PAGE_TRANSITION_AUTO_TOPLEVEL, false);
    window->browser()->OpenURL(urlparams);
  }

  // TODO(pettern): If we ever need to open unfocused windows, we need to
  // add a new method for open delayed and unfocused.
  //  window->Show(focused ? AppWindow::SHOW_ACTIVE : AppWindow::SHOW_INACTIVE);

  return RespondLater();
}

ExtensionFunction::ResponseAction WindowPrivateGetCurrentIdFunction::Run() {
  namespace Results = vivaldi::window_private::GetCurrentId::Results;

  Browser* browser = ::vivaldi::FindBrowserForEmbedderWebContents(
      dispatcher()->GetAssociatedWebContents());
  if (!browser)
    return RespondNow(Error("No Browser instance"));

  return RespondNow(ArgumentList(Results::Create(browser->session_id().id())));
}

ExtensionFunction::ResponseAction WindowPrivateSetStateFunction::Run() {
  using vivaldi::window_private::SetState::Params;

  std::unique_ptr<Params> params = Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params.get());

  Browser* browser;
  std::string error;
  if (!windows_util::GetBrowserFromWindowID(
          this, params->window_id, WindowController::GetAllWindowFilter(),
          &browser, &error)) {
    return RespondNow(Error(error));
  }
  ui::WindowShowState show_state = ConvertToWindowShowState(params->state);

  bool was_fullscreen = browser->window()->IsFullscreen();
  if (show_state != ui::SHOW_STATE_FULLSCREEN &&
      show_state != ui::SHOW_STATE_DEFAULT)
    browser->extension_window_controller()->SetFullscreenMode(
        false, extension()->url());

  switch (show_state) {
    case ui::SHOW_STATE_MINIMIZED:
      browser->extension_window_controller()->window()->Minimize();
      break;
    case ui::SHOW_STATE_MAXIMIZED:
      browser->extension_window_controller()->window()->Maximize();
      break;
    case ui::SHOW_STATE_FULLSCREEN:
      browser->extension_window_controller()->SetFullscreenMode(
          true, extension()->url());
      break;
    case ui::SHOW_STATE_NORMAL:
      if (was_fullscreen) {
        browser->extension_window_controller()->SetFullscreenMode(
            false, extension()->url());
      } else {
        browser->window()->Restore();
      }
      break;
    default:
      break;
  }
  return RespondNow(NoArguments());
}

void WindowPrivateCreateFunction::OnAppUILoaded(ResponseValue result_arg,
                                                bool did_finish) {
  DCHECK(!did_respond());

  if (!did_finish) {
    Respond(Error("window closed before app-doc loaded"));
    return;
  }

  Respond(std::move(result_arg));
}

}  // namespace extensions
