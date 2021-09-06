// Copyright (c) 2017 Vivaldi Technologies AS. All rights reserved

#ifndef BROWSER_LAUNCH_UPDATE_NOTIFIER_H_
#define BROWSER_LAUNCH_UPDATE_NOTIFIER_H_

#include "base/command_line.h"

class Profile;

namespace vivaldi {

void LaunchUpdateNotifier(Profile* profile);

#ifdef OS_WIN
void DisableStandaloneAutoUpdate();
void EnableStandaloneAutoUpdate();
bool IsStandaloneAutoUpdateEnabled();
#endif

}  // namespace vivaldi

#endif  // oROWSER_LAUNCH_UPDATE_NOTIFIER_H_
