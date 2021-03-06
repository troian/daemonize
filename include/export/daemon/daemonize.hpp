/**
 * Copyright [2016]
 *
 * \author [Artur Troian <troian dot ap at gmail dot com>]
 * \author [Oleg Kravchenko <troian dot ap at gmail dot com>]
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

#include <json/json.h>

namespace daemonize {

/**
 * \typedef
 *
 * \brief
 */
typedef void (*cleanup_cb)(void *ctx);

/**
 * \brief   Daemonize application
 *          This function may call \ref exit() in case of fatal error
 *
 * \param[in]  config    - daemon config as Json::Value object with format
 *                         This function will delete this object on destroy
 *                         \code{.json}
 *                         {
 *                             "as_daemon" : true,
 *                             "env_dir" : "/dir/dir",
 *                             "lock_file" : , // usually path to executable
 *                             "pid_file" : "/var/run/service.pid,
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
 *                         \endcode
 * \param[in]  cb        - callback to cleanup context in case of error during init
 * \param[in]  ctx       - user data passed to cleanup callback
 */
pid_t make_daemon(Json::Value *config, cleanup_cb cb = nullptr, void *userdata = nullptr);

/**
 * \brief
 *
 * \param err
 */
#ifdef __GNUC__
__attribute__ ((noreturn))
#endif
void exit_daemon(int err);

/**
 * \brief
 */
class detached {
public:
	/**
	 * \brief
	 *
	 * \param[in]  path
	 * \param[in]  argv
	 * \param[in]  envv
	 *
	 * \return
	 */
	static pid_t execute(const char *path, const char *const argv[], const char *const envv[] = nullptr);

public:
	static pid_t make();
};

class child {
public:
	/**
	 * \brief
	 *
	 * \param[in]  path
	 * \param[in]  argv
	 * \param[in]  envv
	 *
	 * \return
	 */
	static pid_t execute(const char *path, const char *const argv[], const char *const envv[] = nullptr);

private:
	static pid_t make();
};

} // namespace daemonize
