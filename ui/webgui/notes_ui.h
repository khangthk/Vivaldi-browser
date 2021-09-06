// Copyright (c) 2013 Vivaldi Technologies AS. All rights reserved

#ifndef UI_WEBGUI_NOTES_UI_H_
#define UI_WEBGUI_NOTES_UI_H_

#include <string>
#include "content/public/browser/url_data_source.h"
#include "content/public/browser/web_ui_controller.h"

namespace vivaldi {

class NotesUIHTMLSource : public content::URLDataSource {
 public:
  NotesUIHTMLSource();
  ~NotesUIHTMLSource() override;

  std::string GetSource() override;
  void StartDataRequest(
      const GURL& path,
                        const content::WebContents::Getter& wc_getter,
      GotDataCallback callback) override;
  std::string GetMimeType(const std::string& path) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(NotesUIHTMLSource);
};

class NotesUI : public content::WebUIController {
 public:
  explicit NotesUI(content::WebUI* web_ui);

 private:
  DISALLOW_COPY_AND_ASSIGN(NotesUI);
};
}  // namespace vivaldi

#endif  // UI_WEBGUI_NOTES_UI_H_
