# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

include(MesosTestsConfigure)

######################
set(ACTIVE_USER_TEST_HELPER_SRC
  ${ACTIVE_USER_TEST_HELPER_SRC}
  active_user_test_helper.cpp
  )

add_subdirectory(containerizer/)

# TODO(hausdorff): Add the other tests here.

# THE TEST AND HELPER EXECUTABLEs (generates, e.g., stout_tests, etc., on Linux).
#################################$###############################################
add_executable(${ACTIVE_USER_TEST_HELPER_TARGET} ${ACTIVE_USER_TEST_HELPER_SRC})

# ADD BINARY DEPENDENCIES (tells CMake what to compile/build first).
####################################################################
add_dependencies(${ACTIVE_USER_TEST_HELPER_TARGET} ${STOUT_DEPENDENCIES})
