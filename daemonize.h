/**
 * Copyright [2016] [Artur Troian <troian at ap dot gmail dot com>]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _DAEMONIZE_H_
#define _DAEMONIZE_H_

#include <string>
#include <sys/file.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/stat.h>
//#include <execinfo.h>
#include <stdlib.h>
#include <errno.h>
#include <wait.h>
#include <sys/resource.h>
#include <sys/ucontext.h>
#include <ucontext.h>
#include <ulimit.h>
#include <signal.h>

typedef void (*cleanup_cb)(void *ctx);

std::string get_env_dir();
int make_daemon(std::string *env_dir, std::string *lock_file, std::string *pid_file, cleanup_cb cb, void *ctx = nullptr);
void err_exit(int err);

#endif //_DAEMONIZE_H_
