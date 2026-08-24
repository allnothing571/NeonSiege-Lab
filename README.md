# 《霓虹防线》（Neon Siege）

> 当前阶段：一周冲刺第 1 天（SDL2 环境与空窗口）  
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
- [ ] 在本机完成 x64 Debug 构建
- [ ] 完成 `docs/Day1_验收.md` 中的基础练习
- [ ] 用户能够解释 SDL 初始化、事件循环和资源释放顺序

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
