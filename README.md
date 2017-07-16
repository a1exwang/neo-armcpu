## neo-armcpu

## 终极目标
在新板子上复现 armcpu

#### 2017.07.10
- 将新版 toplevel 放进 armcpu, 把原来的 armcpu 作为一个 module
  - ISE 升级到 Vivado 遇到的一些坑
    - IP Core 需要升级, 旧版的不能直接拿来用
    - 需要修改一下 FPGA 的型号
  - 在 @宇翔 帮助下, 成功烧入 ucore 到 Flash, 2KB/s
    - 坑
      - 上电
      - 烧入 soc_toplevel.bit
      - 设置 baudrate
      - 要把最右面拨码开关置0
      - 刷入
    - 顺便用 NaiveBootloader 验证了内存和 Flash 正常工作
  - 尝试直接包一层 toplevel 之后直接烧入新版子, 无法启动 ucore, 想先从在旧板子上复现开始着手.

#### 2017.07.11
- 在旧板子复现 armcpu
  - 找 @李山山 老师要了几块老板子, 没有一块能正常工作, 最好的情况是能进 ucore 但是按键盘任意按键 Kernel Panic.
    - 这条路不通了, 暂时到这.

- 继续找 VGA 没有显示的问题
  - 首先发现了 Flash 读写的信号有所不同, 对着代码改了一下, 还是没有显示
  - 把 instr_addr 显示出来, 发现开机在 0x80000080 ~ 0x8000008C 死循环了
  - 把 instr_data 显示出来, 发现 CPU MMU 中读到的 baseram 的高 8 位都是 1(其余没看)
  - 修改 @宇翔 提供的 serial_load.py, 添加了读内存功能, 读一下内存, 发现读出的是正常的 ucore, 怀疑是内存读写信号有问题.
  - 新板子提供的 toplevel 中, ram_oe, ram_we 等信号叫做 ram_oe_n, .. 本来以为是和 armcpu 中是反的, 擅自加了个 '~', 删掉.
  - 能进入 ucore, 但是 VGA 有问题, 必须将时钟降低到几十 K 的时候才能正常显示, 能进入 Shell

- TODOs
  - 搞清楚新板子内存信号的定义
  - 解决 VGA 高频率时抖动黑屏问题

#### 2017.07.12
- VGA 方向
  - 暂时先用低频率凑合
  - Vivado 里加上 timing constraints 50MHz, VGA能成功在 50MHz 显示了, 但是结果是一下出来, 而不是一行一行出来
- 键盘方向
  - 先写了一个用 touch_btn 控制的键盘, 测通后再弄 USB
  - 研究了一下 sl811 的用法, 觉得需要调研一下 sl811 在 Linux 下的驱动和 USB HID 协议. 有两条路线.
    - 可以试着把 Linux 下的 sl811 驱动移植到 ucore, 这样, 在硬件层面不需要做太多事情.
    - 可以试着用硬件实现 sl811 驱动的一个子集 (只需要读键盘输入就好了), 这样不用改软件.
- ucore 方向
  - 能进入 ucore 了, 但是 kernel panic, invalid pa
  - 怀疑是不是频率太低导致的, 先用高频率时钟启动, 等一会之后再将始终拨慢, 成功进入 Shell, 键盘也可以工作 (这些逗号是用 touch_btn 输入的, 字符在代码里写死了是逗号, >__<)
    ![s1](screenshots/neo_ucore_shell.jpg)

- TODOs
  - VGA, 一下出来的 bug
  - 键盘, 支持 USB 键盘
  - ucore, 应该不需要有太大修改了
  - flash, 之后试一下从 Flash 启动
  - 可以试着用一下 Vivado ILA 调试

#### 2017.07.13
- USB 键盘
  - 首先看一下 sl811-hcd.c 里面发现 USB 设备的代码, 找到了一块简单的获取设备版本号的代码, 感觉可以先实现一下, 试着读出版本号, 写到串口
  - 只要稍微修改实例读写串口就会挂, 发现是 PC 端读写串口不对, `echo/cat` 读写串口不靠谱, 赶快用 `screen /dev/ttyACM0 115200` 吧
  - 尝试参照 sl811-hcd.c 和 sl811 的手册, 初始化 sl811, 设置中断模式, 在这上面浪费了一天, 感觉这条路不通(至少时间不允许), 打算先做串口虚拟键盘
- TODOs
  - USB 太复杂了, 如果遇到瓶颈感觉可以先用串口做一个一个虚拟键盘出来
  - 想做虚拟键盘需要修改 ucore 需要看一下他们的 ucore 怎么 build
    - 直接 make
      - 遇到问题, 有几处路径的小问题, 改之
      - 编译通过, 检查一下 gcc 生成的指令, `mipsel-linux-gnu-objdump -d ../ucore/obj/mm/vmm.o   | awk '{print $3}' | sort | uniq | grep -E "^[a-z]+\$"`, 对比他们之前生成的 ucore
      - 发现多了 3 条指令 `lwl, lwr, bal`, 查了一下是用来做非对其访存的
      - 用这条命令找到那些文件中多余了这些指令

      ```
      #!/bin/bash
        for file in $(find obj -type f); do
          if mipsel-linux-gnu-objdump -d $file   | awk '{print $3}' | sort | uniq | grep -E "^[a-z]+\$" | grep -E "lwl|lwr|bal" > /dev/null; then
            echo "file: " $file
          fi
        done
      ```
      - 发现只有 user app 和 ramdisk.o 中有, 详细看一下 build 这两部分的 Makefile
      - 结果换了 mips-sde-elf 工具链就好了

#### 2017.07.14
- 键盘
  - 在 ucore 测试了通过串口写入 ASCII 码可以正常使用 shell
  - 打算走软件实现 sl811 驱动程序这个方向, 先实现软件读写 sl811 的寄存器
  - 调通了读 sl811 寄存器功能, 明天把写加上

#### 2017.07.15
- SL811
  - 在 Vivado 中仿真 SL811 的模块, 功能是将 SL811 的读写分别映射到两个内存地址
  - 这样的好处就是和 Linux 下的 SL811 驱动实现的方法类似, 易于将 Linux 下的驱动移植过来
- 存在问题
  - SL811 的时序图有些不敢确定, 之后根据时序图再修改一下读写 SL811 的等待时间
  - 加上 SL811 之后 会出现 AlignmentError

#### 2017.07.16
- SL811
  - 发现了时序上的 bug 和 rst_n 写反了, 改过来之后能顺利读 HWREV 寄存器, REV = 2
  - ![hwreg](screenshots/neo_ucore_sl811.jpg)
  - 开始调试写 SL811 寄存器

## Long Term Goals
- ~~修改 armcpu 能在新板子上有 VGA 显示~~
- ~~先用 verilog 写一个假的键盘, 用拨码开关控制, 能在 Shell 中打印出字符.~~
- 添加 USB 键盘支持
- 调研 armcpu 上的那些扩展怎么跑起来
  - 图片渲染
  - 贪吃蛇

## Reference
- [@jiakai 项目](https://git.net9.org/armcpu-devteam/armcpu)
- [Linux sl811 Driver](https://github.com/torvalds/linux/blob/5924bbecd0267d87c24110cbe2041b5075173a25/drivers/usb/host/sl811-hcd.c)
- [Cypress SL811HS Manual](http://www.cypress.com/file/126236/download)
- @张宇翔
