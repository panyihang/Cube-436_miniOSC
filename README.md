 
#  Cube-436_迷你示波器

### 10MSPS采样，自动增益/自动切换基准的简易示波器

# <span class="colour" style="color: rgb(230, 126, 35);">Q群 565264047 欢迎来玩呀～</span>


# <span class="colour" style="color: rgb(224, 62, 45);">实测AD9220可超频至23M，极限性能正在测试</span>

# <span class="colour" style="color: rgb(224, 62, 45);">MCU、ADC、屏幕都是超频运行，板子有点烫手属于正常情况</span>

![3](https://image.lceda.cn/pullimage/WB8XVdREomYZhtNl8M5JkbJpknsZ2PxkMuA4PWYB.jpeg)

![4](https://image.lceda.cn/pullimage/h0vvsvYXFGvT1iCB2Wp3XbAAi31ILU1KrqI5NAv9.jpeg)
<span class="colour" style="color: rgb(53, 152, 219);">2Mhz 正弦波</span>

代码开源仓库：[https://github.com/panyihang/Cube-436_miniOSC](https://github.com/panyihang/Cube-436_miniOSC)
正在更新，代码还不完善，但。。。又不是不能用（欢迎各位巨佬提pr

# 0x00:前言

PCB有几个DRC报错，封装问题不影响使用，忽视就好。

本项目的硬件部分是高考前最后一个晚自修在教室里当众画的，仓促画完略有不足还请见谅

# 0x01:更新说明

ver0.2:更改led驱动供电，原供电方案电流不足

# 0x02:硬件部分

主控MCU：

* 采用树莓派的RP2040
* 核心电压超到1.3V
* 系统频率超到270Mhz
* 经测试可稳定运行

ADC：

* 采用AD9220
* 12位10MSPS采样
* 可超频至20M+
* 可单独控制VREF和通道B的电压

屏幕：

* 8位并口驱动
* 分辨率240x240
* 由于缺少小钱钱，采用库存的屏，型号为 HL015Z1E
* 绑定芯片为st7789，请自行找货/找替代品
* 替代品初始化参数开启反色，显示效果更佳哦～

存储：

* 128Mb SPI Flash
* 用于存放固件
* 可存放波形文件

# 0x03:软件框架

编译：

* 编译器为树莓派提供的c sdk
* 使用vs code的PlatformIO扩展进行开发
* 使用的PIO开发包为WizIO-PICO

为提高效率，修改了编译用的c sdk ，使用官方SDK理论上可以正常编译（现已基本替代，可直接编译）

显示框架：

* 显示框架采用lvgl8.2
* 屏幕驱动方式为gpio批量操作

adc采集：

* 采集深度为10kpit
* 采样精度12bit
* 采用汇编精确控制采样频率
* 未来将会使用pio配合空闲核心进行采样。

自动控制：

* 计算采样数据的最值和方差
* 自动控制ADC的基准电压和采样范围
* 无需担心错过细小的波形

![1](https://image.lceda.cn/pullimage/DhV5DszQxuONb4S6HaB6WCvhJcI0Ubelp8AJFCiY.jpeg)

![2](https://image.lceda.cn/pullimage/Sinc5q41PdtK2LAq1QpCrhfhOf42gW2CfiAX4Hry.jpeg)