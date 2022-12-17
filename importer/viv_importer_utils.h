// Copyright (c) 2013 Vivaldi Technologies AS. All rights reserved

#ifndef IMPORTER_VIV_IMPORTER_UTILS_H_
#define IMPORTER_VIV_IMPORTER_UTILS_H_

base::FilePath GetProfileDir(bool mail=false);
base::FilePath GetMailDirectory(bool mail=false);
base::FilePath GetOperaInstallPathFromRegistry();
base::FilePath GetThunderbirdMailDirectory();

#endif  // IMPORTER_VIV_IMPORTER_UTILS_H_
