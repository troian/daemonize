#
# Copyright [2016] [Artur Troian <troian at ap dot gmail dot com>]
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

project(daemonize)

add_library(
	${PROJECT_NAME}
	STATIC
	daemonize.cpp
	detach.cpp
	child.cpp
	utils.cpp

	include/export/daemon/daemonize.hpp
	include/local/daemon/utils.hpp
)

target_link_libraries(
	${PROJECT_NAME}
	jsoncpp
	${Boost_LIBRARIES}
)

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
		include/export
		${Boost_INCLUDE_DIR}
	PRIVATE
		include/local
)
