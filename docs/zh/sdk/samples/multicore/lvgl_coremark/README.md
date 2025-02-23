# LVGL Coremark 示例

## 概述

双核 LVGL Coremark示例用于执行两核Coremark并显示在图形界面上

在本工程中:
 - 点击 “Start”按键执行双核CoreMark
 - 点击“频率切换”热键执行CPU主频切换

## 硬件设置

  BOOT_PIN 应该设置为：0-OFF, 1-OFF


## 生成和编译多核工程

本示例中：core0示例在FLASH中原地执行， core1工程在ILM里执行。

用户必须先生成 __Core0__ 工程，作为关联工程，__Core1__ 工程会被自动生成

用户必须在编译完 __Core1__ 工程后再编译 __Core0__ 工程

### 生成core0工程
__CMAKE_BUILD_TYPE__ 必须是下列选项中的一种：
- *"flash_sdram"*
- *"flash_sdram_release"

## 运行现象

- 下载core0示例到设备，断开调试器，然后按复位键

当示例正常运行的时候,
- 点击 “Start”按键执行双核CoreMark， 当Coremark程序完成后，会在相应的位置显示Coremark分数
- 点击“频率切换”热键执行CPU主频切换
