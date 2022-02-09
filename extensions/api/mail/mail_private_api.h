// Copyright (c) 2018 Vivaldi Technologies AS. All rights reserved

#ifndef EXTENSIONS_API_MAIL_MAIL_PRIVATE_API_H_
#define EXTENSIONS_API_MAIL_MAIL_PRIVATE_API_H_

#include "base/files/file_path.h"
#include "components/db/mail_client/mail_client_service.h"
#include "extensions/browser/api/file_system/file_system_api.h"
#include "extensions/browser/browser_context_keyed_api_factory.h"
#include "extensions/browser/extension_function.h"
#include "extensions/schema/mail_private.h"

using mail_client::MailClientService;

namespace extensions {

class MailPrivateAsyncFunction : public ExtensionFunction {
 public:
  MailPrivateAsyncFunction() = default;

 protected:
  MailClientService* GetMailClientService();
  Profile* GetProfile() const;
  ~MailPrivateAsyncFunction() override {}
};

class MailPrivateGetFilePathsFunction : public ExtensionFunction {
  DECLARE_EXTENSION_FUNCTION("mailPrivate.getFilePaths", MAIL_GET_FILE_PATHS)
 public:
  MailPrivateGetFilePathsFunction() = default;

 private:
  ~MailPrivateGetFilePathsFunction() override = default;
  void OnFinished(const std::vector<base::FilePath::StringType>& string_paths);

  // ExtensionFunction:
  ResponseAction Run() override;
};

class MailPrivateGetFullPathFunction : public ExtensionFunction {
  DECLARE_EXTENSION_FUNCTION("mailPrivate.getFullPath", MAIL_GET_FULL_PATH)
 public:
  MailPrivateGetFullPathFunction() = default;

 private:
  ~MailPrivateGetFullPathFunction() override = default;

  // ExtensionFunction:
  ResponseAction Run() override;
};

class MailPrivateGetMailFilePathsFunction : public ExtensionFunction {
  DECLARE_EXTENSION_FUNCTION("mailPrivate.getMailFilePaths",
                             MAIL_GET_MAIL_FILE_PATHS)
 public:
  MailPrivateGetMailFilePathsFunction() = default;

 private:
  ~MailPrivateGetMailFilePathsFunction() override = default;
  void OnFinished(const std::vector<base::FilePath::StringType>& string_paths);

  // ExtensionFunction:
  ResponseAction Run() override;
};

class MailPrivateWriteBufferToMessageFileFunction : public ExtensionFunction {
  DECLARE_EXTENSION_FUNCTION("mailPrivate.writeBufferToMessageFile",
                             MAIL_WRITE_BUFFER_TO_MESSAGE_FILE)
 public:
  MailPrivateWriteBufferToMessageFileFunction() = default;

 private:
  ~MailPrivateWriteBufferToMessageFileFunction() override = default;
  void OnFinished(bool result);
  // ExtensionFunction:
  ResponseAction Run() override;
};

class MailPrivateWriteTextToMessageFileFunction : public ExtensionFunction {
  DECLARE_EXTENSION_FUNCTION("mailPrivate.writeTextToMessageFile",
                             MAIL_WRITE_TEXT_TO_MESSAGE_FILE)
 public:
  MailPrivateWriteTextToMessageFileFunction() = default;

 private:
  ~MailPrivateWriteTextToMessageFileFunction() override = default;
  void OnFinished(bool result);
  // ExtensionFunction:
  ResponseAction Run() override;
};

class MailPrivateDeleteMessageFileFunction : public ExtensionFunction {
  DECLARE_EXTENSION_FUNCTION("mailPrivate.deleteMessageFile",
                             MAIL_DELETE_MESSAGE_FILE)
 public:
  MailPrivateDeleteMessageFileFunction() = default;

 private:
  ~MailPrivateDeleteMessageFileFunction() override = default;
  void OnFinished(bool result);
  // ExtensionFunction:
  ResponseAction Run() override;
};

struct ReadFileResult {
  bool success;
  std::string raw;
};

class MailPrivateReadFileToBufferFunction : public ExtensionFunction {
  DECLARE_EXTENSION_FUNCTION("mailPrivate.readFileToBuffer",
                             MAIL_READ_FILE_TO_BUFFER)
 public:
  MailPrivateReadFileToBufferFunction() = default;

 private:
  ~MailPrivateReadFileToBufferFunction() override = default;
  void OnFinished(ReadFileResult result);
  // ExtensionFunction:
  ResponseAction Run() override;
};

class MailPrivateReadMessageFileToBufferFunction : public ExtensionFunction {
  DECLARE_EXTENSION_FUNCTION("mailPrivate.readMessageFileToBuffer",
                             MAIL_READ_MESSAGE_FILE_TO_BUFFER)
 public:
  MailPrivateReadMessageFileToBufferFunction() = default;

 private:
  ~MailPrivateReadMessageFileToBufferFunction() override = default;
  void OnFinished(ReadFileResult result);
  // ExtensionFunction:
  ResponseAction Run() override;
};

class MailPrivateReadFileToTextFunction : public ExtensionFunction {
  DECLARE_EXTENSION_FUNCTION("mailPrivate.readFileToText",
                             MAIL_READ_FILE_TO_TEXT)
 public:
  MailPrivateReadFileToTextFunction() = default;

 private:
  ~MailPrivateReadFileToTextFunction() override = default;
  void OnFinished(ReadFileResult result);
  // ExtensionFunction:
  ResponseAction Run() override;
};

struct GetDirectoryResult {
  bool success;
  base::FilePath::StringType path;
};

class MailPrivateGetFileDirectoryFunction : public ExtensionFunction {
  DECLARE_EXTENSION_FUNCTION("mailPrivate.getFileDirectory",
                             MAIL_GET_FILE_DIRECTORY)
 public:
  MailPrivateGetFileDirectoryFunction() = default;

 private:
  ~MailPrivateGetFileDirectoryFunction() override = default;
  void OnFinished(GetDirectoryResult result);
  // ExtensionFunction:
  ResponseAction Run() override;
};

class MailPrivateCreateFileDirectoryFunction : public ExtensionFunction {
  DECLARE_EXTENSION_FUNCTION("mailPrivate.createFileDirectory",
                             MAIL_CREATE_FILE_DIRECTORY)
 public:
  MailPrivateCreateFileDirectoryFunction() = default;

 private:
  ~MailPrivateCreateFileDirectoryFunction() override = default;
  void OnFinished(GetDirectoryResult result);
  // ExtensionFunction:
  ResponseAction Run() override;
};

class MailPrivateCreateMessagesFunction : public MailPrivateAsyncFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("mailPrivate.createMessages", MAIL_CREATE_MESSAGES)
  MailPrivateCreateMessagesFunction() = default;

 private:
  ~MailPrivateCreateMessagesFunction() override = default;
  // ExtensionFunction:
  ResponseAction Run() override;

  // Callback for the create message function to provide results.
  void CreateMessagesComplete(std::shared_ptr<bool> result);

  // The task tracker for the MailClientService callbacks.
  base::CancelableTaskTracker task_tracker_;
};

class MailPrivateDeleteMessagesFunction : public MailPrivateAsyncFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("mailPrivate.deleteMessages", MAIL_DELETE_MESSAGES)
  MailPrivateDeleteMessagesFunction() = default;

 private:
  ~MailPrivateDeleteMessagesFunction() override = default;
  // ExtensionFunction:
  ResponseAction Run() override;

  // Callback for the DeleteMessages function to provide results.
  void DeleteMessagesComplete(std::shared_ptr<bool> result);

  // The task tracker for the MailClientService callbacks.
  base::CancelableTaskTracker task_tracker_;
};

class MailPrivateAddMessageBodyFunction : public MailPrivateAsyncFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("mailPrivate.addMessageBody",
                             MAIL_ADD_MESSAGE_BODY)
  MailPrivateAddMessageBodyFunction() = default;

 private:
  ~MailPrivateAddMessageBodyFunction() override = default;
  // ExtensionFunction:
  ResponseAction Run() override;

  // Callback for the AddMessageBody function to provide results.
  void AddMessageBodyComplete(
      std::shared_ptr<mail_client::MessageResult> result);

  // The task tracker for the MailClientService callbacks.
  base::CancelableTaskTracker task_tracker_;
};

class MailPrivateSearchMessagesFunction : public MailPrivateAsyncFunction {
  DECLARE_EXTENSION_FUNCTION("mailPrivate.searchMessages", MAIL_SEARCH_MESSAGES)
 public:
  MailPrivateSearchMessagesFunction() = default;

 private:
  ~MailPrivateSearchMessagesFunction() override = default;

  // ExtensionFunction:
  ResponseAction Run() override;

  // Callback for the MessageSearch function to provide results.
  void MessagesSearchComplete(
      std::shared_ptr<mail_client::SearchListIdRows> results);

  base::CancelableTaskTracker task_tracker_;
};

class MailPrivateMatchMessageFunction : public MailPrivateAsyncFunction {
  DECLARE_EXTENSION_FUNCTION("mailPrivate.matchMessage", MAIL_MATCH_MESSAGE)
 public:
  MailPrivateMatchMessageFunction() = default;

 private:
  ~MailPrivateMatchMessageFunction() override = default;

  // ExtensionFunction:
  ResponseAction Run() override;

  // Callback for the MatchMessage function to provide results.
  void MatchMessageComplete(std::shared_ptr<bool> results);

  base::CancelableTaskTracker task_tracker_;
};

class MailPrivateRebuildDatabaseFunction : public MailPrivateAsyncFunction {
  DECLARE_EXTENSION_FUNCTION("mailPrivate.rebuildDatabase",
                             MAIL_SEARCH_REBUILD_DATABASE)
 public:
  MailPrivateRebuildDatabaseFunction() = default;

 private:
  ~MailPrivateRebuildDatabaseFunction() override = default;

  // ExtensionFunction:
  ResponseAction Run() override;

  // Callback for the RebuildDatabse function to provide results.
  void RebuildStartedCallback(std::shared_ptr<bool> results);

  base::CancelableTaskTracker task_tracker_;
};

}  // namespace extensions

#endif  // EXTENSIONS_API_MAIL_MAIL_PRIVATE_API_H_
