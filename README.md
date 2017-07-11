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

## TODOs
- 修改 armcpu 能在新板子上有 VGA 显示
