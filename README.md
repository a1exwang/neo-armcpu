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

## TODOs
- 修改 armcpu 能在新板子上有 VGA 显示
