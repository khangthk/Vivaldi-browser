// Copyright (c) 2019 Vivaldi Technologies AS. All rights reserved

#ifndef UI_CONTENT_VIVALDI_EVENT_HOOKS_H_
#define UI_CONTENT_VIVALDI_EVENT_HOOKS_H_

#include "base/supports_user_data.h"
#include "chromium/content/common/content_export.h" // nogncheck
#include "ui/base/dragdrop/mojom/drag_drop_types.mojom-forward.h"

namespace blink {
class WebMouseEvent;
class WebMouseWheelEvent;
}  // namespace blink

namespace content {
struct NativeWebKeyboardEvent;
class RenderWidgetHostImpl;
class RenderWidgetHostViewBase;
class WebContents;
}  // namespace content

namespace ui {
class LatencyInfo;
}  // namespace ui

// Hooks into Chromium event processing. The implementation is provided in
// tabs_private_api.cc and stored in WebContents associated with the Vivaldi
// window using SupportsUserData API.
class CONTENT_EXPORT VivaldiEventHooks : public base::SupportsUserData::Data {
 public:
  // Extra flag to extend ui::DragDropTypes::DragOperation to indicate cancelled
  // drag operation.
  static const int DRAG_CANCEL = 1 << 30;

  // Check for a mouse gesture event before it is dispatched to the web page
  // or default chromium handlers. Return true to stop further event
  // propagation or false to allow normal event flow.
  static bool HandleMouseEvent(content::RenderWidgetHostViewBase* root_view,
                                const blink::WebMouseEvent& event);

  // Check for a wheel gesture event before it is dispatched to the web page
  // or default chromium handlers. Return true to stop further event
  // propagation or false to allow normal event flow.
  static bool HandleWheelEvent(content::RenderWidgetHostViewBase* root_view,
                                const blink::WebMouseWheelEvent& event,
                                const ui::LatencyInfo& latency);

  // Check for a wheel gesture after the event was not consumed by any child
  // view. Return true to stop further event propagation or false to allow
  // normal event flow.
  static bool HandleWheelEventAfterChild(
      content::RenderWidgetHostViewBase* root_view,
      const blink::WebMouseWheelEvent& event);

  // Handle a keyboard event before it is send to the renderer process. Return
  // true to stop further event propagation or false to allow normal event flow.
  static bool HandleKeyboardEvent(content::RenderWidgetHostImpl* widget_host,
                                  const content::NativeWebKeyboardEvent& event);

  // Hook to notify UI about the end of the drag operation and pointer position
  // when the user released the pointer. Return true to prevent any default
  // action in Chromium. cancelled indicate that the platform API indicated
  // explicitly cancelled drag (currently can be true only on Windows).
  static bool HandleDragEnd(content::WebContents* web_contents,
                            ui::mojom::DragOperation operation,
                            bool cancelled,
                            int screen_x,
                            int screen_y);

 protected:
  static bool HasInstance();

  static void InitInstance(VivaldiEventHooks& instance);

  virtual bool DoHandleMouseEvent(
      content::RenderWidgetHostViewBase* root_view,
      const blink::WebMouseEvent& event) = 0;

  virtual bool DoHandleWheelEvent(content::RenderWidgetHostViewBase* root_view,
                                const blink::WebMouseWheelEvent& event,
                                const ui::LatencyInfo& latency) = 0;

  virtual bool DoHandleWheelEventAfterChild(
      content::RenderWidgetHostViewBase* root_view,
      const blink::WebMouseWheelEvent& event) = 0;

  virtual bool DoHandleKeyboardEvent(
      content::RenderWidgetHostImpl* widget_host,
      const content::NativeWebKeyboardEvent& event) = 0;

  virtual bool DoHandleDragEnd(content::WebContents* web_contents,
                               ui::mojom::DragOperation operation,
                               bool cancelled,
                               int screen_x,
                               int screen_y) = 0;

 private:
  static VivaldiEventHooks* instance_;
};

#endif  // UI_CONTENT_VIVALDI_EVENT_HOOKS_H_
