// Copyright (c) 2015 Vivaldi Technologies AS. All rights reserved

#include "extensions/api/import_data/import_data_api.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "app/vivaldi_resources.h"

#include "base/lazy_instance.h"
#include "base/stl_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/values.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/content_settings/host_content_settings_map_factory.h"
#include "chrome/browser/extensions/api/content_settings/content_settings_api.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/extension_web_ui.h"
#include "chrome/browser/importer/external_process_importer_host.h"
#include "chrome/browser/importer/importer_list.h"
#include "chrome/browser/platform_util.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/chrome_select_file_policy.h"
#include "chrome/browser/ui/webui/settings/settings_utils.h"
#include "chrome/browser/ui/webui/settings/site_settings_helper.h"
#include "chrome/common/importer/importer_data_types.h"
#include "chrome/common/pref_names.h"

#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "components/content_settings/core/common/pref_names.h"
#include "components/prefs/pref_service.h"

#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"

#include "extensions/browser/view_type_utils.h"
#include "extensions/common/error_utils.h"
#include "extensions/common/manifest_handlers/background_info.h"
#include "extensions/common/url_pattern_set.h"

#include "chrome/app/chrome_command_ids.h"
#include "chrome/browser/ui/browser_finder.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/models/simple_menu_model.h"
#include "ui/views/controls/menu/menu_runner.h"

#include "chrome/browser/ui/browser_commands.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"

#include "chrome/browser/prefs/session_startup_pref.h"

#include "content/public/browser/render_view_host.h"
#include "extensions/browser/guest_view/web_view/web_view_guest.h"
#include "extensions/common/extension_messages.h"

#include "extensions/schema/import_data.h"
#include "extensions/tools/vivaldi_tools.h"

#include "components/strings/grit/components_strings.h"
#include "ui/vivaldi_browser_window.h"
#include "ui/vivaldi_ui_utils.h"

class Browser;

namespace extensions {

namespace GetProfiles = vivaldi::import_data::GetProfiles;
using content::WebContents;

// ProfileSingletonFactory
bool ProfileSingletonFactory::instanceFlag = false;
ProfileSingletonFactory* ProfileSingletonFactory::single = NULL;

ProfileSingletonFactory* ProfileSingletonFactory::getInstance() {
  if (!instanceFlag) {
    single = new ProfileSingletonFactory();
    instanceFlag = true;
    return single;
  } else {
    return single;
  }
}

ProfileSingletonFactory::ProfileSingletonFactory() {
  api_importer_list_.reset(new ImporterList());
  profilesRequested = false;
}

ProfileSingletonFactory::~ProfileSingletonFactory() {
  instanceFlag = false;
  profilesRequested = false;
}

ImporterList* ProfileSingletonFactory::getImporterList() {
  return single->api_importer_list_.get();
}

void ProfileSingletonFactory::setProfileRequested(bool profileReq) {
  profilesRequested = profileReq;
}

bool ProfileSingletonFactory::getProfileRequested() {
  return profilesRequested;
}

namespace {

// These are really flags, but never sent as flags.
static struct ImportItemToStringMapping {
  importer::ImportItem item;
  const char* name;
} import_item_string_mapping[]{
    // clang-format off
    {importer::FAVORITES, "favorites"},
    {importer::PASSWORDS, "passwords"},
    {importer::HISTORY, "history"},
    {importer::COOKIES, "cookies"},
    {importer::SEARCH_ENGINES, "search"},
    {importer::NOTES, "notes"},
    {importer::SPEED_DIAL, "speeddial"},
    // clang-format on
};

const size_t kImportItemToStringMappingLength =
    base::size(import_item_string_mapping);

const std::string ImportItemToString(importer::ImportItem item) {
  for (size_t i = 0; i < kImportItemToStringMappingLength; i++) {
    if (item == import_item_string_mapping[i].item) {
      return import_item_string_mapping[i].name;
    }
  }
  // Missing datatype in the array?
  NOTREACHED();
  return nullptr;
}
}  // namespace

ImportDataAPI::ImportDataAPI(content::BrowserContext* context)
    : browser_context_(context),
      importer_host_(nullptr),
      import_succeeded_count_(0) {}

ImportDataAPI::~ImportDataAPI() {}

void ImportDataAPI::StartImport(const importer::SourceProfile& source_profile,
                                uint16_t imported_items) {
  if (!imported_items)
    return;

  import_succeeded_count_ = 0;

  DCHECK(!importer_host_);

  // If another import is already ongoing, let it finish silently.
  if (importer_host_)
    importer_host_->set_observer(nullptr);

  base::Value importing(true);

  importer_host_ = new ExternalProcessImporterHost();
  importer_host_->set_observer(this);

  Profile* profile = Profile::FromBrowserContext(browser_context_);

  importer_host_->StartImportSettings(
      source_profile, profile, imported_items,
      base::MakeRefCounted<ProfileWriter>(profile).get());
}

void ImportDataAPI::ImportStarted() {
  ::vivaldi::BroadcastEvent(vivaldi::import_data::OnImportStarted::kEventName,
                            vivaldi::import_data::OnImportStarted::Create(),
                            browser_context_);
}

void ImportDataAPI::ImportItemStarted(importer::ImportItem item) {
  import_succeeded_count_++;
  const std::string item_name = ImportItemToString(item);

  ::vivaldi::BroadcastEvent(
      vivaldi::import_data::OnImportItemStarted::kEventName,
      vivaldi::import_data::OnImportItemStarted::Create(item_name),
      browser_context_);
}

void ImportDataAPI::ImportItemEnded(importer::ImportItem item) {
  import_succeeded_count_--;
  const std::string item_name = ImportItemToString(item);

  ::vivaldi::BroadcastEvent(
      vivaldi::import_data::OnImportItemEnded::kEventName,
      vivaldi::import_data::OnImportItemEnded::Create(item_name),
      browser_context_);
}
void ImportDataAPI::ImportItemFailed(importer::ImportItem item,
                                     const std::string& error) {
  // Ensure we get an error at the end.
  import_succeeded_count_++;
  const std::string item_name = ImportItemToString(item);

  ::vivaldi::BroadcastEvent(
      vivaldi::import_data::OnImportItemFailed::kEventName,
      vivaldi::import_data::OnImportItemFailed::Create(item_name, error),
      browser_context_);
}

void ImportDataAPI::ImportEnded() {
  importer_host_->set_observer(nullptr);
  importer_host_ = nullptr;

  ::vivaldi::BroadcastEvent(
      vivaldi::import_data::OnImportEnded::kEventName,
      vivaldi::import_data::OnImportEnded::Create(import_succeeded_count_),
      browser_context_);
}

static base::LazyInstance<BrowserContextKeyedAPIFactory<ImportDataAPI>>::
    DestructorAtExit g_factory_import = LAZY_INSTANCE_INITIALIZER;

// static
BrowserContextKeyedAPIFactory<ImportDataAPI>*
ImportDataAPI::GetFactoryInstance() {
  return g_factory_import.Pointer();
}

ImportDataGetProfilesFunction::ImportDataGetProfilesFunction() {}

ImportDataGetProfilesFunction::~ImportDataGetProfilesFunction() {}

void ImportDataGetProfilesFunction::Finished() {
  namespace Results = vivaldi::import_data::GetProfiles::Results;

  std::vector<vivaldi::import_data::ProfileItem> nodes;
  for (size_t i = 0; i < api_importer_list_->count(); ++i) {
    const importer::SourceProfile& source_profile =
        api_importer_list_->GetSourceProfileAt(i);

    nodes.emplace_back();
    vivaldi::import_data::ProfileItem* profile = &nodes.back();

    uint16_t browser_services = source_profile.services_supported;

    profile->name = base::UTF16ToUTF8(source_profile.importer_name);
    profile->index = i;
    if (source_profile.profile.length() > 0) {
      // NOTE(tomas@vivaldi.com): VB-76639 To distinguish firefox profiles
      profile->name += " " + base::UTF16ToUTF8(source_profile.profile);
    }

    profile->history = ((browser_services & importer::HISTORY) != 0);
    profile->favorites = ((browser_services & importer::FAVORITES) != 0);
    profile->passwords = ((browser_services & importer::PASSWORDS) != 0);
    profile->supports_master_password =
        ((browser_services & importer::MASTER_PASSWORD) != 0);
    profile->search = ((browser_services & importer::SEARCH_ENGINES) != 0);
    profile->notes = ((browser_services & importer::NOTES) != 0);
    profile->speeddial = ((browser_services & importer::SPEED_DIAL) != 0);
    profile->email = ((browser_services & importer::EMAIL) != 0);

    profile->supports_standalone_import =
        (source_profile.importer_type == importer::TYPE_OPERA ||
         source_profile.importer_type == importer::TYPE_VIVALDI ||
         source_profile.importer_type == importer::TYPE_EDGE_CHROMIUM ||
         source_profile.importer_type == importer::TYPE_BRAVE);

    profile->profile_path =
#if defined(OS_WIN)
        base::WideToUTF8(source_profile.source_path.value());
#else
        source_profile.source_path.value();
#endif  // defined(OS_WIN)

    profile->mail_path =
#if defined(OS_WIN)
        base::WideToUTF8(source_profile.mail_path.value());
#else
        source_profile.mail_path.value();
#endif  // defined(OS_WIN)
    profile->has_default_install = !source_profile.source_path.empty();

    if (profile->supports_standalone_import) {
      profile->will_show_dialog_type = "folder";
    } else if (source_profile.importer_type ==
                   importer::TYPE_OPERA_BOOKMARK_FILE ||
               source_profile.importer_type == importer::TYPE_BOOKMARKS_FILE) {
      profile->will_show_dialog_type = "file";
    } else {
      profile->will_show_dialog_type = "none";
    }

    std::vector<vivaldi::import_data::UserProfileItem> profileItems;

    for (size_t i = 0; i < source_profile.user_profile_names.size(); ++i) {
      profileItems.emplace_back();
      vivaldi::import_data::UserProfileItem* profItem = &profileItems.back();

      profItem->profile_display_name = base::UTF16ToUTF8(
          source_profile.user_profile_names.at(i).profileDisplayName);
      profItem->profile_name =
          source_profile.user_profile_names.at(i).profileName;
    }

    profile->user_profiles = std::move(profileItems);
  }

  Respond(ArgumentList(Results::Create(nodes)));
}

ExtensionFunction::ResponseAction ImportDataGetProfilesFunction::Run() {
  ProfileSingletonFactory* singl = ProfileSingletonFactory::getInstance();
  api_importer_list_ = singl->getInstance()->getImporterList();

  // if (!singl->getProfileRequested() &&
  // !api_importer_list->count()){
  singl->setProfileRequested(true);
  api_importer_list_->DetectSourceProfiles(
      g_browser_process->GetApplicationLocale(), true,
      base::BindOnce(&ImportDataGetProfilesFunction::Finished, this));
  return RespondLater();
  // }
}

ImportDataStartImportFunction::ImportDataStartImportFunction() {}

ImportDataStartImportFunction::~ImportDataStartImportFunction() {
  if (select_file_dialog_)
    select_file_dialog_->ListenerDestroyed();
}

// ExtensionFunction:
ExtensionFunction::ResponseAction ImportDataStartImportFunction::Run() {
  using vivaldi::import_data::StartImport::Params;
  namespace Results = vivaldi::import_data::StartImport::Results;

  std::unique_ptr<Params> params = Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params.get());

  std::vector<std::string>& ids = params->items_to_import;
  size_t count = ids.size();
  if (count < 9) {
    return RespondNow(Error("items_to_import must have at least 9 elements"));
  }

  int browser_index;
  if (!base::StringToInt(ids[0], &browser_index)) {
    return RespondNow(Error("items_to_import is not an integer"));
  }

  ImporterList* api_importer_list =
      ProfileSingletonFactory::getInstance()->getInstance()->getImporterList();

  importer::SourceProfile source_profile =
      api_importer_list->GetSourceProfileAt(browser_index);
  int supported_items = source_profile.services_supported;
  int selected_items = importer::NONE;
  importer_type_ = source_profile.importer_type;

  if (ids[1] == "true") {
    selected_items |= importer::HISTORY;
  }
  if (ids[2] == "true") {
    selected_items |= importer::FAVORITES;
  }
  if (ids[3] == "true") {
    selected_items |= importer::PASSWORDS;
  }
  if (ids[4] == "true") {
    selected_items |= importer::SEARCH_ENGINES;
  }
  if (ids[5] == "true") {
    selected_items |= importer::NOTES;
  }
  source_profile.selected_profile_name = ids[7];
  if (params->master_password.get()) {
    source_profile.master_password = *params->master_password;
  }
  if (ids[8] == "true") {
    selected_items |= importer::SPEED_DIAL;
  }

  imported_items_ = (selected_items & supported_items);

  SessionID::id_type window_id = params->window_id;
  std::u16string dialog_title;
  if (importer_type_ == importer::TYPE_BOOKMARKS_FILE) {
    dialog_title =
        l10n_util::GetStringUTF16(IDS_IMPORT_HTML_BOOKMARKS_FILE_TITLE);
    return HandleChooseBookmarksFileOrFolder(window_id, dialog_title, "html",
                                             base::FilePath(), true);
  } else if (importer_type_ == importer::TYPE_OPERA_BOOKMARK_FILE) {
    dialog_title =
        l10n_util::GetStringUTF16(IDS_IMPORT_OPERA_BOOKMARKS_FILE_TITLE);
    base::FilePath path(FILE_PATH_LITERAL("bookmarks.adr"));
    return HandleChooseBookmarksFileOrFolder(window_id, dialog_title, "adr",
                                             path, true);
  } else if ((importer_type_ == importer::TYPE_OPERA ||
              importer_type_ == importer::TYPE_EDGE_CHROMIUM ||
              importer_type_ == importer::TYPE_BRAVE ||
              importer_type_ == importer::TYPE_VIVALDI) &&
             ids[6] == "false") {
    if (importer_type_ == importer::TYPE_OPERA) {
      dialog_title = l10n_util::GetStringUTF16(IDS_IMPORT_OPERA_PROFILE_TITLE);
    } else if (importer_type_ == importer::TYPE_EDGE_CHROMIUM) {
      dialog_title =
          l10n_util::GetStringUTF16(IDS_IMPORT_EDGE_CHROMIUM_PROFILE_TITLE);
    } else if (importer_type_ == importer::TYPE_BRAVE) {
      dialog_title = l10n_util::GetStringUTF16(IDS_IMPORT_BRAVE_PROFILE_TITLE);
    } else if (importer_type_ == importer::TYPE_VIVALDI) {
      dialog_title =
          l10n_util::GetStringUTF16(IDS_IMPORT_VIVALDI_PROFILE_TITLE);
    }
    return HandleChooseBookmarksFileOrFolder(window_id, dialog_title, "ini",
                                             base::FilePath(), false);
  } else {
    if (imported_items_) {
      StartImport(source_profile);
    } else {
      LOG(WARNING) << "There were no settings to import from '"
                   << source_profile.importer_name << "'.";
    }
    return RespondNow(ArgumentList(Results::Create("Ok", "")));
  }
}

ExtensionFunction::ResponseAction
ImportDataStartImportFunction::HandleChooseBookmarksFileOrFolder(
    SessionID::id_type window_id,
    const std::u16string& title,
    base::StringPiece extension,
    const base::FilePath& default_file,
    bool file_selection) {
  VivaldiBrowserWindow* window = VivaldiBrowserWindow::FromId(window_id);
  if (!window) {
    return RespondNow(Error("No such window"));
  }

  AddRef();

  select_file_dialog_ = ui::SelectFileDialog::Create(this, nullptr);

  ui::SelectFileDialog::FileTypeInfo file_type_info;

  if (file_selection) {
    file_type_info.extensions.resize(1);

    file_type_info.extensions[0].push_back(
        base::FilePath::FromUTF8Unsafe(extension).value());
  }
  select_file_dialog_->SelectFile(
      file_selection ? ui::SelectFileDialog::SELECT_OPEN_FILE
                     : ui::SelectFileDialog::SELECT_FOLDER,
      title, default_file, &file_type_info, 0, base::FilePath::StringType(),
      window->GetNativeWindow(), nullptr);
  return RespondLater();
}

void ImportDataStartImportFunction::FileSelectionCanceled(void* params) {
  namespace Results = vivaldi::import_data::StartImport::Results;

  DCHECK(!params);
  Respond(ArgumentList(Results::Create("Cancel", "")));

  Release();  // Balanced in HandleChooseBookmarksFileOrFolder()
}

void ImportDataStartImportFunction::FileSelected(const base::FilePath& path,
                                                 int /*index*/,
                                                 void* params) {
  namespace Results = vivaldi::import_data::StartImport::Results;

  DCHECK(!params);
  importer::SourceProfile source_profile;
  source_profile.source_path = path;
  source_profile.importer_type = importer_type_;

  StartImport(source_profile);
  Respond(ArgumentList(Results::Create("Ok", path.AsUTF8Unsafe())));

  Release();  // Balanced in HandleChooseBookmarksFileOrFolder()
}

void ImportDataStartImportFunction::StartImport(
    const importer::SourceProfile& source_profile) {
  ImportDataAPI::GetFactoryInstance()
      ->Get(browser_context())
      ->StartImport(source_profile, imported_items_);
}

}  // namespace extensions
