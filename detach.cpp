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

#include <sys/wait.h>
#include <unistd.h>

#include <daemon/daemonize.hpp>
#include <daemon/utils.hpp>

namespace daemonize {

pid_t detached::execute(const char *path, const char *const argv[], const char *const envv[])
{
	pid_t pid = make();

	if (pid > 0 || pid < 0)
		return pid;

	/* execute requested program */
	if (envv) {
		execve(path, const_cast<char * const *>(argv), const_cast<char * const *>(envv));
	} else {
		execv(path, const_cast<char * const *>(argv));

	}
	// exec*() doesn't return, if successful
	_exit(EXIT_FAILURE);
}

pid_t detached::make()
{
	pid_t pid;
	int fds[2];

	/* create pipe to retrive pid of daemon */
	if (pipe(fds) != 0) {
		return -1;
	}

	/* first fork */
	if ((pid = fork()) > 0) {
		/* close pipe for writting now to avoid deadlock, if child failed */
		close(fds[1]);

		if (-1 == pid) {
			/* can't fork, return error */
			goto done;
		}

		int status;

		/* wait until child process finish with daemon startup */
		while (-1 == waitpid(pid, &status, 0)) {
			/* ignore POSIX signals */
			if (EINTR != errno) {
				pid = -1;

				goto done;
			}
		}

		/* daemon startup failed */
		if (EXIT_FAILURE == WEXITSTATUS(status)) {
			pid = -1;

			goto done;
		}

		/* read pid of daemon from pipe */
		if (sizeof(pid) != read(fds[0], &pid, sizeof(pid))) {
			pid = -1;
		}

		done:
		close(fds[0]);

		return pid;
	}

	/* second fork for daemon startup */
	if ((pid = fork()) != 0) {
		if (-1 == pid) {
			/* fail to fork, report about it */
			_exit(EXIT_FAILURE);
		}

		/* report about successful attempt running of daemon */
		_exit(EXIT_SUCCESS);
	}

	/* now detach to init process */
	if (-1 == setsid()) {
		_exit(EXIT_FAILURE);
	}

	/* now we should report our pid... */
	pid = getpid();

	/* to avoid duplicates of daemon */
	if (sizeof(pid) != write(fds[1], &pid, sizeof(pid))) {
		_exit(EXIT_FAILURE);
	}

	// Close all of filedescriptors
	close_derived_fds();

	return 0;
}

} // namespace daemonize
