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

#include <string>
#include <boost/filesystem.hpp>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <daemon/daemonize.h>

static int         *g_lock_fd   = nullptr;
static std::string *g_env_dir   = nullptr;
static std::string *g_pid_file  = nullptr;
static std::string *g_lock_file = nullptr;

cleanup_cb cleanup = nullptr;
void *cleanup_ctx = nullptr;

void err_exit(int err)
{
	if (cleanup)
		cleanup(cleanup_ctx);

	exit(err);
}

static bool already_running()
{
	g_lock_fd = new int;

	*g_lock_fd = open(g_lock_file->c_str(), O_RDONLY, S_IRUSR | S_IWUSR);
	if (g_lock_fd <= 0) {
		fprintf(stderr, "Can't open executable to lock: \"%s\": %s\n", g_lock_file->c_str(), strerror(errno));
		delete g_lock_fd;
		return true;
	}

	if (flock(*g_lock_fd, LOCK_EX | LOCK_NB) != 0) {
		fprintf(stderr, "Can't lock the lock file \"%s\". Is another instance running?\n", g_lock_file->c_str());
		close(*g_lock_fd);
		delete g_lock_fd;
		return true;
	}
	return false;
}

static int write_pid()
{
	int fd = open(g_pid_file->c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd <= 0) {
		fprintf(stderr, "Unable to open %s: %s", g_pid_file->c_str(), strerror(errno));
		err_exit(EXIT_FAILURE);
	}

	char buf[16];
	ftruncate(fd, 0);
	sprintf(buf, "%ld\n", (long)getpid());
	write(fd, buf, strlen(buf));
}

std::string get_env_dir()
{
	return *g_env_dir;
}

int make_daemon(std::string *env_dir, std::string *lock_file, std::string *pid_file, cleanup_cb cb, bool *daemonize, std::string *f_stdout, std::string *f_stderr, void *ctx)
{
	struct rlimit rl;

	g_env_dir   = env_dir;
	g_pid_file  = pid_file;
	g_lock_file = lock_file;

	cleanup = cb;
	cleanup_ctx = ctx;

	if (already_running()) {
		errno = EALREADY;
		return -1;
	}

	if (*daemonize == true) {
		pid_t pid;
		struct sigaction sa;

		umask(0);

		if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
			fprintf(stderr, "Unable to ger descriptor file. Error: %s", strerror(errno));
			err_exit(EXIT_FAILURE);
		}

		pid = fork();

		if (pid < 0) {
			fprintf(stderr, "Start Daemon Error: %s", strerror(errno));
			err_exit(EXIT_FAILURE);
		} else if (pid != 0) {
			exit(EXIT_SUCCESS);
		}

		if (setsid() < 0) {
			fprintf(stderr, "Unable to set signature id. Error: %s", strerror(errno));
			err_exit(EXIT_FAILURE);
		}

		sa.sa_handler = SIG_IGN;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		if (sigaction(SIGHUP, &sa, NULL) < 0) {
			fprintf(stderr, "Unable to ignore signal SIGHUP. Error: %s", strerror(errno));
			err_exit(EXIT_FAILURE);
		}

		// Let process be a session leader
		if ((pid = fork()) < 0) {
			fprintf(stderr, "Unable to fork. Error: %s", strerror(errno));
			err_exit(EXIT_FAILURE);
		} else if (pid != 0) {/* parent process */
			exit(EXIT_SUCCESS);
		}

		// Close all file descriptors
		fflush(stdin);
		fflush(stdout);
		fflush(stderr);

		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);

		// check of log directory exists
		std::string log_path(*env_dir);
		log_path += "/log";
		if (!boost::filesystem::exists(log_path)) {
			boost::filesystem::create_directory(log_path);
		}

		//reopen stdin, stdout, stderr
		int stdin_fd = open("/dev/null", O_RDONLY);
		if (stdin_fd != 0) {
			if (stdin_fd > 0)
				close(stdin_fd);
			fprintf(stderr, "Unable to redirect stdin: Opened to: %d. Error: %s", stdin_fd, strerror(errno));
			err_exit(EXIT_FAILURE);
		}

		std::string std_file;

		if (f_stdout->compare("/dev/null") == 0) {
			std_file = *f_stdout;
		} else {
			std_file.append(*env_dir);
			std_file.append("/log/stdout.log");
		}

		int stdout_fd = open(std_file.c_str(), O_CREAT | O_WRONLY | O_TRUNC);
		if (stdout_fd != 1) {
			if (stdout_fd > 0)
				close(stdout_fd);
			fprintf(stderr, "Unable to redirect stdout: Opened to: %d. Error: %s", stdout_fd, strerror(errno));
			err_exit(EXIT_FAILURE);
		}

		if (chmod(std_file.c_str(), 0644) < 0) {
			fprintf(stderr, "Unable change file permision: [%s]. Reason: %s", std_file.c_str(), strerror(errno));
			err_exit(EXIT_FAILURE);
		}

		if (f_stderr->compare("/dev/null") == 0) {
			std_file = *f_stderr;
		} else {
			std_file.clear();
			std_file.append(*env_dir);
			std_file.append("/log/stderr.log");
		}


		int stderr_fd = open(std_file.c_str(), O_CREAT | O_WRONLY | O_TRUNC);

		if (stderr_fd != 2) {
			if (stderr_fd > 0)
				close(stderr_fd);
			fprintf(stderr, "Unable to redirect stderr: Opened to: %d. Error: %s", stderr_fd, strerror(errno));
			err_exit(EXIT_FAILURE);
		}

		if (chmod(std_file.c_str(), 0644) < 0) {
			fprintf(stderr, "Unable change file permision: [%s]. Reason: %s", std_file.c_str(), strerror(errno));
			err_exit(EXIT_FAILURE);
		}
	}

	// Setup environment dir
	if (chdir(env_dir->c_str()) < 0) {
		fprintf(stderr, "Unable to change dir to [%s]. Error: %s", env_dir->c_str(), strerror(errno));
		err_exit(EXIT_FAILURE);
	}

	if (rl.rlim_max == RLIM_INFINITY) {
		rl.rlim_max = 1024;
	}

	struct rlimit core_limits;
	core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;

	if (setrlimit(RLIMIT_CORE, &core_limits) < 0) {
		fprintf(stderr, "Unable to set rlimits. Error: %s", strerror(errno));
		err_exit(EXIT_FAILURE);
	}

	write_pid();

	int lock_fd = *g_lock_fd;

	delete g_lock_fd;

	return lock_fd;
}
