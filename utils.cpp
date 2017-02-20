//
// Created by Artur Troian on 2/17/17.
//

#include <unistd.h>
#include <sys/stat.h>

#include <cstdlib>
#include <cstring>

#include <daemon/utils.hpp>

namespace daemonize {

void close_derived_fds()
{
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
			if (close(fd) != 0) {
				_exit(EXIT_FAILURE);
			}
		}
	}
}

} // namespace daemonize
