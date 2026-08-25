# 《霓虹防线》（Neon Siege）

> 当前阶段：第 3 天技术任务已完成，准备进入第 4 天（敌人、碰撞与计分）
>
> 项目定位：C++ 学习项目；后续仅在自建程序和授权环境中开展游戏安全实验。

## 本周目标（纵向切片）

- C++17 + SDL2 + Visual Studio 2022 x64
- 固定时间步游戏循环
- 玩家移动与瞄准射击
- 敌人生成、追踪和碰撞
- 生命、计分、波次、暂停、结束和重开
- 最高分本地保存

本周不实现商店、地图、素材系统、联网、外挂或反作弊功能。未完成的功能不会写入简历。

## 第 1 天状态

- [x] 建立 CMake 工程
- [x] 提供 SDL2 空窗口启动代码
- [x] 在本机完成 x64 Debug 构建
- [x] `--smoke-test` 自动运行一帧并以退出码 0 结束
- [x] 完成 `docs/Day1_验收.md` 中的基础练习
- [x] 用户能够解释 SDL 初始化、事件循环和资源释放顺序

第 1 天最终复验：实体练习控制台内容与文件内容一致，两个程序退出码均为 0。SDL2 第三方源码出现一条本地代码页警告，但项目自有源码没有产生 `/W4` 警告。

## 第 2 天状态

- [x] 使用高精度计数器获取真实帧时间
- [x] 使用累加器进行 60 Hz 固定时间步更新，并限制最大帧时间
- [x] WASD 连续输入、方向归一化和窗口边界限制
- [x] 贴墙斜向输入时保持沿边速度
- [x] 读取鼠标位置并绘制玩家朝向线
- [x] Debug 构建及 `--smoke-test` 通过

第2天最终复验：项目自有源码无编译警告，窗口移动、边界、鼠标朝向和退出流程均通过人工测试；冒烟测试退出码为0。

## 第 3 天状态

- [x] 将玩家状态和更新/绘制逻辑整理为 `Player`
- [x] 使用 `Bullet` 和 `std::vector<Bullet>` 管理动态子弹
- [x] 鼠标左键单发、固定时间步移动和越界清理
- [x] Debug 构建及 `--smoke-test` 通过

第3天技术任务已完成。练手投递属于外部岗位选择和提交流程，待确认目标岗位后执行。

## 构建

项目通过 CMake `FetchContent` 获取 SDL2 2.32.4，首次配置需要访问 GitHub。

```powershell
& 'D:\VS2022\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' `
  -S . -B build -G 'Visual Studio 17 2022' -A x64

& 'D:\VS2022\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' `
  --build build --config Debug
```

生成后运行：

```powershell
.\build\Debug\NeonSiege.exe
```

按 `Esc` 或关闭窗口退出。

自动冒烟测试（隐藏窗口，只运行一帧）：

```powershell
$env:SDL_VIDEODRIVER = 'dummy'
.\build\Debug\NeonSiege.exe --smoke-test
Remove-Item Env:SDL_VIDEODRIVER
```

## 计划中的源码结构

```text
src/
├─ main.cpp
├─ Game.h/cpp
├─ Player.h/cpp
├─ Enemy.h/cpp
├─ Bullet.h/cpp
├─ WaveManager.h/cpp
├─ SaveData.h/cpp
└─ GameState.h
```

只有在对应功能实际运行并通过验收后，才创建和勾选后续模块。

## 长期路线（不属于本周简历成果）

1. 完成可玩的游戏主体。
2. 仅对自建游戏进行内存与存档篡改实验。
3. 在有实测结果后逐项增加完整性校验、反调试和行为检测。
