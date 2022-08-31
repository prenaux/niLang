// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "Platform.h"

#if (defined niLinux || defined niQNX) && !defined niNoProcess

//--------------------------------------------------------------------------------------------
//
//  Process Utils
//
//--------------------------------------------------------------------------------------------
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include "API/niLang/STL/vector.h"

namespace {

enum ParsingState {
  KEY_NAME,
  KEY_VALUE
};

}  // namespace

namespace base {

bool LaunchApp(const astl::vector<astl::string>& argv,
               const file_handle_mapping_vector& fds_to_remap,
               ProcessHandle* process_handle,
               const astl::vector<ni::cString>& aEnvs) {
  bool retval = true;

  char* env_copy[aEnvs.size() + 1];
  niLoop(i,aEnvs.size()) {
    env_copy[i] = const_cast<char*>(aEnvs[i].Chars());
  }
  env_copy[aEnvs.size()] = NULL;

  char* argv_copy[argv.size() + 1];
  for (size_t i = 0; i < argv.size(); i++) {
    argv_copy[i] = new char[argv[i].size() + 1];
    strcpy(argv_copy[i], argv[i].c_str());
  }
  argv_copy[argv.size()] = NULL;

  // Make sure we don't leak any FDs to the child process by marking all FDs
  // as close-on-exec.
  SetAllFDsToCloseOnExec();

  int pid = fork();
  if (pid == 0) {
    for (file_handle_mapping_vector::const_iterator it = fds_to_remap.begin();
         it != fds_to_remap.end();
         ++it) {
      int src_fd = it->first;
      int dest_fd = it->second;
      if (src_fd == dest_fd) {
        int flags = fcntl(src_fd, F_GETFD);
        if (flags != -1) {
          fcntl(src_fd, F_SETFD, flags & ~FD_CLOEXEC);
        }
      } else {
        dup2(src_fd, dest_fd);
      }
    }
#ifdef niQNX
    execve(argv_copy[0], argv_copy, env_copy);
#else
    execvpe(argv_copy[0], argv_copy, env_copy);
#endif

  } else if (pid < 0) {
    retval = false;
  } else {
    if (process_handle)
      *process_handle = pid;
  }

  for (size_t i = 0; i < argv.size(); i++)
    delete[] argv_copy[i];

  return retval;
}

NamedProcessIterator::NamedProcessIterator(const astl::string& executable_name,
                                           const ProcessFilter* filter)
    : executable_name_(executable_name), filter_(filter) {
  procfs_dir_ = opendir("/proc");
}

NamedProcessIterator::~NamedProcessIterator() {
  if (procfs_dir_) {
    closedir(procfs_dir_);
    procfs_dir_ = NULL;
  }
}

const ProcessEntry* NamedProcessIterator::NextProcessEntry() {
  bool result = false;
  do {
    result = CheckForNextProcess();
  } while (result && !IncludeEntry());

  if (result)
    return &entry_;

  return NULL;
}

bool NamedProcessIterator::CheckForNextProcess() {
  // TODO(port): skip processes owned by different UID

  dirent* slot = 0;
  const char* openparen;
  const char* closeparen;
  char buf[NAME_MAX + 12] = {};

  // Arbitrarily guess that there will never be more than 200 non-process
  // files in /proc.  Hardy has 53.
  int skipped = 0;
  const int kSkipLimit = 200;
  while (skipped < kSkipLimit) {
    slot = readdir(procfs_dir_);
    // all done looking through /proc?
    if (!slot)
      return false;

    // If not a process, keep looking for one.
    bool notprocess = false;
    int i;
    for (i = 0; i < NAME_MAX && slot->d_name[i]; ++i) {
      if (!isdigit(slot->d_name[i])) {
        notprocess = true;
        break;
      }
    }
    if (i == NAME_MAX || notprocess) {
      skipped++;
      continue;
    }

    // Read the process's status.
    sprintf(buf, "/proc/%s/stat", slot->d_name);
    FILE *fp = fopen(buf, "r");
    if (!fp)
      return false;

    const char* result = fgets(buf, sizeof(buf), fp);
    fclose(fp);
    if (!result)
      return false;

    // Parse the status.  It is formatted like this:
    // %d (%s) %c %d ...
    // pid (name) runstate ppid
    // To avoid being fooled by names containing a closing paren, scan
    // backwards.
    openparen = strchr(buf, '(');
    closeparen = strrchr(buf, ')');
    if (!openparen || !closeparen)
      return false;

    char runstate = closeparen[2];

    // Is the process in 'Zombie' state, i.e. dead but waiting to be reaped?
    // Allowed values: D R S T Z
    if (runstate != 'Z')
      break;

    // Nope, it's a zombie; somebody isn't cleaning up after their children.
    // (e.g. WaitForProcessesToExit doesn't clean up after dead children yet.)
    // There could be a lot of zombies, can't really decrement i here.
  }
  if (skipped >= kSkipLimit) {
    //         NOTREACHED();
    return false;
  }

  entry_.pid = atoi(slot->d_name);
  entry_.ppid = atoi(closeparen + 3);

  // TODO(port): read pid's commandline's $0, like killall does.  Using the
  // short name between openparen and closeparen won't work for long names!
  int len = closeparen - openparen - 1;
  if (len > NAME_MAX)
    len = NAME_MAX;
  memcpy(entry_.szExeFile, openparen + 1, len);
  entry_.szExeFile[len] = 0;

  return true;
}

bool NamedProcessIterator::IncludeEntry() {
  // TODO(port): make this also work for non-ASCII filenames
  if (!executable_name_.empty()) {
    astl::string exeName(executable_name_.begin(),executable_name_.end());
    if (strcmp(exeName.c_str(),entry_.szExeFile) != 0)
      return false;
  }
  if (!filter_)
    return true;
  return filter_->Includes(entry_.pid, entry_.ppid);
}

}  // namespace base

#endif
