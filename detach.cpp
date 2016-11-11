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

pid_t detached::execute(const char *path, char *const *argv, char *const *envv)
{
	pid_t pid = daemonize();

	if (pid > 0 || pid < 0)
		return pid;

	/* execute requested program */
	execve(path, argv, envv);

	/* execl() don't return, if successful */
	_exit(EXIT_FAILURE);
}

pid_t detached::execute(const char *path, char *const *argv)
{
	pid_t pid = daemonize();

	if (pid > 0 || pid < 0)
		return pid;

	/* execute requested program */
	execv(path, argv);

	/* execl() don't return, if successful */
	_exit(EXIT_FAILURE);
}

pid_t detached::daemonize()
{
	pid_t pid;
	int fds[2];

	/* create pipe to retrive pid of daemon */
	if (pipe(fds)) {
		return (-1);
	}

	/* first fork */
	if ((pid = fork())) {
		/* close pipe for writting now to avoid deadlock, if child failed */
		close(fds[1]);

		if (-1 == pid) {
			/* can't fork, return error */
			goto done;
		}

		int status;
		pid_t rc;

		/* wait until child process finish with daemon startup */
		while (-1 == (rc = waitpid(pid, &status, 0))) {
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

		return (pid);
	}

	/* second fork for daemon startup */
	if ((pid = fork())) {
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
	/* retrieve maximum fd number */
	int max_fds = getdtablesize();

	if (max_fds == -1) {
		_exit(EXIT_FAILURE);
	}

	/* close all fds, except standard (in, out and err) streams */
	for (int fd = 3; fd < max_fds; ++fd) {
		struct stat st;
		std::memset(&st, 0, sizeof(struct stat));

		if (fstat(fd, &st)) {
			/* fd not used */
			continue;
		}

		if (close(fd)) {
			_exit(EXIT_FAILURE);
		}
	}

	return 0;
}

} // namespace app_daemon
