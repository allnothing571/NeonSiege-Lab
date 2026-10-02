# Neon Siege 轻肉鸽升级 第一部分：核心选择与受控强化选项生成

本文件记录第一阶段（选项生成与核心选择）的实现范围、验收结果与遗留事项。

## 1. 本阶段目标

为游戏建立一套与旧升级系统并存的轻肉鸽构筑骨架：

- 一局开始时从三个核心中选一个；
- 每次升级从三张强化卡中选一张；
- 强化卡分「专属」与「通用」两类，专属只对所属核心生效；
- 只生成选项、记录构筑、算出参数预览，不改动任何战斗数值。

完成标准：

- 3 种核心、12 种强化卡，共 15 种目录条目的定义完整、无重复、可往返查询；
- 核心只能选一次，强化必须先有核心；
- 候选池按「专属 / 输出续航 / 防御操控」分组，满级与不属于当前核心的卡被过滤；
- 补位从完整剩余通用池均匀随机抽取，不偏向枚举靠前的卡；
- 候选不足 3 个时返回真实数量，池子耗尽时返回 false 且不破坏已有构筑；
- 非法生成调用与同类型重复生成都返回原面板，不推进随机引擎、不改动构筑；
- 同一随机种子下结果可复现，重复调用不会隐式重抽；
- 核心候选与强化候选都支持完整参数预览，预览与实际选择后的结果完全一致，
  且与取得顺序无关；被拒绝的预览不改动输出参数；
- 旧升级编号、回放格式、最高分与设置文件一律不变。

## 2. 架构边界

本阶段只新增核心层模块，运行时的调用链保持不变：

`Simulation -> PresentationEvent -> GameApplication -> SdlAudioManager -> SDL_mixer`

肉鸽模块自身不接入上述任何一环：

- 新模块位于 `src/core/`，不依赖图形、音频或 SDL 库；
- 旧 `UpgradeSystem` 原样保留，`Simulation.cpp` 仍只调用旧系统；
- 没有任何既有源文件（`src/`、`exercises/`）引用 `Roguelite*` 标识符；
- 正式游戏不会展示本阶段尚未生效的核心与专属强化。

设计规则：

- `src/core/RogueliteUpgrade*` 只描述「有哪些选项、选了什么、参数会变成多少」；
- 数值不写入 `Player`，`ArmorCore` 的一次性回血只通过
  `RogueliteApplyResult::restoredHealth` 上报，由后续战斗实现去施加；
- 伤害倍率保持浮点，取整是后续战斗实现的责任。

## 3. 本地文件修改清单

行号以本次实现完成后的文件为准；后续编辑造成行号漂移时，以所列函数或功能块为定位依据。

### 新增文件

| 文件 | 作用 |
| --- | --- |
| `src/core/RogueliteUpgradeTypes.h` | 核心/强化/分类枚举、候选结构、参数结构、调参常量 `RogueliteTuning` |
| `src/core/RogueliteUpgradeCatalog.h` / `.cpp` | 12 张强化卡与 3 个核心的定义表、合法性查询、归属与分类查询 |
| `src/core/RogueliteUpgradeSystem.h` / `.cpp` | 核心选择、候选生成、等级、参数预览、重置与计数 |
| `exercises/roguelite_test_support.h` | 两个测试共用的断言宏与失败计数 |
| `exercises/commercial_roguelite_catalog.cpp` | 目录完整性测试（6 个用例） |
| `exercises/commercial_roguelite_progression.cpp` | 进程与构筑测试（27 个用例） |

### 修改文件

`CMakeLists.txt` 三处：

- `neon_core` 的 `target_sources`：在 `src/core/UpgradeSystem.cpp` 与
  `src/core/WaveDifficulty.h` 之间加入 5 个新模块文件；
- 测试目标区：在 `CommercialWaveDifficulty` 之后新增
  `CommercialRogueliteCatalog` 与 `CommercialRogueliteProgression`，
  两者只链接 `neon_core`，MSVC 下带 `/W4 /permissive- /utf-8`；
- `BUILD_TESTING` 区：在 `WaveDifficulty` 之后注册
  `add_test(NAME RogueliteCatalog ...)` 与
  `add_test(NAME RogueliteProgression ...)`。

### 明确未修改

`src/core/PresentationEvent.h`、`src/core/Simulation.cpp`、
`src/core/UpgradeSystem.h`/`.cpp`、`src/core/UpgradeType.h`、
`src/core/GameplayConfig.h`、`src/sdl/` 全部文件、`src/GameApplication.cpp`
在本阶段没有改动，`git diff --name-only` 只列出 `CMakeLists.txt`。

## 4. 内容清单

### 4.1 三个核心

按固定顺序提供（`rogueliteCoreOfferOrder()`），核心面板不消耗随机数：

1. 聚焦射击 `FocusFire`：连续命中同一目标叠加伤害加成，基础每层 +10%、最多 3 层、1.5 秒未命中失效。
2. 贯穿弹道 `PiercingRounds`：命中后继续穿透，首个目标 ×1.10，最多多打 1 个目标，额外目标 ×0.60。
3. 电弧联动 `ArcLink`：每 3 次普通命中放电一次，主目标 ×0.40，最多连锁 2 个目标，连锁倍率 ×0.40，半径 150 像素。

### 4.2 12 张强化卡

| 分类 | 卡 | 上限 | 效果 |
| --- | --- | --- | --- |
| 专属（聚焦射击） | 深度聚焦 `DeepFocus` | 1 | 每层伤害加成改为 +15%，仍为 3 层 |
| 专属（聚焦射击） | 锁定动能 `LockMomentum` | 1 | 击杀后保留层数，额外 1.5 秒 |
| 专属（贯穿弹道） | 深度贯穿 `DeepPenetration` | 1 | 最多多打 2 个目标 |
| 专属（贯穿弹道） | 贯穿增幅 `PenetrationAmplifier` | 1 | 额外目标倍率提高到 ×0.85 |
| 专属（电弧联动） | 延伸回路 `ExtendedCircuit` | 1 | 最多连锁 3 个目标 |
| 专属（电弧联动） | 集中放电 `ConcentratedDischarge` | 1 | 主目标倍率提高到 ×0.80 |
| 输出续航 | 伤害强化 `DamageBoost` | 2 | 每级伤害倍率 +15% |
| 输出续航 | 射速提升 `FireRate` | 2 | 射击间隔变为 基础射击间隔 / (1 + 0.10 × 等级) |
| 输出续航 | 弹药系统 `AmmoSystem` | 2 | 每级弹匣 +3、装填 −0.10 秒 |
| 防御操控 | 装甲核心 `ArmorCore` | 2 | 每级最大生命 +15，选择时立即恢复 15 点 |
| 防御操控 | 机动校准 `MobilityCalibration` | 2 | 每级移动速度 +8% |
| 防御操控 | 弹道校准 `BallisticCalibration` | 2 | 每级散布 −1 度、弹速 +60 |

### 4.3 面板生成规则

- 槽 A：当前核心可用的专属卡（若有），专属最多占这一个槽位；
- 槽 B：输出续航类通用卡；
- 槽 C：防御操控类通用卡；
- 剩余空位：从**完整剩余通用池**均匀随机抽取，抽取后移除，直到填满或池子耗尽；
  收集候选时**不按空余槽位提前停止**，因此补位不偏向枚举靠前的卡；
- 只对已填满的部分做 `std::shuffle`；
- 已满级的卡与不属于当前核心的专属卡在收集阶段就被排除，因此候选池是
  **完整剩余池**，不按空余槽位提前停止，补位不偏向枚举靠前的卡；
- 保障槽（专属 / 输出续航 / 防御操控）从各自的容器抽取，**不会**从通用池里
  删除对应卡；因此在保障槽抽完之后、补位之前，先从通用池里删除所有已经出现在
  本轮面板中的卡，再对剩余池做均匀随机不放回抽取；否则补位可能重复给出保障槽
  刚放入的那张卡；
- 候选不足 3 个时返回真实数量；池子耗尽时返回 false，且此时面板已清空。

B/C 的先后顺序固定为先 B（输出续航）后 C（防御操控）。

### 4.4 生成调用的拒绝与重复契约

- 同类型面板已挂起时，再次调用返回 `true` 并**原样返回已有面板**，不重抽；
- 已有核心时调用核心生成返回 `false`；尚无核心时调用强化生成返回 `false`；
- 以上拒绝都在**清空面板或改动任何状态之前**判定，因此原面板、构筑、等级、
  计数与最近选择记录全部保持不变，随机引擎也不被推进；
- 面板只在抽取确定会发生时才清空，成功选择后正常清空且不能重复选核心。

### 4.5 参数预览

`statsAfter(config, optionIndex, result)` 支持两类候选：

- 强化候选：用当前核心 + 该卡临时提升一级计算完整参数；
- 核心候选：用候选核心 + 当前已有等级计算完整参数，因为选择核心会激活其加成结构；
- 两条路径都不写入构筑、不消耗随机数；
- 索引非法或候选不可选择时返回 `false`，且 `result` 的所有字段保持原值。

已确认的退化行为：由于专属只占一个槽位，当通用卡全部满级而仍有专属没拿时，
面板会退化为 1 个选项（不是把 3 个槽位都填满专属）。

## 5. 自动化验收

构建方式（本机已验证）：

```
cmake --build out\build\x64-Publish --config <Debug|Release> --target <目标>
```

沙箱环境下文件跟踪器会干扰 `Lib.exe`，需要直接调用 MSBuild 并关闭文件跟踪：

```
MSBuild.exe out\build\x64-Publish\NeonSiege.sln /p:Configuration=Release /p:Platform=x64 "/p:TrackFileAccess=false" /v:m /nologo
```

该方式不修改仓库内任何文件，只影响本次构建命令行。

### 5.1 实测结果

#### 5.1.1 最终自动化验收（由项目所有者在本机执行，全部通过）

| 阶段 | 配置 | 结果 | 退出码 | 用时 |
| --- | --- | --- | --- | --- |
| 完整构建 | Debug | 成功 | 0 | — |
| 目录测试 `CommercialRogueliteCatalog` | Debug | 通过 | 0 | — |
| 进程测试 `CommercialRogueliteProgression` | Debug | 通过 | 0 | — |
| 完整回归 CTest | Debug | 56/56 通过 | 0 | 24.76 秒 |
| 完整构建 | Release | 成功 | 0 | — |
| 目录测试 `CommercialRogueliteCatalog` | Release | 通过 | 0 | — |
| 进程测试 `CommercialRogueliteProgression` | Release | 通过 | 0 | — |
| 完整回归 CTest | Release | 56/56 通过 | 0 | 22.16 秒 |

完整回归在项目内临时目录 `Testing\RogueliteTemp` 下执行，执行结束后已恢复
`TEMP`、`TMP` 环境变量到执行前的值。

#### 5.1.2 历史记录：最终修正之前发生过的失败（已修复，不再是待解决故障）

以下两次失败都发生在**本轮最终修正之前**，其对应的缺陷与测试错误均已在本轮
修正（见 6.2、6.3），**不是当前遗留故障**，保留在此仅作为过程记录：

| 项目 | 当时的实测结果 |
| --- | --- |
| 构建配置 | Debug |
| `CommercialRogueliteProgression` | **共 158 条断言失败，退出码 1** |
| `levelUpAllGenerals()` | 一次「跨容器迭代器」断言：`generalCards().begin()` 与 `generalCards().end()` 来自两个不同的临时容器；已改为先从同一个存活容器取出迭代器 |

**158 条失败的归因**：这个数字**不是 158 个独立缺陷**。它来自同一批根因的连锁断言，
其中包含一个真实实现缺陷——补位池在保障槽抽取后没有移除本轮已选卡，导致面板出现
重复选项；以及若干**测试准备或期望错误**——专属槽位数量（违反「专属最多占一个
槽位」还把剩余专属填满多个槽位）、核心选择（对 `applyOffer(1)` 期待了错误的核心）、
选择计数（`total()` 已含核心选择却期待为 0 或 `step + 1`）和单级参数（用升到满级的
`takeCard` 去期待单级 1.15）。同一个根因会牵连到后续每一条断言，因此断言数量被放大。
各项已在 6.2、6.3 逐条记录。

原先把本节写成「PASSED」是错的，此处按实测结果更正。

**关于最初三处问题的实测证据**：本阶段**没有**取得「最初三处问题修正之前」的
失败输出作为证据——测试文件当时尚未跟踪，一次误操作把它截断，原始失败记录已丢失。
因此本节只保留上述两次真实发生的失败记录，不虚构更早的失败样例。

### 5.2 完整回归基线与预期规模

```
ctest --test-dir out\build\x64-Publish -C <Debug|Release> --output-on-failure
```

改动前基线为 **54 项全部通过**；本次注册 `RogueliteCatalog` 与
`RogueliteProgression` 后完整规模为 **56 项**，两配置实测均为 56/56。本阶段不新增
测试目标，完整测试数恒为 56。其余项目包括 `UpgradeSystem`、`WaveDifficulty`、
`PresentationEvents`、`FrontendStates`、`FrontendInput` 与 `NeonSiegeSmoke`。

完整回归在项目内可控临时目录 `Testing\RogueliteTemp` 中执行，不使用系统默认
临时目录；执行结束后 `TEMP`、`TMP` 环境变量已恢复原值。

### 5.3 `commercial_roguelite_catalog.cpp` 覆盖的用例（6）

目录构成、核心提供顺序、强化目录构成、专属归属、非法标识、
分类名与语言无关。

### 5.4 `commercial_roguelite_progression.cpp` 覆盖的用例（27）

原有 23 个用例：初始核心三选一与固定顺序、核心生成不消耗随机数、未选核心拒绝强化、
核心只能选一次、三个核心的候选池分离与归属、满级通用卡被过滤、
专属卡不重复出现、25 个种子下面板保证「专属/输出/防御各 1」、
专属耗尽后的降级、通用耗尽后由剩余专属补位、全部耗尽时返回 false 且构筑不变、
同种子可复现且重复生成不隐式重抽、非法调用被拒绝且不污染状态、
基础参数、各等级参数（含单级 1.15）、最终参数与取得顺序无关、
三套核心参数（基础与两个专属独立及组合）、预览与选择后完全一致、
`ArmorCore` 每次选择只回血一次、连续六次强化不提前停止、
`reset` 清空核心/等级/面板/计数、非法卡片查询安全、`lastApplied` 记录。

新增 4 个用例：

- `checkFullPoolReferenceTopUp`：两张专属取得后六张通用卡均未取得的补位场景，
  以及任一通用分类耗尽、需要补多个槽位的场景；用固定种子与独立的完整池参考抽样
  逐个对照候选与随机引擎状态，不使用抽样频率阈值，也不只检查面板数量。
- `checkGenerationPreservesPendingState`：同类型重复生成与异类型拒绝调用前后，
  比较全部可查询状态与引擎状态，并确认被拒绝后原面板仍能预览并正常选择。
- `checkEveryCorePreview`：三个核心分别验证预览与实际选择后完整参数一致，
  覆盖全部基础参数与三个核心效果结构的所有字段。
- `checkEveryStrengtheningPreview`：覆盖全部 12 张强化卡及各可升级等级，
  比较全部基础参数与三个核心效果结构的每个字段（启用标志、倍率、目标数量、
  时间、半径），浮点字段使用容差。

### 5.5 验收状态

**独立肉鸽模块通过当前自动化规则验收。**

依据是本轮最终实测：Debug 与 Release 均完整构建成功（退出码 0），两个配置的
`CommercialRogueliteCatalog`（目录测试）与 `CommercialRogueliteProgression`
（进程测试）退出码均为 0，完整 CTest 两配置各 56/56 通过、退出码 0
（Debug 24.76 秒、Release 22.16 秒），完整回归在项目内 `Testing\RogueliteTemp`
下执行且 `TEMP`、`TMP` 已恢复。

这条结论的**边界**：它只覆盖独立模块自身的规则（目录完整性、候选生成、
拒绝与重复契约、参数预览、等级与计数）。**真实玩法接入、旧升级系统迁移、
玩法多样性、数值平衡与人工手感验收都仍未完成**，因此不能说「实际玩法升级
已完成」，也不能说本阶段已经端到端验收通过。

注意：`out/build/x64-Publish/{Debug,Release}/CommercialRogueliteProgression.exe`
的时间戳早于本轮源码改动，对照本节结论前必须重新构建，不能用旧可执行文件的结果
代替构建结果。

## 6. 实现过程中发现并修正的缺陷

以下三处都在 `src/` 的真实代码里，由测试暴露后修正：

1. `RogueliteUpgradeSystem::generateCoreOffers(std::mt19937&)` 的未引用形参触发
   `warning C4100`，签名改为 `[[maybe_unused]] std::mt19937& randomEngine`。
2. `RogueliteUpgradeCatalog.cpp` 的合法性判断原先只比较 `None` / `Count` 哨兵，
   导致 `static_cast<RogueliteStrengtheningType>(999)` 被判为合法；改为范围检查。
3. **关键缺陷**：`generateStrengtheningOffers` 把遗留的核心面板误判为「已挂起的强化面板」，
   于是返回陈旧的核心面板并允许第二次选核心；`offersArePending` 改为带
   `RogueliteOptionKind` 参数，两类面板分别判断。

测试侧另有两处错误期望值被修正（不是库缺陷）：
`ArcLink` 下可构筑卡是 8 张（6 通用 + 2 专属）而不是 5 张；
把全部可构筑卡拉满需要 14 次选择（6 通用 ×2 + 2 专属 ×1）而不是 8 次。

### 6.1 验收缺口修正（本轮）

1. **补位偏差**：`generateStrengtheningOffers` 原先在收集候选时就按空余槽位提前
   停止，补位因此偏向枚举靠前的通用卡；改为先收集完整剩余池，再用
   `takeRandom` 均匀抽取并移除，直到填满或耗尽。
2. **非法调用清空面板**：原先只有强化生成会把面板清空，且部分拒绝路径在状态改动
   之后才判定；现在两类生成都在**改动任何状态之前**完成合法性判定，同类型重复
   生成原样返回已有面板，异类型调用返回 `false` 且不推进随机引擎。
3. **核心候选没有参数预览**：`statsAfter` 原先对核心候选返回 `false`，与
   「选择核心会激活其加成结构」的实际行为不符；现在核心候选会返回使用候选核心
   与当前等级算出的完整预览，头文件中「核心不改变战斗参数」的错误注释已删除。

### 6.2 项目所有者实测暴露的补位缺陷（本轮修正）

那次 Debug 进程测试共 **158 条断言失败**，其中包含一个真实实现缺陷（也是本节主题）：
`generateStrengtheningOffers` 收集通用池时**面板尚未生成**，此时检查「是否已在
本轮面板中」恒为假；而输出续航与防御操控两个保障槽使用各自独立的容器，
`takeRandom` 不会从通用池里同步删除对应卡。结果是保障槽刚放入的卡仍留在通用池里，
补位时可能再次抽中同一张卡，面板出现重复选项。

**158 条不是 158 个独立缺陷**：同一批根因会牵连后续的每一条断言，数字因此被放大。
除本节的补位重复之外，另一半来自测试准备或期望错误——专属槽位数量、核心选择、
选择计数、单级参数，逐条见 6.3。

修正做法：保留完整通用候选池 → 专属、输出续航、防御操控保障槽照常抽取 →
从通用池中删除所有已经出现在本轮面板中的卡 → 对剩余池做均匀随机不放回补位。
原注释「收集阶段已删除本轮已选卡」是错的，已改写；`checkFullPoolReferenceTopUp`
的完整池参考抽样与「不重复」断言均未放宽。

### 6.3 项目所有者实测暴露的测试错误（本轮修正）

1. `checkCategoryExhaustionTopsUpFromRemainingCards` 原先期待剩余专属填满两个
   槽位，违反「专属最多占一个槽位」；改为准备阶段只取通用卡把六张通用升满、
   确认两张专属仍未取得，生成后断言只有**一个**专属选项，取走后再断言另一张
   仍可生成，两张都取走后池子才耗尽。
2. `checkDeterminismAndNoImplicitReroll` 原先在已选过核心的情况下期待
   `selectionCounts().total()` 为 0；改为先保存重复生成前的计数与全部状态，
   再验证前后相等，并分别断言 `coreSelections == 1`、
   `strengtheningSelections == 0`。
3. `checkIllegalCallsAreRejected` 原先对 `applyOffer(1)` 期待 FocusFire，
   而固定核心顺序的第二项是 PiercingRounds；改为选择前记录该槽位实际提供的核心，
   选择后与拒绝重复选择后都与记录值比较，未改动核心目录顺序。
4. `checkGeneralParameterLevels` 的单级子场景原先用 `takeCard` 把卡升到满级，
   却期待单级倍率 1.15；改用 `takeOneLevel`，先断言 DamageBoost 等级恰为 1，
   再验证倍率 1.15，没有把单级预期改成 1.30。
5. `checkSixStepSequenceDoesNotStopEarly` 原先期待 `total()` 为 `step + 1`，
   但 `total()` 包含开局的核心选择；现在每次断言 `coreSelections == 1`、
   `strengtheningSelections == step + 1`、`total() == step + 2`。
6. `checkArmorRestoreHappensOncePerSelection` 原先在循环结束后硬写
   `CHECK_TRUE(..., false)`，只要 ArmorCore 没在 24 轮内出现就必然失败；
   改为在循环外比较「实际恢复量」与 `armorCoreRestoredHealth`，仍是真实断言，
   且失败信息能区分「没出现」与「恢复量不对」。
7. `levelUpAllGenerals()` 中的
   `const std::set<...> wanted{generalCards().begin(), generalCards().end()}`
   让两个迭代器分别来自两个不同的临时容器（调试迭代器检查下的断言）；改为先把
   `generalCards()` 的返回值存入 `const auto cards`，再用 `cards.begin(),
   cards.end()` 构造，保证两个迭代器来自同一个存活容器。同一文件与 `exercises/`
   全目录已只读复查，没有其他同类写法；调试迭代器检查未被关闭。
8. `checkFullExhaustionIsHandled` 原先逐张 `levelUpCardFully` 后用
   `if (everyCardMaxed)` 把不满足时的断言整个跳过，卡没满级时会**静默通过**；
   改为用 `levelUpAllCards(system, eligibleCards(system), ...)` 确定性地拉满，
   断言 `roundsUsed == 14`（6 通用 ×2 + 2 专属 ×1），并补一条
   「耗尽检查确实在满级构筑上运行」的断言。

**关于最初三处问题的实测证据**：本阶段**没有**取得「最初三处问题修正之前」的
失败输出作为证据——测试文件当时尚未跟踪，一次误操作把它截断，原始失败记录已丢失。
因此文档只保留项目所有者实测的 158 条断言失败与跨容器迭代器断言这两条真实证据，
不虚构更早的失败样例。

## 7. 当前状态

- 核心与选项生成：**独立肉鸽模块通过当前自动化规则验收**。验收缺口（补位偏差、
  非法调用保留面板、核心预览、跨容器迭代器）已全部修正；本轮最终实测为
  Debug 与 Release 完整构建退出码 0、目录测试与进程测试退出码 0、
  完整 CTest 两配置各 56/56 通过（Debug 24.76 秒、Release 22.16 秒）。
- 战斗接入（聚焦伤害、贯穿碰撞、电弧伤害）：**未开始**，本阶段有意不接入。
- 旧升级系统迁移：**未开始**，旧系统保持原样，迁移要等战斗效果完成之后。
- 玩法多样性、数值平衡、人工手感验收：**未完成**；本阶段无可操作入口，未执行
  人工验收，等战斗接入后再做实际手感验收。
- 本阶段未自动提交或推送任何改动。
- 明确区分：「独立肉鸽模块通过当前自动化规则验收」**不代表真实玩法已接入并验证**，
  也不能表述为实际玩法升级已经完成。

## 8. 遗留风险

- 参数公式目前只被测试覆盖，尚未在任何真实战斗中验证过手感与平衡；
  `RogueliteTuning` 里的数值都是初稿。
- 卡池的多样性、平衡性与战斗手感均未验证。
- `RogueliteStats` 中的伤害倍率与三项核心加成结构与后续战斗实现绑定，
  接入时若发现字段不够，需要在本阶段之外扩展。
- 每个核心可构筑 8 种强化卡（6 通用 + 2 专属），全部拉满需要 14 次强化；
  首版计划每局 6 次强化，因此 14 次耗尽不是当前容量阻塞项。
