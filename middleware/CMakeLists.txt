# Copyright (c) 2021-2023 HPMicro
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory_ifdef(CONFIG_LVGL lvgl)
add_subdirectory_ifdef(CONFIG_TINYUSB tinyusb)
add_subdirectory_ifdef(CONFIG_TINYCRYPT tinycrypt)
add_subdirectory_ifdef(CONFIG_FATFS fatfs)
add_subdirectory_ifdef(CONFIG_FREERTOS FreeRTOS)
add_subdirectory_ifdef(CONFIG_MOTORCTRL hpm_mcl)
add_subdirectory_ifdef(CONFIG_SDMMC hpm_sdmmc)
add_subdirectory_ifdef(CONFIG_LIBJPEG libjpeg-turbo)
add_subdirectory_ifdef(CONFIG_LWIP lwip)
add_subdirectory_ifdef(CONFIG_COREMARK coremark)
add_subdirectory_ifdef(CONFIG_TFLM tflm)
add_subdirectory_ifdef(CONFIG_HPM_MATH hpm_math)
add_subdirectory_ifdef(CONFIG_AUDIO_CODEC audio_codec)
add_subdirectory_ifdef(CONFIG_SEGGER_RTT segger_rtt)
add_subdirectory_ifdef(CONFIG_ERPC erpc)
add_subdirectory_ifdef(CONFIG_CHERRYUSB cherryusb)
add_subdirectory_ifdef(CONFIG_MBEDTLS mbedtls)
add_subdirectory_ifdef(CONFIG_UCOS_III ucos_iii)
add_subdirectory(azure_rtos)
add_subdirectory_ifdef(CONFIG_MICROROS microros)
