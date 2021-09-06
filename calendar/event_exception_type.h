// Copyright (c) 2019 Vivaldi Technologies AS. All rights reserved
//
// Based on code that is:
//
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CALENDAR_EVENT_EXCEPTION_TYPE_H_
#define CALENDAR_EVENT_EXCEPTION_TYPE_H_

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/time/time.h"

namespace calendar {

// EventException
// Holds all information associated with a event exception.
// Used during event creation
class EventExceptionType {
 public:
  EventExceptionType();  // = default;
  ~EventExceptionType() = default;
  EventExceptionType(const EventExceptionType& other);  // = default;

  base::string16 title;
  base::string16 description;
  base::Time exception_date;
  base::Time start;
  base::Time end;
  bool cancelled;
};

typedef std::vector<EventExceptionType> EventExceptions;

}  // namespace calendar

#endif  //  CALENDAR_EVENT_EXCEPTION_TYPE_H_
