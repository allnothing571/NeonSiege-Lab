# Neon Siege 商业化音频升级执行手册

## 1. 升级目标

本轮升级为游戏建立可独立维护的音效管线，覆盖菜单反馈、战斗反馈、
波次与结算反馈，并把主音量和音效音量纳入现有设置系统。

完成标准：

- 使用 SDL_mixer 2.8.2 加载和播放 WAV（Waveform Audio File Format，波形音频文件格式）音效。
- 缺失或损坏的音效不会阻止游戏启动，游戏应静音降级并输出诊断信息。
- 旧版三字段设置文件仍可读取，新版设置保存五个字段。
- 主音量和音效音量均可在设置页以 10% 为步长调整，范围为 0% 到 100%。
- 游戏逻辑只产生 `PresentationEvent`，不直接依赖 SDL_mixer。
- 自动化测试通过后再进行人工听觉验收；本轮不自动提交或推送。

## 2. 架构边界

音频调用链如下：

`Simulation -> PresentationEvent -> GameApplication -> SdlAudioManager -> SDL_mixer`

核心规则：

- `src/core` 只描述“发生了什么”，不加载文件、不播放声音。
- `src/sdl` 负责音效目录、加载、音量、冷却和播放。
- `GameApplication` 负责把菜单操作和每帧表现事件转发给音频管理器。
- 素材路径统一相对于运行目录中的 `assets/`，禁止硬编码本机绝对路径。

## 3. 本地文件修改清单

行号以本次实现完成后的文件为准；后续编辑造成行号漂移时，以所列函数或功能块为定位依据。

### 步骤 1：接入 SDL_mixer

文件：`CMakeLists.txt`

修改块：

- SDL 依赖选项区：仅启用 WAV 解码，关闭当前版本不使用的编解码器和示例。
- `FetchContent_Declare(SDL2_mixer)`：固定 `release-2.8.2`。
- `neon_sdl` 源文件和链接区：加入 `SdlAudioCatalog`、`SdlAudioManager` 和
  `SDL2_mixer::SDL2_mixer-static`。
- 测试目标区：加入三个商业音频测试并注册到 CTest（CMake Test，CMake 测试工具）。

提示词：

> 在现有 SDL2 静态依赖结构中接入 SDL_mixer 2.8.2，只启用 WAV，保持核心层不依赖 SDL，并为音频目录、设置兼容和音频管理器分别注册自动化测试。

验收标准：CMake 可配置，`SDL2_mixer-static`、`neon_sdl` 和 `NeonSiege` 均可链接。

### 步骤 2：建立音效目录

文件：

- `src/sdl/SdlAudioCatalog.h`
- `src/sdl/SdlAudioCatalog.cpp`

修改块：定义 14 个 `SoundId`，并为每项登记相对路径、基础音量和重复播放冷却时间。

提示词：

> 建立类型安全的音效目录，确保枚举顺序和资源定义顺序一一对应，所有路径都相对于 assets，连续高频事件必须支持毫秒级冷却。

验收标准：目录恰好包含 14 个唯一、非空路径，基础音量在 0 到 100 之间。

### 步骤 3：实现音频管理器

文件：

- `src/sdl/SdlAudioManager.h`
- `src/sdl/SdlAudioManager.cpp`

修改块：

- 初始化 SDL 音频子系统和 SDL_mixer。
- 加载、持有和释放 `Mix_Chunk`。
- 计算 `基础音量 × 主音量 × 音效音量`。
- 根据冷却时间抑制过密的重复声音。
- 将 `PresentationEventType` 映射到 `SoundId`。
- 缺失资源只写入 `AudioLoadReport`，不终止游戏。

提示词：

> 实现 RAII（Resource Acquisition Is Initialization，资源获取即初始化）风格的 SDL 音频管理器，允许重复 shutdown，缺失音效静音降级，并正确处理包含中文的 UTF-8 路径。

验收标准：虚拟音频驱动下可初始化、加载测试 WAV、播放、停止并重复关闭；中文路径不导致崩溃。

### 步骤 4：扩展表现事件

文件：

- `src/core/PresentationEvent.h`
- `src/core/Simulation.cpp`
- `src/sdl/SdlPresentationEffects.cpp`
- `exercises/day7_presentation_events.cpp`

修改块：新增玩家射击、敌人射击、装填开始、装填完成、波次开始、升级确认、胜利和失败事件；
由模拟器在对应状态变化点记录，并让既有画面特效显式忽略只用于音频的事件。

提示词：

> 扩展表现事件但不改变遥测事件语义；装填开始与装填完成必须是两个独立事件，零时长装填只播放完成音，避免两个声音同帧叠加。

验收标准：波次开始、玩家射击、命中、受伤、死亡、胜利和失败场景测试通过；消费后事件队列为空。

### 步骤 5：扩展设置数据与设置页

文件：

- `src/sdl/SdlAppSettings.h`
- `src/sdl/SdlAppSettings.cpp`
- `src/sdl/SdlGameRenderer.cpp`
- `src/GameApplication.cpp`

修改块：

- `AppSettings` 新增 `masterVolume` 和 `effectsVolume`。
- 读取旧三字段格式时使用 80% 和 100% 默认值；保存为五字段格式。
- 设置页扩展为七行，新增主音量与音效音量。
- 左右键或点击以 10% 调整，取消时恢复已保存值，应用时持久化。

提示词：

> 在不破坏旧设置文件的前提下加入两级音量控制；预览调整应立即生效，取消必须恢复，应用必须保存，所有数值必须限制在 0% 到 100%。

验收标准：旧格式兼容、越界值钳制、损坏文件回退和新版保存读取测试全部通过。

### 步骤 6：接入游戏主循环

文件：`src/GameApplication.cpp`

修改块：

- 创建并初始化 `SdlAudioManager`，烟雾测试模式不打开真实音频设备。
- 启动时加载目录中的音效；失败仅输出“继续静音运行”。
- 菜单移动、确认、返回、设置预览分别播放 UI（User Interface，用户界面）声音。
- 每帧消费表现事件后，同时交给音频管理器和画面表现系统。
- 返回主菜单、重新开局和退出前停止声道，SDL 退出前释放音频资源。

提示词：

> 把音频作为可失败的表现层能力接入主循环；任何设备或素材错误都不能阻塞游戏，且事件只消费一次并同时服务画面与声音。

验收标准：无音效文件时 `NeonSiege --smoke-test` 仍成功；退出时没有资源泄漏或重复关闭错误。

### 步骤 7：自动化测试

文件：

- `exercises/commercial_audio_catalog.cpp`
- `exercises/commercial_audio_settings.cpp`
- `exercises/commercial_audio_manager.cpp`
- `exercises/day7_presentation_events.cpp`

提示词：

> 使用 SDL 虚拟音频驱动和测试生成的短静音 WAV 验证音频管理器，不依赖真实声卡或商业音效素材；测试临时文件必须清理。

验收标准：音频定向测试、表现事件测试、前端测试和烟雾测试通过，随后完整 Debug 与 Release 回归通过。

## 4. 音效素材清单

以下文件当前需要由项目拥有者提供并登记来源：

```text
assets/audio/ui/select.wav
assets/audio/ui/confirm.wav
assets/audio/ui/back.wav
assets/audio/combat/player_shot.wav
assets/audio/combat/enemy_shot.wav
assets/audio/combat/projectile_hit.wav
assets/audio/combat/player_damaged.wav
assets/audio/combat/enemy_died.wav
assets/audio/combat/reload_start.wav
assets/audio/combat/reload_complete.wav
assets/audio/system/wave_start.wav
assets/audio/system/upgrade_selected.wav
assets/audio/system/victory.wav
assets/audio/system/game_over.wav
```

建议规格：PCM（Pulse-Code Modulation，脉冲编码调制）WAV、16-bit、
44.1 kHz 或 48 kHz、单声道或双声道。短反馈音建议小于 1 秒，胜利和失败提示
建议小于 4 秒。不要提交来源不明、禁止商用或要求未满足的素材。

当前 14 个槽位已全部就位，统一为 **44.1 kHz、16-bit、双声道 PCM**，并且
全部由 `tools/generate_audio_assets.py` 从零确定性合成。2026-09-29 已将菜单
选择、菜单确认、升级确认和胜利四个第三方素材完整替换；生成器不读取、不采样
也不变换旧文件，因此运行时音频集合不存在第三方原始素材再分发问题。来源、
生成函数、实测时长和峰值见 `assets/ASSET_SOURCES.md`。

## 5. 非本地文件的手动操作

### 5.1 获取 SDL_mixer 源码

若 CMake 自动下载在中文路径环境中异常，使用 PowerShell 手动执行：

```powershell
$mixerSource = 'D:\E盘迁移暂存\秋招\NeonSiege-Lab-public\out\dependencies\SDL_mixer'
git clone --depth 1 --branch release-2.8.2 `
    'https://github.com/libsdl-org/SDL_mixer.git' `
    $mixerSource
```

随后配置时加入：

```powershell
-DFETCHCONTENT_SOURCE_DIR_SDL2_MIXER="$mixerSource"
```

验收标准：`git -C $mixerSource log -1 --oneline` 显示 `release-2.8.2`，工作区干净。

### 5.2 获取音效素材

1. 只从明确允许商业使用的来源下载或自行制作。
2. 保存原始下载页、作者、许可证、下载日期和是否要求署名。
3. 转换为上述 WAV 规格并按清单重命名。
4. 将文件放入 `assets/audio/` 对应子目录。
5. 在 `assets/ASSET_SOURCES.md` 中逐项登记，不能只写“免费素材”。

### 5.3 生成项目原创音效

本轮 14 个槽位全部采用自行制作路线，仓库脚本即为唯一来源：

```powershell
python tools/generate_audio_assets.py            # 写入 14 个 WAV
python tools/generate_audio_assets.py --check    # 校验磁盘与生成器一致
```

脚本只依赖 Python 标准库且完全确定，因此提交的 WAV 可以按字节复现；
修改音色应改脚本再重新生成，不要直接编辑 WAV。生成器自行保证：开头无
静音、尾部有淡出、峰值保留设计余量、无削波，并以不同的音高区间、频谱
重心和包络把玩家射击、敌人射击、命中、受击和敌亡区分开。

## 6. 自动验证命令

构建目录为 `out/build/x64-Publish`，使用 Visual Studio 17 2022 生成器，因此
Debug 与 Release 复用同一棵树，不需要另外的构建目录。

```powershell
Set-Location -LiteralPath 'D:\NeonSiege\commercial'

$buildDir = 'out\build\x64-Publish'
$msbuild = 'D:\VS2022\MSBuild\Current\Bin\MSBuild.exe'
$ctest = 'D:\VS2022\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe'

# 构建全部目标。TrackFileAccess=false 是当前沙箱环境下的必要绕行：
# 文件跟踪器经管道捕获 Lib.exe 输出时无法创建子进程
# （TRK0002 / MSB6006: Lib.exe 已退出，代码为 2），静态库对象文件较多的
# SDL2-static 与 freetype 因此失败；关闭增量跟踪后全树构建成功。
# 该参数不修改仓库中的任何文件，只影响本次构建的增量判定。
& $msbuild $buildDir\NeonSiege.sln /p:Configuration=Release `
    "/p:TrackFileAccess=false" /v:m /nologo

# 当前受控执行环境不允许测试程序写入用户配置下的系统临时目录；使用构建树
# 内的中文目录同时验证写权限和 UTF-8 路径兼容性。
$oldTemp = $env:TEMP
$oldTmp = $env:TMP
$testTemp = Join-Path $buildDir 'Testing\中文音频临时目录'
New-Item -ItemType Directory -Force -Path $testTemp | Out-Null

try {
    $env:TEMP = $testTemp
    $env:TMP = $testTemp

    & $ctest --test-dir $buildDir -C Release --output-on-failure `
        -R '^(AudioCatalog|AudioSettings|AudioManager|ReloadEvents|PresentationEvents|FrontendStates|FrontendInput|NeonSiegeSmoke)$'

    # Debug 与 Release 都要跑完整回归
    & $ctest --test-dir $buildDir -C Debug --output-on-failure
    & $ctest --test-dir $buildDir -C Release --output-on-failure
}
finally {
    $env:TEMP = $oldTemp
    $env:TMP = $oldTmp
}
```

### 6.1 音频素材客观检查

三个仓库内 Python 工具用于在人工试听前先把客观指标跑一遍。
`generate_audio_assets.py` 是 14 个原创音效的唯一来源：不要手工编辑这些 WAV，
需要调整时改生成器并重新生成。

```powershell
python tools/generate_audio_assets.py --check   # 与磁盘文件逐字节比对
python tools/audio_verify.py --spectrum         # 格式、峰值、头尾、削波、频段
python tools/audio_verify.py --levels           # 实测响度与目录 baseVolume 对照
python tools/audio_contact_sheet.py --out out\audio_contact_sheet.txt
```

### 6.2 真实素材目录验收（不修改 CMakeLists.txt）

页面内的 `AudioManager` 测试针对的是临时目录里自造的短静音 WAV，覆盖不到
真实素材。`tools/asset_acceptance_harness.cpp` 因此独立编译，链接已经构建好
的 `neon_sdl` 与 SDL 静态库，直接对真实 `assets/` 运行：14 个条目全部可加载、
逐个移走任一文件都只报告该文件缺失、音量钳制与重复关闭安全。损坏文件检查
使用 `out/asset_acceptance/` 下的隔离素材副本，因为 Windows 上失败的
SDL_mixer 加载可能持有文件句柄直至测试进程退出。它不进入 CMake 构建，
所以补齐素材不需要改 `CMakeLists.txt`。

```powershell
powershell -ExecutionPolicy Bypass -File tools\run_asset_acceptance.ps1 `
    -Configuration Release
powershell -ExecutionPolicy Bypass -File tools\run_asset_acceptance.ps1 `
    -Configuration Debug
# 直接跑源码树（脚本会自行还原被临时移走的文件）
powershell -ExecutionPolicy Bypass -File tools\run_asset_acceptance.ps1 `
    -Configuration Release -SourceTree
```

## 7. 手动听觉验收（自动测试通过后执行）

1. 启动 Release 版 `NeonSiege.exe`，确认无音频初始化错误。
2. 在主菜单移动、确认、返回，确认三种 UI 声音可区分且不刺耳。
3. 在设置页把主音量设为 0%，确认所有音效静音；恢复后应立即有声。
4. 把音效音量设为 0%，结果应同样静音；取消设置后应恢复原值，应用后重启仍保留。
5. 手动换弹和弹匣打空后的自动换弹都应先播放开始音，弹药实际填满时再播放完成音；满弹匣或重复换弹输入不得重复播放。
6. 完成一局，逐项确认射击、敌方射击、命中、受伤、敌人死亡、波次开始、升级、胜利和失败反馈。
7. 连续快速射击和连续击中时，确认没有爆音、明显叠音失控或菜单悬停音刷屏。
8. 暂时移走任意一个 WAV 后启动游戏，确认游戏仍可运行并只报告该资源缺失。

通过标准：事件与声音时机一致、音量控制有效、快速事件无明显噪声堆叠、缺失素材可静音降级。

## 8. 当前状态

本节区分“文件已放入目录”“客观检查通过”“人工试听通过”三种状态，不把前两者
写成验收完成。

### 8.1 代码与自动化

- 两段式换弹表现事件与音频映射：已实现；保留既有 `ReloadStarted` /
  `ReloadCompleted` 遥测语义，新增的只是表现事件和声音映射。
- Release 定向验收（AudioCatalog、AudioSettings、AudioManager、ReloadEvents、
  PresentationEvents、FrontendStates、FrontendInput、NeonSiegeSmoke）：
  8/8 通过。
- 完整 CTest 回归：Release 54/54、Debug 54/54 通过，无新增失败。
- 当前受控环境的系统临时目录不可写；上述 CTest 结果使用构建树内的中文
  临时目录，两个音频文件测试均通过，中文路径兼容性同时得到覆盖。
- 真实素材目录验收（`tools/run_asset_acceptance.ps1`）：Release 与 Debug
  均 ACCEPTANCE OK，0 失败；14 项加载、逐项缺失及隔离损坏文件降级均通过。
- 素材客观检查（`tools/audio_verify.py`）：14 个文件全部为
  44.1 kHz / 16-bit / 双声道 PCM，0 削波、0 头静音超标、0 尾部硬截断。
- 生成器可复现性（`tools/generate_audio_assets.py --check`）：14 个原创文件
  与生成器输出逐字节一致。

### 8.2 正式音效素材：14/14 已放入目录

- 项目原创合成（14 个）：菜单选择、菜单确认、菜单返回、玩家射击、敌人射击、
  命中、玩家受伤、敌人死亡、装填开始、装填完成、波次开始、升级确认、胜利、
  游戏失败。来源为 `tools/generate_audio_assets.py`，授权为项目自有，可随公开
  仓库分发。
- 原第三方 UI 音频已被全新合成文件覆盖，未参与采样或变换，运行时目录不再
  包含其音频数据。

### 8.3 尚未解决的风险

1. **黄（待人工试听定性）**：客观响度对照（`audio_verify.py --levels`）
   显示换弹开始与完成的目录有效响度约为 -24.9 / -26.2 dBFS，二者接近但
   频谱重心不同；仍需人耳确认“开始”和“完成”能否在战斗中清楚区分。
   此外 `ProjectileHit` 的实测有效响度比 `PlayerShot` 低约 11 dB，且
   `EnemyDied` 与 `ProjectileHit` 在 50 ms / 35 ms 冷却下多实体同时触发时仍
   可能出现瞬时叠加。是否需要收紧冷却或提高基础音量，必须由人工试听给出
   结论后再改，不允许为降噪而删除表现事件。
### 8.4 人工听觉验收

- 状态：**两段式换弹声音及四个原创替代音效人工试听通过**。
- 2026-09-29，项目拥有者完成菜单选择、菜单确认、升级确认和胜利四个原创替代
  音效的游戏内验收，当前音色、音量与触发时机通过。
- 2026-09-27，项目拥有者在 Release 游戏内试听后反馈“还可以，暂时先这样”。
  这表示当前开始音、完成音及其游戏内组合可以保留，不代表最终混音冻结；后续
  若在更密集的战斗场景中出现遮蔽、刺耳或音量失衡，仍可重新调整。
- 自动化已经证明格式、可加载性、缺件与损坏降级和音量控制逻辑；本次人工
  反馈补充了主观音色判断。第 7 节其余整局听觉项目没有逐项形成单独记录，
  因此不能扩大表述为所有商业音频均已完成最终人工验收。

### 8.5 两段式换弹变更边界

- 核心层：增加 `ReloadStarted` 表现事件，不改遥测、存档或回放格式。
- SDL 层：音效目录由 13 项增至 14 项，并映射开始与完成两个声音。
- 素材层：新增 `reload_start.wav`，重做 `reload_complete.wav`。
- 测试和工具：同步 14 项计数、换弹边界断言、生成器、试听列表以及隔离的
  损坏文件探针。
- `CMakeLists.txt`、`GameApplication.cpp` 与公开设置接口未因两段式换弹修改。

### 8.6 Git

提交与推送不在本轮自动执行范围内。当前工作区仍有未提交改动，包括本轮之前
就存在的音频架构改动。
