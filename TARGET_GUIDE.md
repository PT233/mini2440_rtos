# Mini2440 RTOS - Keil 项目双 Target 配置

## 概述

本项目配置了 **两个 Target**，方便在仿真器和硬件之间切换：

1. **simulator** - Keil 仿真器调试（无需硬件）
2. **board** - 硬件 J-Link 调试（需要开发板）

## 切换方法

### 方法 1：在 Keil 中直接切换（推荐）

1. 打开 `mini2440rtos.uvproj`
2. **Project** 菜单 → **Select Project Target** 
3. 选择 `simulator` 或 `board`
4. **Build** 后即可调试

### 方法 2：编辑 uvproj 文件

如果 Keil 界面不显示 Target 列表，手动修改 uvproj：

```xml
<TargetSelection>
  2    <!-- 1=simulator, 2=board -->
</TargetSelection>
```

---

## 各 Target 配置详情

### 🟢 Simulator Target

| 项 | 值 |
|---|---|
| **启动代码** | `arch_keil\startup_keil_sim.S` |
| **散点文件** | `s3c2440_sim.sct` |
| **代码地址** | `0x00000000` (仿真器 SRAM) |
| **调试器** | SARM.DLL (Keil 内置仿真器) |
| **UseSimulator** | 1 |
| **UseTarget** | 0 |

**特点：**
- 跳过所有硬件初始化（CP15、时钟、SDRAM）
- 仿真器内置模拟 SRAM 和外设
- 快速启动，可立即验证逻辑

**适用场景：**
- 无硬件环境
- 快速调试应用代码
- 验证中断和任务切换逻辑

---

### 🔴 Board Target (硬件)

| 项 | 值 |
|---|---|
| **启动代码** | `arch_keil\startup_keil.S` |
| **散点文件** | `s3c2440.sct` |
| **代码地址** | `0x30000000` (SDRAM) |
| **调试器** | Segger\JLTAgdi.dll (J-Link) |
| **UseSimulator** | 0 |
| **UseTarget** | 1 |

**特点：**
- 完整硬件初始化（CP15 禁用、PLL、SDRAM）
- NAND → SDRAM 代码复制
- 全栈初始化（各 CPU 模式）

**适用场景：**
- 实际硬件调试
- 硬件外设驱动调试
- 最终产品验证

---

## 文件对应关系

```
startup_keil.S          → board Target（硬件完整启动）
startup_keil_sim.S      → simulator Target（简化启动）
s3c2440.sct             → board Target（0x30000000 链接）
s3c2440_sim.sct         → simulator Target（0x00000000 链接）
mini2440rtos.uvproj     → 包含两个 Target 定义
```

---

## 常见问题

### Q: 如何确认当前是哪个 Target？

Keil 顶部的工具栏会显示当前 Target 名称。

### Q: 编译后选错了 Target，怎么办？

1. 在 **Project → Select Project Target** 选择正确 Target
2. 点击 **Rebuild** 重新编译
3. 重新加载到调试器

### Q: 仿真器模式可以调试中断吗？

可以。Keil 仿真器支持定时器、中断等。但不支持真实硬件时序。

### Q: 硬件模式需要什么调试器？

需要 **J-Link 调试器**（已配置为 Segger\JLTAgdi.dll）

---

## 技术细节

### startup_keil.S (硬件)

**执行流程：**
1. 关闭看门狗 + 禁用中断
2. 安装向量表到 0x00000000
3. 禁用 CP15 缓存/MMU
4. PLL 时钟配置（FCLK=400MHz）
5. SDRAM 初始化
6. **检查 PC**：若 < 0x30000000，则从 NAND 复制到 0x30000000
7. 堆栈初始化（各 CPU 模式）
8. 跳转到 main()

**关键堆栈地址：**
```
0x33FF0000   <-- STACK_BASE
  ↓ 0x0400
0x33FEFBF00  <-- UndefStack
  ↓ 0x0400
0x33FEF800   <-- AbortStack
  ↓ 0x0400
0x33FEF400   <-- FIQStack
  ↓ 0x0400
0x33FEF000   <-- IRQStack
  ↓ 0x0400
0x33FEEC00   <-- SVCStack (main)
```

### startup_keil_sim.S (仿真器)

**执行流程：**
1. 安装向量表到 0x00000000
2. **跳过** CP15 操作（仿真器不支持）
3. **跳过** 时钟初始化（仿真器模拟）
4. **跳过** SDRAM 初始化（仿真器模拟）
5. 设置 SVC 堆栈
6. 跳转到 main()

**堆栈地址（IRAM 内）：**
```
0x40000F00   <-- SVCStack
```

---

## 验证步骤

### 仿真器验证

```bash
1. Project → Select Project Target → simulator
2. Build (F7)
3. Debug (Ctrl+F5)
4. 在 main() 设置断点
5. Run
   → 如果停在 main()，说明启动成功 ✓
```

### 硬件验证（需要 J-Link）

```bash
1. Project → Select Project Target → board
2. Build (F7)
3. Debug (Ctrl+F5)
4. 连接 J-Link
5. 在 main() 设置断点
6. Run
   → 如果停在 main()，说明硬件启动成功 ✓
```

---

## 修改 Target 配置

若需修改 Target 设置：

### 在 Keil 中修改

1. **Project → Options for Target**
2. 选择对应 Target（顶部标签页）
3. 修改配置
4. 点击 **OK**

### 直接编辑 uvproj

搜索 `<TargetName>simulator</TargetName>` 或 `<TargetName>board</TargetName>` 对应段落修改。

---

## 许可证

Mini2440 RTOS Project

