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
- 键盘方向
  - 先写了一个用 touch_btn 控制的键盘, 测通后再弄 USB
- ucore 方向
  - 能进入 ucore 了, 但是 kernel panic, invalid pa
  - 怀疑是不是频率太低导致的, 先用高频率时钟启动, 等一会之后再将始终拨慢, 成功进入 Shell, 键盘也可以工作 (这些逗号是用 touch_btn 输入的, 字符在代码里写死了是逗号, >__<)
    ![s1](screenshots/neo_ucore_shell.jpg)

- TODOs
  - VGA, 让 VGA 再高频率下可以运行
  - 键盘, 支持 USB 键盘
  - ucore, 应该不需要有太大修改了
  - 可以试着用一下 Vivado ILA 调试

## Long Term Goals
- ~~修改 armcpu 能在新板子上有 VGA 显示~~
- ~~先用 verilog 写一个假的键盘, 用拨码开关控制, 能在 Shell 中打印出字符.~~
- 添加 USB 键盘支持
