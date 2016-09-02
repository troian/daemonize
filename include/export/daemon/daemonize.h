/**
 * Copyright [2016] [Artur Troian <troian dot ap at gmail dot com>]
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

#pragma once

#include <string>
#include <sys/file.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <wait.h>
#include <sys/resource.h>
#include <sys/ucontext.h>
#include <ucontext.h>
#include <ulimit.h>
#include <signal.h>

/**
 * \typedef
 *
 * \brief
 */
typedef void (*cleanup_cb)(void *ctx);

/**
 * \brief  Get application environment dir
 *
 * \return
 */
std::string get_env_dir();

/**
 * \brief
 *
 * \param[in]  env_dir
 * \param[in]  lock_file
 * \param[in]  pid_file
 * \param[in]  cb
 * \param[in]  daemonize
 * \param[in]  ctx
 *
 * \return
 */
int make_daemon(std::string *env_dir, std::string *lock_file, std::string *pid_file, cleanup_cb cb, bool *daemonize, std::string *f_stdout, std::string *f_stderr, void *ctx = nullptr);

/**
 * \brief
 *
 * \param err
 */
void err_exit(int err);
