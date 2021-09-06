// Copyright (c) 2013 Vivaldi Technologies AS. All rights reserved

#include "ui/webgui/notes_ui.h"
#include "base/logging.h"
#include "base/memory/ref_counted_memory.h"
#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/web_ui.h"

namespace vivaldi {

NotesUIHTMLSource::NotesUIHTMLSource() {}

std::string NotesUIHTMLSource::GetSource() {
  return "notes";
}

void NotesUIHTMLSource::StartDataRequest(
    const GURL& path,
    const content::WebContents::Getter& wc_getter,
    GotDataCallback callback) {
  NOTREACHED() << "We should never get here since the extension should have"
               << "been triggered";

  std::move(callback).Run(nullptr);
}

std::string NotesUIHTMLSource::GetMimeType(const std::string& path) {
  NOTREACHED() << "We should never get here since the extension should have"
               << "been triggered";
  return "text/html";
}

NotesUIHTMLSource::~NotesUIHTMLSource() {}

NotesUI::NotesUI(content::WebUI* web_ui) : WebUIController(web_ui) {
  Profile* profile = Profile::FromWebUI(web_ui);
  content::URLDataSource::Add(profile, std::make_unique<NotesUIHTMLSource>());
}
}  // namespace vivaldi
