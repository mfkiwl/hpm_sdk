# Copyright (c) 2022 HPMicro
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.13)

set(CMAKE_BUILD_TYPE sec_core_img)
set(BUILD_FOR_SECONDARY_CORE 1)
set(IS_SEC_CORE_IMG true)
set(SEC_CORE_IMG_C_ARRAY_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/../core0/src/sec_core_img.c)
set(CONFIG_ERPC 1)
set(CONFIG_IPC_EVENT_MGR 1)
set(CONFIG_IPC_EVENT_MGR_MBX 1)

find_package(hpm-sdk REQUIRED HINTS $ENV{HPM_SDK_BASE})

project(erpc_rpmsg_bm_core1)

set(CMAKE_CXX_STANDARD 11)

sdk_compile_definitions(-DHPM_FEATURE_MBX_SIDE_B)
sdk_compile_options(-O1)

sdk_inc(src)
sdk_app_src(src/main_core1.c)

sdk_inc(../config)
sdk_inc(../generated)
sdk_src(../generated/erpc_matrix_multiply_server.cpp)

# ERPC Baremetal Server Middleware Files
sdk_src(../../../../../middleware/erpc/erpc/erpc_c/infra/erpc_basic_codec.cpp)
sdk_src(../../../../../middleware/erpc/erpc/erpc_c/infra/erpc_crc16.cpp)
sdk_src(../../../../../middleware/erpc/erpc/erpc_c/infra/erpc_message_buffer.cpp)
sdk_src(../../../../../middleware/erpc/erpc/erpc_c/infra/erpc_message_loggers.cpp)
sdk_src(../../../../../middleware/erpc/erpc/erpc_c/infra/erpc_pre_post_action.cpp)
sdk_src(../../../../../middleware/erpc/erpc/erpc_c/infra/erpc_server.cpp)
sdk_src(../../../../../middleware/erpc/erpc/erpc_c/infra/erpc_simple_server.cpp)

sdk_src(../../../../../middleware/erpc/erpc/erpc_c/port/erpc_port_stdlib.cpp)

sdk_src(../../../../../middleware/erpc/erpc/erpc_c/setup/erpc_server_setup.cpp)
sdk_src(../../../../../middleware/erpc/erpc/erpc_c/setup/erpc_setup_mbf_rpmsg.cpp)
sdk_src(../../../../../middleware/erpc/erpc/erpc_c/setup/erpc_setup_rpmsg_lite_remote.cpp)

sdk_src(../../../../../middleware/erpc/erpc/erpc_c/transports/erpc_rpmsg_lite_transport.cpp)

sdk_ld_options("--std=c++11")
generate_ses_project()


