// Copyright (c) 2019 Vivaldi Technologies AS. All rights reserved

#ifndef COMPONENTS_REQUEST_FILTER_ADBLOCK_FILTER_ADBLOCK_REQUEST_FILTER_H_
#define COMPONENTS_REQUEST_FILTER_ADBLOCK_FILTER_ADBLOCK_REQUEST_FILTER_H_

#include <map>

#include "components/request_filter/adblock_filter/adblock_metadata.h"
#include "components/request_filter/request_filter.h"

namespace adblock_filter {
class RulesIndexManager;
class BlockedUrlsReporter;
class Resources;

class AdBlockRequestFilter : public vivaldi::RequestFilter {
 public:
  AdBlockRequestFilter(base::WeakPtr<RulesIndexManager> rules_index_manager,
                       base::WeakPtr<BlockedUrlsReporter> blocked_urls_reporter,
                       base::WeakPtr<Resources> resources);
  ~AdBlockRequestFilter() override;
  AdBlockRequestFilter(const AdBlockRequestFilter&) = delete;
  AdBlockRequestFilter& operator=(const AdBlockRequestFilter&) = delete;

  // Implementing vivaldi::RequestFilter
  bool WantsExtraHeadersForAnyRequest() const override;
  bool WantsExtraHeadersForRequest(
      vivaldi::FilteredRequestInfo* request) const override;
  bool OnBeforeRequest(content::BrowserContext* browser_context,
                       const vivaldi::FilteredRequestInfo* request,
                       BeforeRequestCallback callback) override;
  bool OnBeforeSendHeaders(content::BrowserContext* browser_context,
                           const vivaldi::FilteredRequestInfo* request,
                           const net::HttpRequestHeaders* headers,
                           BeforeSendHeadersCallback callback) override;
  void OnSendHeaders(content::BrowserContext* browser_context,
                     const vivaldi::FilteredRequestInfo* request,
                     const net::HttpRequestHeaders& headers) override;
  bool OnHeadersReceived(content::BrowserContext* browser_context,
                         const vivaldi::FilteredRequestInfo* request,
                         const net::HttpResponseHeaders* headers,
                         HeadersReceivedCallback callback) override;
  void OnBeforeRedirect(content::BrowserContext* browser_context,
                        const vivaldi::FilteredRequestInfo* request,
                        const GURL& redirect_url) override;
  void OnResponseStarted(content::BrowserContext* browser_context,
                         const vivaldi::FilteredRequestInfo* request) override;
  void OnCompleted(content::BrowserContext* browser_context,
                   const vivaldi::FilteredRequestInfo* request) override;
  void OnErrorOccured(content::BrowserContext* browser_context,
                      const vivaldi::FilteredRequestInfo* request,
                      int net_error) override;

 private:
  base::WeakPtr<RulesIndexManager> rules_index_manager_;
  base::WeakPtr<BlockedUrlsReporter> blocked_urls_reporter_;
  base::WeakPtr<Resources> resources_;
};

}  // namespace adblock_filter

#endif  // COMPONENTS_REQUEST_FILTER_ADBLOCK_FILTER_ADBLOCK_REQUEST_FILTER_H_
