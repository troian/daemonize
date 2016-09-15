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

#include <json/json.h>
#include <json/value.h>

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
 * \param[in]  env_dir   - executable environment dir
 * \param[in]  lock_file - lock file to prevert running multiple instance
 * \param[in]  pid_file  - file to store pid
 * \param[in]  cb        - callback to cleanup context in case of error during init
 * \param[in]  daemonize - either run as daemon or app
 * \param[in]  config    - daemon config as Json::Value object with format
 *                         \code
 *                         {
 *                             "io_mode" : "io_daemon",
 *                             "io_daemon" : {
 *                                 "stdin": "/dev/null",
 *                                 "stdout": "/dev/null",
 *                                 "stderr": "/dev/null"
 *                             },
 *                             "io_debug" : {
 *                                 "stdin": "/dev/null",
 *                                 "stdout": "stdout",
 *                                 "stderr": "stderr"
 *                             }
 *                         \end
 * \param[in]  ctx
 *
 * \return
 */
int make_daemon(std::string *env_dir
				, std::string *lock_file
				, std::string *pid_file
				, cleanup_cb cb
				, bool *daemonize
				, Json::Value *config
				, void *ctx = nullptr);

/**
 * \brief
 *
 * \param err
 */
void err_exit(int err);
