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


#include <daemon/daemonize.h>

#include <cstdlib>
#include <cstring>

namespace app_daemon {

pid_t child::execute(const char *path, char *const *argv, char *const *envv)
{
	pid_t pid = run();

	if (pid > 0 || pid < 0)
		return pid;

	/* execute requested program */
	execve(path, argv, envv);

	/* execve() don't return, if successful */
	_exit(EXIT_FAILURE);
}

pid_t child::execute(const char *path, char *const *argv)
{
	pid_t pid = run();

	if (pid > 0 || pid < 0)
		return pid;

	/* execute requested program */
	execv(path, argv);

	/* execve() don't return, if successful */
	_exit(EXIT_FAILURE);
}

pid_t child::run()
{
	pid_t pid;

	pid = fork();

	if (pid == 0) {
		// Close all of filedescriptors
		/* retrieve maximum fd number */
		int max_fds = getdtablesize();

		if (max_fds == -1) {
			_exit(EXIT_FAILURE);
		}

		/* close all fds, except standard (in, out and err) streams */
		for (int fd = 3; fd < max_fds; ++fd) {
			struct stat st;
			std::memset(&st, 0, sizeof(struct stat));

			if (fstat(fd, &st) == 0) {
				/* fd used */
				if (close(fd)) {
					_exit(EXIT_FAILURE);
				}
			}
		}
	}

	return pid;
}

} // namespace app_daemon
