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

#include <syslog.h>
#include <bits/siginfo.h>
#include <getopt.h>
#include <signal.h>
#include <pthread.h>

#include <string>
#include <memory>
#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>
#include <daemon/daemonize.hpp>

static std::string *env_dir = nullptr;
static std::string *pid_file = nullptr;
static std::string *lock_file = nullptr;
static int lock_fd = 0;

void cleanup(void *ctx)
{
	if (lock_fd > 0) {
		if (flock(lock_fd, LOCK_UN) != 0) {
			syslog(LOG_ERR, "Can't unlock the lock file \"%s\". Ooops?\n", lock_file->c_str());
		}
		lock_fd = 0;
	}

	unlink(pid_file->c_str());
	delete env_dir;
	delete pid_file;
	delete lock_file;
}

void signal_handler(int sig, siginfo_t *info, void *ctx)
{
	void     *array[50];
	void     *caller_address;
	size_t    size;
	struct sigaction sigact;

	/* Get the address at the time the signal was raised */
#if defined(__i386__) // gcc specific
	sig_ucontext_t *uc = static_cast<sig_ucontext_t *>(ctx);
	caller_address = static_cast<void *>(uc->uc_mcontext.eip); // EIP: x86 specific
#elif defined(__x86_64__) // gcc specific
	sig_ucontext_t *uc = static_cast<sig_ucontext_t *>(ctx);
	caller_address = static_cast<void *>(uc->uc_mcontext.rip); // RIP: x86_64 specific
#elif defined(__arm__)
	ucontext_t *uc = static_cast<ucontext_t *>(ctx);
//	struct sigcontext *crashContext = &uc->uc_mcontext;
	caller_address = static_cast<void *>(uc->uc_mcontext.arm_pc);
#else
	#error Unsupported architecture. // TODO(Artur Troian): Add support for other arch.
#endif

	syslog(LOG_ERR, "Process fault into error. signal %s. Error: %s\n",
	       strsignal(sig),
	       info->si_errno < 0 ? strerror(info->si_errno) : "<none>");

	std::string bt_file(*env_dir);
	bt_file += "/log/backtrace.txt";
	int bt_fd = open(bt_file.c_str(), O_CREAT | O_WRONLY | O_TRUNC);
	if (bt_fd < 0) {
		syslog(LOG_ERR, "Unable create backtrace file: [%s]. Reason: %s", bt_file.c_str(), strerror(errno));
		err_exit(EXIT_FAILURE);
	}
	if (chmod(bt_file.c_str(), 0644) < 0) {
		syslog(LOG_ERR, "Unable change file permision: [%s]. Reason: %s", bt_file.c_str(), strerror(errno));
		err_exit(EXIT_FAILURE);
	}

	size = backtrace(array, 50);

	/* overwrite sigaction with caller's address */
	array[1] = caller_address;

	backtrace_symbols_fd(array, size, bt_fd);

	close(bt_fd);

	sigact.sa_flags = SA_RESETHAND;
	sigaction(sig, &sigact, NULL);

	raise(sig);

	_exit(EXIT_FAILURE);
}

int worker()
{
	struct sigaction sigact;
	int              sig;
	sigset_t         wait_mask;

	openlog("splendid_server", LOG_PID, LOG_DAEMON);

	sigact.sa_flags = SA_SIGINFO;
	sigact.sa_sigaction = signal_handler;

	sigemptyset(&sigact.sa_mask);

	if (sigaction(SIGFPE, &sigact, &old_sigact) < 0) {
		syslog(LOG_ERR, "Unable set sigaction SIGFPE. Error: %s", strerror(errno));
		err_exit(errno);
	}
	if (sigaction(SIGILL, &sigact, NULL) < 0) {
		syslog(LOG_ERR, "Unable set sigaction SIGILL. Error: %s", strerror(errno));
		err_exit(errno);
	}
	if (sigaction(SIGSEGV, &sigact, NULL) < 0) {
		syslog(LOG_ERR, "Unable set sigaction SIGSEGV. Error: %s", strerror(errno));
		err_exit(errno);
	}
	if (sigaction(SIGBUS, &sigact, NULL) < 0) {
		syslog(LOG_ERR, "Unable set sigaction SIGBUS. Error: %s", strerror(errno));
		err_exit(errno);
	}
	if (sigaction(SIGHUP, &sigact, NULL) < 0) {
		syslog(LOG_ERR, "Unable set sigaction SIGHUP. Error: %s", strerror(errno));
		err_exit(errno);
	}
	if (sigaction(SIGABRT, &sigact, NULL) < 0) {
		syslog(LOG_ERR, "Unable set sigaction SIGABRT. Error: %s", strerror(errno));
		err_exit(errno);
	}

	sigemptyset(&wait_mask);

	if (sigaddset(&wait_mask, SIGQUIT) < 0) {
		syslog(LOG_ERR, "Unable add SIGQUIT to set. Error: %s", strerror(errno));
		err_exit(errno);
	}
	if (sigaddset(&wait_mask, SIGINT) < 0) {
		syslog(LOG_ERR, "Unable add SIGINT to set. Error: %s", strerror(errno));
		err_exit(errno);
	}
	if (sigaddset(&wait_mask, SIGTERM) < 0) {
		syslog(LOG_ERR, "Unable add SIGTERM to set. Error: %s", strerror(errno));
		err_exit(errno);
	}
	if (sigaddset(&wait_mask, SIGKILL) < 0) {
		syslog(LOG_ERR, "Unable add SIGKILL to set. Error: %s", strerror(errno));
		err_exit(errno);
	}

	if (pthread_sigmask(SIG_BLOCK, &wait_mask, NULL) != 0) {
		syslog(LOG_ERR, "Error on sig mask SIG_BLOCK. %s", strerror(errno));
		exit(EXIT_FAILURE);
	}



	// Run all necessary stuff here

	syslog(LOG_INFO, "[DAEMON] Started\n");
	sigwait(&wait_mask, &sig);
	syslog(LOG_ERR, "Received signal: %s", strsignal(sig));

	syslog(LOG_INFO, "[DAEMON] Stopped\n");

	closelog();

	return 0;
}

int main(int argc, char **argv)
{
	static struct option long_options[] = {
		{"env_dir",  required_argument, 0, 'e'},
		{"pid_file", required_argument, 0, 'p'},
		{NULL, 0, 0, 0}
	};

	int value;
	int option_index = 0;

	while((value = getopt_long(argc, argv, "e:p:", long_options, &option_index)) != -1) {
		switch(value) {
		case 'e': {
			env_dir = new std::string(optarg);
			break;
		}
		case 'p': {
			pid_file = new std::string(optarg);
			break;
		}
		default:
			break;
		}
	}

	boost::filesystem::path abs_exe_path = boost::filesystem::system_complete(argv[0]);
	lock_file = new std::string(abs_exe_path.string());

	lock_fd = make_daemon(env_dir, lock_file, pid_file, cleanup);

	worker();

	cleanup(nullptr);

	exit(EXIT_SUCCESS);
}
