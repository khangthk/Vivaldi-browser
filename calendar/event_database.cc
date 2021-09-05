// Copyright (c) 2017 Vivaldi Technologies AS. All rights reserved
//
// Based on code that is:
//
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "calendar/event_database.h"

#include <string>
#include <vector>

#include "base/strings/utf_string_conversions.h"
#include "calendar/calendar_type.h"
#include "calendar/event_type.h"
#include "sql/statement.h"

namespace calendar {

EventDatabase::EventDatabase() {}

EventDatabase::~EventDatabase() {}

bool EventDatabase::CreateEventTable() {
  const char* name = "events";
  if (GetDB().DoesTableExist(name))
    return true;

  // Note: revise implementation for InsertOrUpdateURLRowByID() if you add any
  // new constraints to the schema.
  std::string sql;
  sql.append("CREATE TABLE ");
  sql.append(name);
  sql.append(
      "("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      // Using AUTOINCREMENT is for sync propose. Sync uses this |id| as an
      // unique key to identify the Events. If here did not use AUTOINCREMENT,
      // and Sync was not working somehow, a ROWID could be deleted and re-used
      // during this period. Once Sync come back, Sync would use ROWIDs and
      // timestamps to see if there are any updates need to be synced. And sync
      //  will only see the new Event, but missed the deleted Event.
      "calendar_id INTEGER, "
      "alarm_id INTEGER, "
      "title LONGVARCHAR,"
      "description LONGVARCHAR,"
      "start INTEGER NOT NULL,"
      "end INTEGER NOT NULL,"
      "all_day INTEGER,"
      "is_recurring INTEGER,"
      "start_recurring INTEGER,"
      "end_recurring INTEGER,"
      "location LONGVARCHAR,"
      "url LONGVARCHAR,"
      "etag LONGVARCHAR,"
      "href LONGVARCHAR,"
      "uid LONGVARCHAR,"
      "event_type_id INTEGER,"
      "task INTEGER,"
      "complete INTEGER,"
      "trash INTEGER,"
      "trash_time INTEGER, "
      "sequence INTEGER DEFAULT 0 NOT NULL,"
      "ical LONGVARCHAR,"
      "rrule LONGVARCHAR,"
      "organizer LONGVARCHAR,"
      "timezone LONGVARCHAR,"
      "is_template INTEGER DEFAULT 0 NOT NULL,"
      "created INTEGER,"
      "last_modified INTEGER"
      ")");

  return GetDB().Execute(sql.c_str());
}

EventID EventDatabase::CreateCalendarEvent(calendar::EventRow row) {
  sql::Statement statement(GetDB().GetCachedStatement(
      SQL_FROM_HERE,
      "INSERT OR REPLACE INTO events "
      "(calendar_id, alarm_id, title, description, "
      "start, end, all_day, is_recurring, start_recurring, end_recurring, "
      "location, url, etag, href, uid, event_type_id, task, complete, trash, "
      "trash_time, sequence, ical, rrule, organizer, timezone, is_template, "
      "created, last_modified) "
      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
      "?, ?, ?, ?, ?, ?, ?)"));

  statement.BindInt64(0, row.calendar_id());
  statement.BindInt64(1, row.alarm_id());
  statement.BindString16(2, row.title());
  statement.BindString16(3, row.description());
  statement.BindInt64(4, row.start().ToInternalValue());
  statement.BindInt64(5, row.end().ToInternalValue());
  statement.BindInt(6, row.all_day() ? 1 : 0);
  statement.BindInt(7, row.is_recurring() ? 1 : 0);
  statement.BindInt64(8, row.start_recurring().ToInternalValue());
  statement.BindInt64(9, row.end_recurring().ToInternalValue());
  statement.BindString16(10, row.location());
  statement.BindString16(11, row.url());
  statement.BindString(12, row.etag());
  statement.BindString(13, row.href());
  statement.BindString(14, row.uid());
  statement.BindInt64(15, row.event_type_id());
  statement.BindInt(16, row.task() ? 1 : 0);
  statement.BindInt(17, row.complete() ? 1 : 0);
  statement.BindInt(18, row.trash() ? 1 : 0);
  statement.BindInt64(19,
                      row.trash() ? base::Time().Now().ToInternalValue() : 0);
  statement.BindInt64(20, row.sequence());
  statement.BindString16(21, row.ical());
  statement.BindString(22, row.rrule());
  statement.BindString(23, row.organizer());
  statement.BindString(24, row.timezone());
  statement.BindInt(25, row.is_template() ? 1 : 0);
  statement.BindInt64(26, base::Time().Now().ToInternalValue());
  statement.BindInt64(27, base::Time().Now().ToInternalValue());

  if (!statement.Run()) {
    return 0;
  }
  return GetDB().GetLastInsertRowId();
}

bool EventDatabase::GetAllCalendarEvents(EventRows* events) {
  events->clear();
  sql::Statement s(GetDB().GetCachedStatement(
      SQL_FROM_HERE, "SELECT" CALENDAR_EVENT_ROW_FIELDS " FROM events "
                     "where is_template = 0"));

  while (s.Step()) {
    EventRow event;
    FillEventRow(s, &event);
    events->push_back(event);
  }

  return true;
}

bool EventDatabase::GetAllCalendarEventTemplates(EventRows* events) {
  events->clear();
  sql::Statement s(GetDB().GetCachedStatement(
      SQL_FROM_HERE, "SELECT" CALENDAR_EVENT_ROW_FIELDS " FROM events "
                     "where is_template = 1"));

  while (s.Step()) {
    EventRow event;
    FillEventRow(s, &event);
    events->push_back(event);
  }

  return true;
}

bool EventDatabase::GetRowForEvent(calendar::EventID event_id,
                                   EventRow* out_event) {
  sql::Statement statement(GetDB().GetCachedStatement(
      SQL_FROM_HERE,
      "SELECT" CALENDAR_EVENT_ROW_FIELDS "FROM events WHERE id=?"));
  statement.BindInt64(0, event_id);

  if (!statement.Step())
    return false;

  FillEventRow(statement, out_event);

  return true;
}

bool EventDatabase::UpdateEventRow(const EventRow& event) {
  sql::Statement statement(GetDB().GetCachedStatement(SQL_FROM_HERE,
                                                      "UPDATE events SET \
        calendar_id=?, alarm_id=?, title=?, description=?, start=?, end=?, \
        all_day=?, is_recurring=?, start_recurring=?, end_recurring=?, \
        location=?, url=?, etag=?, href=?, uid=?, event_type_id=?, \
        task=?, complete=?, trash=?, trash_time=?, sequence=?, ical=?, \
        rrule=?, organizer=?, timezone=? WHERE id=?"));
  statement.BindInt64(0, event.calendar_id());
  statement.BindInt64(1, event.alarm_id());
  statement.BindString16(2, event.title());
  statement.BindString16(3, event.description());
  statement.BindInt64(4, event.start().ToInternalValue());
  statement.BindInt64(5, event.end().ToInternalValue());
  statement.BindInt(6, event.all_day() ? 1 : 0);
  statement.BindInt(7, event.is_recurring() ? 1 : 0);
  statement.BindInt64(8, event.start_recurring().ToInternalValue());
  statement.BindInt64(9, event.end_recurring().ToInternalValue());
  statement.BindString16(10, event.location());
  statement.BindString16(11, event.url());
  statement.BindString(12, event.etag());
  statement.BindString(13, event.href());
  statement.BindString(14, event.uid());
  statement.BindInt64(15, event.event_type_id());
  statement.BindInt(16, event.task() ? 1 : 0);
  statement.BindInt(17, event.complete() ? 1 : 0);
  statement.BindInt(18, event.trash() ? 1 : 0);
  statement.BindInt64(19,
                      event.trash() ? base::Time().Now().ToInternalValue() : 0);
  statement.BindInt(20, event.sequence());
  statement.BindString16(21, event.ical());
  statement.BindString(22, event.rrule());
  statement.BindString(23, event.organizer());
  statement.BindString(24, event.timezone());
  statement.BindInt64(25, event.id());

  return statement.Run();
}

// Must be in sync with CALENDAR_EVENT_ROW_FIELDS.
// static
void EventDatabase::FillEventRow(sql::Statement& s, EventRow* event) {
  EventID id = s.ColumnInt64(0);
  CalendarID calendar_id = s.ColumnInt64(1);
  AlarmID alarm_id = s.ColumnInt64(2);
  base::string16 title = s.ColumnString16(3);
  base::string16 description = s.ColumnString16(4);
  base::Time start = base::Time::FromInternalValue(s.ColumnInt64(5));
  base::Time end = base::Time::FromInternalValue(s.ColumnInt64(6));
  int all_day = s.ColumnInt(7);
  int is_recurring = s.ColumnInt(8);
  base::Time start_recurring = base::Time::FromInternalValue(s.ColumnInt64(9));
  base::Time end_recurring = base::Time::FromInternalValue(s.ColumnInt64(10));
  base::string16 location = s.ColumnString16(11);
  base::string16 url = s.ColumnString16(12);
  std::string etag = s.ColumnString(13);
  std::string href = s.ColumnString(14);
  std::string uid = s.ColumnString(15);
  EventTypeID event_type_id = s.ColumnInt64(16);
  int task = s.ColumnInt(17);
  int complete = s.ColumnInt(18);
  int trash = s.ColumnInt(19);
  base::Time trash_time = base::Time::FromInternalValue(s.ColumnInt64(20));
  int sequence = s.ColumnInt(21);
  base::string16 ical = s.ColumnString16(22);
  std::string rrule = s.ColumnString(23);
  std::string organizer = s.ColumnString(24);
  std::string timezone = s.ColumnString(25);

  event->set_id(id);
  event->set_calendar_id(calendar_id);
  event->set_alarm_id(alarm_id);
  event->set_title(title);
  event->set_description(description);
  event->set_start(start);
  event->set_end(end);
  event->set_all_day(all_day == 1 ? true : false);
  event->set_is_recurring(is_recurring == 1 ? true : false);
  event->set_start_recurring(start_recurring);
  event->set_end_recurring(end_recurring);
  event->set_location(location);
  event->set_url(url);
  event->set_etag(etag);
  event->set_href(href);
  event->set_uid(uid);
  event->set_event_type_id(event_type_id);
  event->set_task(task == 1 ? true : false);
  event->set_complete(complete == 1 ? true : false);
  event->set_trash(trash == 1 ? true : false);
  event->set_trash_time(trash_time);
  event->set_sequence(sequence);
  event->set_ical(ical);
  event->set_rrule(rrule);
  event->set_organizer(organizer);
  event->set_timezone(timezone);
}

bool EventDatabase::DeleteEvent(calendar::EventID event_id) {
  sql::Statement statement(GetDB().GetCachedStatement(
      SQL_FROM_HERE, "DELETE from events WHERE id=?"));
  statement.BindInt64(0, event_id);

  return statement.Run();
}

bool EventDatabase::DeleteEventsForCalendar(CalendarID calendar_id) {
  sql::Statement statement(GetDB().GetCachedStatement(
      SQL_FROM_HERE, "DELETE FROM events WHERE calendar_id=?"));
  statement.BindInt64(0, calendar_id);
  return statement.Run();
}

bool EventDatabase::DoesEventIdExist(EventID event_id) {
  sql::Statement statement(
      GetDB().GetUniqueStatement("select count(*) as count from events \
        WHERE id=?"));
  statement.BindInt64(0, event_id);

  if (!statement.Step())
    return false;

  return statement.ColumnInt(0) == 1;
}

// Updates to version 2
bool EventDatabase::MigrateEventsWithoutSequenceAndIcalColumns() {
  if (!GetDB().DoesTableExist("events")) {
    NOTREACHED() << "Events table should exist before migration";
    return false;
  }

  if (!GetDB().DoesColumnExist("events", "sequence") &&
      !GetDB().DoesColumnExist("events", "ical")) {
    // Old versions don't have the sequence and ical column, we modify the table
    // to add that field.
    if (!GetDB().Execute("ALTER TABLE events "
                         "ADD COLUMN sequence INTEGER DEFAULT 0 NOT NULL"))
      return false;

    if (!GetDB().Execute("ALTER TABLE events "
                         "ADD COLUMN ical LONGVARCHAR"))
      return false;

    if (!GetDB().Execute("update calendar set ctag = ''"))
      return false;

    if (!GetDB().Execute("update events set etag = ''"))
      return false;
  }
  return true;
}

// Updates to version 4. Adds columns rrule to events
bool EventDatabase::MigrateCalendarToVersion4() {
  if (!GetDB().DoesTableExist("events")) {
    NOTREACHED() << "events table should exist before migration";
    return false;
  }

  if (!GetDB().DoesColumnExist("events", "rrule")) {
    if (!GetDB().Execute("ALTER TABLE events "
                         "ADD COLUMN rrule LONGVARCHAR"))
      return false;
  }
  return true;
}

// Updates to version 6. Adds columns timezone to events and calendar table
bool EventDatabase::MigrateCalendarToVersion6() {
  if (!GetDB().DoesTableExist("events")) {
    NOTREACHED() << "events table should exist before migration";
    return false;
  }

  if (!GetDB().DoesColumnExist("events", "timezone")) {
    if (!GetDB().Execute("ALTER TABLE events "
                         "ADD COLUMN timezone LONGVARCHAR"))
      return false;
  }

  if (!GetDB().DoesTableExist("calendar")) {
    NOTREACHED() << "calendar table should exist before migration";
    return false;
  }

  if (!GetDB().DoesColumnExist("calendar", "timezone")) {
    if (!GetDB().Execute("ALTER TABLE calendar "
                         "ADD COLUMN timezone LONGVARCHAR"))
      return false;
  }
  return true;
}

// Updates to version 8. Adds column is_template to events table
bool EventDatabase::MigrateCalendarToVersion8() {
  if (!GetDB().DoesTableExist("events")) {
    NOTREACHED() << "events table should exist before migration";
    return false;
  }

  if (!GetDB().DoesColumnExist("events", "is_template")) {
    if (!GetDB().Execute("ALTER TABLE events "
                         "ADD COLUMN is_template INTEGER DEFAULT 0 NOT NULL"))
      return false;
  }
  return true;
}

}  // namespace calendar
