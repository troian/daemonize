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

#include <sys/file.h>
#include <unistd.h>

#include <daemon/daemonize.hpp>
#include <daemon/utils.hpp>


namespace daemonize {

pid_t child::execute(const char *path, const char * const argv[], const char * const envv[]) {
	pid_t pid = make();

	if (pid > 0 || pid < 0) {
		return pid;
	}

	/* execute requested program */
	if (envv) {
		execve(path, const_cast<char * const *>(argv), const_cast<char * const *>(envv));
	} else {
		execv(path, const_cast<char * const *>(argv));
	}

	// execv*() doesn't return, if successful
	_exit(EXIT_FAILURE);
}

pid_t child::make() {
	pid_t pid;

	pid = fork();

	if (pid == 0) {
		// Close all of file descriptors
		close_derived_fds();
	}

	return pid;
}

} // namespace daemonize
