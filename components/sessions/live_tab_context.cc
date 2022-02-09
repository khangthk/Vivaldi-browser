// Copyright (c) 2016 Vivaldi Technologies

#include "components/sessions/core/live_tab_context.h"
#include "components/page_actions/page_actions_tab_helper.h"
#include "components/sessions/content/content_live_tab.h"

namespace sessions {

std::string LiveTabContext::GetExtData() const {
  return std::string();
}

LiveTab* LiveTabContext::AddRestoredTab(
    const std::vector<SerializedNavigationEntry>& navigations,
    int tab_index,
    int selected_navigation,
    const std::string& extension_app_id,
    absl::optional<tab_groups::TabGroupId> group,
    const tab_groups::TabGroupVisualData& group_visual_data,
    bool select,
    bool pin,
    const PlatformSpecificTabData* tab_platform_data,
    const sessions::SerializedUserAgentOverride& user_agent_override,
    const SessionID* tab_id) {
  const std::map<std::string, bool> dummy_page_action_overrides;
  const std::string dummy_ext_data;
  return AddRestoredTab(navigations, tab_index, selected_navigation,
                        extension_app_id, group, group_visual_data, select, pin,
                        tab_platform_data, user_agent_override, tab_id,
                        dummy_page_action_overrides, dummy_ext_data);
}

LiveTab* LiveTabContext::AddRestoredTab(
    const std::vector<SerializedNavigationEntry>& navigations,
    int tab_index,
    int selected_navigation,
    const std::string& extension_app_id,
    absl::optional<tab_groups::TabGroupId> group,
    const tab_groups::TabGroupVisualData& group_visual_data,
    bool select,
    bool pin,
    const PlatformSpecificTabData* tab_platform_data,
    const sessions::SerializedUserAgentOverride& user_agent_override,
    const SessionID* tab_id,
    const std::map<std::string, bool> page_action_overrides,
    const std::string& ext_data) {
  return AddRestoredTab(navigations, tab_index, selected_navigation,
                        extension_app_id, group, group_visual_data, select, pin,
                        tab_platform_data, user_agent_override, tab_id);
}

LiveTab* LiveTabContext::ReplaceRestoredTab(
    const std::vector<SerializedNavigationEntry>& navigations,
    absl::optional<tab_groups::TabGroupId> group,
    int selected_navigation,
    const std::string& extension_app_id,
    const PlatformSpecificTabData* tab_platform_data,
    const sessions::SerializedUserAgentOverride& user_agent_override) {
  const std::map<std::string, bool> dummy_page_action_overrides;
  const std::string dummy_ext_data;
  return ReplaceRestoredTab(navigations, group, selected_navigation,
                            extension_app_id, tab_platform_data,
                            user_agent_override, dummy_page_action_overrides,
                            dummy_ext_data);
}

LiveTab* LiveTabContext::ReplaceRestoredTab(
    const std::vector<SerializedNavigationEntry>& navigations,
    absl::optional<tab_groups::TabGroupId> group,
    int selected_navigation,
    const std::string& extension_app_id,
    const PlatformSpecificTabData* tab_platform_data,
    const sessions::SerializedUserAgentOverride& user_agent_override,
    const std::map<std::string, bool> page_action_overrides,
    const std::string& ext_data) {
  return ReplaceRestoredTab(navigations, group, selected_navigation,
                            extension_app_id, tab_platform_data,
                            user_agent_override);
}

const std::string& ContentLiveTab::GetExtData() const {
  return web_contents()->GetExtData();
}

}  // namespace sessions
