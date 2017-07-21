title: USB 1.1 on neo-armcpu(a MIPS-like CPU)
speaker: 王奥丞
url: https://github.com/ksky521/nodeppt
transition: slide3
theme: moon
usemathjax: yes

[slide]
# USB on neo-armcpu
# 王奥丞

[slide]
## 原有成果
----
- ARMCPU - 32bit MIPS 指令集的一个子集 {:&.rollIn}
- MIPS 移植版 ucore labs
- 贪吃蛇, 幻灯片放映等用户级程序
- 外设: PS2 键盘, VGA 输出

[slide]
## 目标
----
- 复现 ucore-mips on ARMCPU {:&.rollIn}
- 复现用户程序
- 添加 SL811 USB Host Controller 的支持
- 基于以上, 添加 USB 键盘支持

[slide]
----
## 成果及实现 1
- 进入 ucore shell (通过 VGA 输出)
- ![ucore](/screenshots/neo_ucore_shell.jpg)

[slide]
----
## 成果及实现 2
- 成功用 USB 键盘操作 ucore
- ![sl811](/report/sl811.png)

[slide]
----
## 成果及实现 3.1
- 用键盘和 VGA 玩贪吃蛇
- ![snake1](/report/snake1.jpg)

[slide]
## 成果及实现 3.2
- 用键盘和 VGA 玩贪吃蛇
- ![snake2](/report/snake2.jpg)

[slide]
----
## 一些展望
- 在 ucore 中实现分层的 USB 协议, 从而使增加新设备更容易 {:&.rollIn}
- 通过中断/消息队列来实现 SL811 驱动, 而不是轮询提升性能
- 添加 USB Hub 的支持

[slide]
----
## 遇到的一些问题 1
- mips-linux-gnu-gcc 编译出来的ucore 编译出来会多出几条 armcpu 上没实现的指令 {:&.rollIn}
  - 最后发现只有指定版本的 mips-sde-elf-gcc 才能复现 ucore 的编译结果 {:&.rollIn}
- SL811 能读出寄存器值, 但是无法写入
  - 编译了一个 Vivado ILA 进去, 看了一眼发现一个读写信号写反了 {:&.rollIn}
- SL811 能读到版本号, 其他寄存器写入之后再读出没变化
  - 换了块板子好了 {:&.rollIn}

[slide]
----
## 遇到的一些问题 2
- SL811 对于某些设备有时无法 GetDescriptor, 设备会发送 NAK 给 Host  {:&.rollIn}
  - 怀疑是 USB frame 必须在 1ms 内发完的限制 {:&.rollIn}
  - 最后跟踪了 U-Boot 的 SL811 驱动, 发现他也会收到 NAK
  - 处理办法是重试, 然后就解决...
  - 解决了...
  - 卡了两天 >_<
- ucore fork 用户态进程进行轮询, 会卡住 Shell
  - 在 x86 版 ucore lab 8 上测试同样用户程序代码, 不会卡住  {:&.rollIn}
  - 发现这个 ucore 是阉割版, 并没有做到 lab8 那个程度
  - 进程调度还是手动 yield 的那个版本..

[slide]
----
## 未解之谜
- 某些键盘 (实验室里 Dell 的键盘) 在电脑上可以识别, 到 SL811 上 GetDescriptor timeout {:&.rollIn}
  - 怀疑是供电不足? {:&.rollIn}
- 在中断异常处理器中轮询 SL811 寄存器会导致启动的时候 kernel panic
  - 异常是 assertion failed: initproc != NULL && initproc->pid == 1 {:&.rollIn}
  - 还没有开中断就挂了, 但是删掉了这部分代码, 就完全正常
- VGA 在第一周的时候高频率下(几百K以上?)会闪烁, 到了第二周就好了
  - 怀疑可能是因为换了板子? {:&.rollIn}

[slide]
----
# 感谢列表
- 李山山老师
- 张宇翔
