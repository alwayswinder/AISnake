# AISnake PRD — 3D 贪吃蛇

技术约束：UE 5.3+ C++ Blank 项目。核心逻辑用 C++，UI 用 UMG，蓝图只做资源绑定和场景配置。

---

## 一、核心需求

### 1. 基础玩法
- 蛇在网格地图中移动，吃到食物后身体增长，分数 +10。
- 撞到自身时游戏结束。
- 撞到障碍物时游戏结束（无敌效果期间除外）。
- 边界不是死亡——蛇头超出边界后从对侧穿出。

### 2. 输入与移动
- 使用传统输入系统，映射 MoveUp / MoveDown / MoveLeft / MoveRight。
- 键位：W/S/A/D 和上下左右方向键。
- 长按任意方向键 0.5 秒后进入加速，松开恢复正常速度。
- 禁止 180° 掉头（防反向）。

> ⚠️ **W/S 方向反转**：UE 默认 Y 轴朝前，若蛇的移动坐标系与相机俯视方向不一致，W（MoveUp）会映射成屏幕向下。需在 `SnakePlayerController` 中统一以屏幕空间（相机朝向）定义上下方向，而非直接使用世界 Y 轴正负。

### 3. 食物系统
| 类型 | 概率 | 效果 |
|---|---|---|
| Normal | 70% | 无特殊效果 |
| Invisible | 10% | 持续 10 秒，可穿过自身和障碍物 |
| Invincible | 20% | 持续 20 秒，撞障碍物则摧毁之，撞自身仍结束游戏 |

每次吃到食物后：
- 蛇身 +1，分数 +10。
- 播放 0.5 秒收集动画（弹跳上升 + 缩放消失）。
- 在新的随机合法位置重新生成食物。
- 额外随机生成 1 个障碍物。

### 4. 障碍物系统
- 每次吃到食物后随机生成一个障碍物。
- 生成位置不能与蛇身、食物、已有障碍物重叠。

### 5. 存档与排行榜
- 游戏结束自动保存分数。
- 使用 SaveGame 维护 Top 10 高分，按降序排列。
- 存档槽：`SnakeHighScores`，UserIndex = 0。

### 6. UI 与游戏流程
```
启动 → 显示菜单 UI（开始按钮 + 高分榜）
点击开始 → 隐藏菜单 UI，显示游戏 UI，生成蛇和食物
游戏中 → 游戏 UI 实时显示当前分数
游戏结束 → 隐藏游戏 UI，显示菜单 UI（本次分数 + Top 10 榜单）
```

> ⚠️ **菜单无鼠标光标**：显示菜单 Widget 时必须同步设置 `SetInputMode(FInputModeUIOnly)` 并调用 `SetShowMouseCursor(true)`；进入游戏时切回 `FInputModeGameOnly` 并隐藏光标。否则菜单按钮无法点击。

### 7. 颜色与材质
使用无光照纯颜色材质，C++ 按状态切换，蓝图子类负责赋材质资产。

| 对象 | 状态 | 颜色 |
|---|---|---|
| SnakeObstacle | 默认 | 红色 |
| Food | Normal | 橙色 |
| Food | Invisible | 蓝色 |
| Food | Invincible | 紫色 |
| Snake | Normal | 橙色 |
| Snake | Invisible 效果期间 | 蓝色 |
| Snake | Invincible 效果期间 | 紫色 |

### 8. 验收标准
- [ ] 可从空项目编译并运行
- [ ] 能开始游戏
- [ ] 蛇能移动、加速、吃食物、增长
- [ ] 能触发隐身 / 无敌效果
- [ ] 能生成障碍物
- [ ] 边界可环绕
- [ ] 碰撞逻辑正确
- [ ] 游戏结束流程正确
- [ ] Top 10 高分可保存和读取

---

## 二、代码实现

### 1. C++ 类一览

| 类 | 基类 | 职责 |
|---|---|---|
| `SnakeSaveGame` | `USaveGame` | 存储 Top 10 高分列表 |
| `SnakeSegment` | `AActor` | 单个蛇身节，跟随前一节移动 |
| `SnakeObstacle` | `AActor` | 静态障碍物 |
| `Food` | `AActor` | 食物（含 `EFoodType` 枚举） |
| `Snake` | `APawn` | 移动 Tick、方向队列、防反向、蛇身管理、边界环绕、碰撞、加速、隐身/无敌 |
| `SnakePlayerController` | `APlayerController` | 接收输入，将方向传给 Snake |
| `SnakeGameUI` | `UUserWidget` | 游戏中 HUD：实时分数 |
| `SnakeMenuUI` | `UUserWidget` | 菜单：开始按钮 + 高分榜 |
| `SnakeHUD` | `AHUD` | 创建并管理两个 Widget |
| `SnakeManager` | `AActor` | 核心管理器（见下方详述） |
| `SnakeGameMode` | `AGameModeBase` | 设置默认 PlayerController 和 HUD |

### 2. SnakeManager 详细职责
- 管理分数、游戏状态机（Menu / Playing / GameOver）。
- 生成 / 销毁 Snake、Food、Obstacle；维护合法随机生成位置。
- 重新开始时先销毁所有旧对象，再重新生成，防止残留。

> ⚠️ **重启未清理旧对象**：`RestartGame` 中必须遍历并调用 `Destroy()` 清除上一局所有 Snake、SnakeSegment、Food、Obstacle 实例，再重置分数和状态。若只重置引用指针而不调用 `Destroy()`，旧 Actor 仍在场景中，会导致碰撞误判和内存泄漏。

- **地面可视化**：场景内放置一个 Plane，大小与 GridSize 完全一致，使用网格线材质（引用存储在成员变量中，蓝图子类赋值）。

> ⚠️ **边界 / Plane / 相机不匹配**：GridSize（逻辑格数 × 格子尺寸）、Plane 的 Scale、相机 OrthoWidth（或 FOV 对应的可视范围）三者必须用同一个 `GridSize` 常量推导，不得硬编码独立数值。建议在 SnakeManager 中定义 `GridCellSize` 和 `GridCount` 两个变量，其余所有计算均引用这两个量。

- **相机**：高度与 GridSize 关联，俯视整个游戏区域；FOV / OrthoWidth 略大于地图边界；Possess 后不得发生视角跳变，相机朝向与玩家操作方向一致。

> ⚠️ **Possess 后相机错位**：`PlayerController::Possess(Snake)` 会重置 Controller 的 ControlRotation，导致绑定在 Snake 上的 SpringArm / Camera 发生跳变。解决方案：将相机挂载在 SnakeManager（或独立 CameraActor）上而非 Snake Pawn 上，Possess 后手动调用 `SetViewTarget` 或 `SetViewTargetWithBlend` 指向固定相机，彻底隔离 Possess 对视角的影响。

- 负责调用 SaveGame 的读写逻辑。

### 3. 构建配置

**AISnake.Build.cs** 依赖模块：
```
Core, CoreUObject, Engine, InputCore, EnhancedInput, UMG, Slate, SlateCore
```

**Config/DefaultInput.ini** — 传统输入映射：
```ini
+ActionMappings=(ActionName="MoveUp",    Key=W,    bShift=False,bCtrl=False,bAlt=False,bCmd=False)
+ActionMappings=(ActionName="MoveUp",    Key=Up,   bShift=False,bCtrl=False,bAlt=False,bCmd=False)
+ActionMappings=(ActionName="MoveDown",  Key=S,    bShift=False,bCtrl=False,bAlt=False,bCmd=False)
+ActionMappings=(ActionName="MoveDown",  Key=Down, bShift=False,bCtrl=False,bAlt=False,bCmd=False)
+ActionMappings=(ActionName="MoveLeft",  Key=A,    bShift=False,bCtrl=False,bAlt=False,bCmd=False)
+ActionMappings=(ActionName="MoveLeft",  Key=Left, bShift=False,bCtrl=False,bAlt=False,bCmd=False)
+ActionMappings=(ActionName="MoveRight", Key=D,    bShift=False,bCtrl=False,bAlt=False,bCmd=False)
+ActionMappings=(ActionName="MoveRight", Key=Right,bShift=False,bCtrl=False,bAlt=False,bCmd=False)
```

### 4. 关键实现要点
- **移动**：基于网格 Tick，每个 Tick 移动一个格子，Tick 间隔决定速度。
- **方向队列**：每帧最多入队一次方向变更，Tick 时消费队首，防止同帧多次反向。
- **边界环绕**：蛇头位置取模（`FMath::Fmod`）映射回合法网格范围。
- **碰撞检测**：Tick 移动后检测头部格子是否与蛇身节、障碍物、食物坐标重合。
- **加速**：`BindAction` 的 `IE_Pressed` / `IE_Released` 计时；持续按下 ≥ 0.5 秒则缩短 Tick 间隔。
- **隐身/无敌**：Snake 内部维护 `bIsInvisible` / `bIsInvincible` 布尔和剩余时间；碰撞检测逻辑分支读取这两个标志。
- **材质切换**：在 Snake、Food、SnakeObstacle 中声明 `UMaterialInterface*` 成员变量（`UPROPERTY(EditDefaultsOnly)`），状态变更时调用 `MeshComponent->SetMaterial(0, ...)` 切换。

> ⚠️ **游戏结束崩溃（重复绑定）**：`SnakeHUD` 或 `SnakeMenuUI` 的开始按钮 `OnClicked` 若在每次 `ShowMenu()` 时都调用 `AddDynamic` 绑定，重启多次后会重复触发导致二次初始化崩溃。解决方案：按钮绑定只在 Widget 初始化（`NativeConstruct`）时执行一次，或绑定前先调用 `RemoveDynamic` 解绑旧回调。

> ⚠️ **道具特效下新增 Segment 材质错误**：Invisible / Invincible 效果期间吃食物新增的 `SnakeSegment`，若在 `AddSegment()` 中直接使用默认材质（Normal），会出现颜色不一致。正确做法：`AddSegment()` 完成后立即检查 Snake 当前状态，并对所有 Segment（含新增的）调用一次统一的 `ApplyCurrentEffectMaterial()` 方法同步颜色。

---

## ⚠️ 易错 Bug 速查

| # | Bug | 位置 | 根因 | 解决方案 |
|---|---|---|---|---|
| 1 | Possess 后相机错位 | SnakeManager / Camera | Possess 重置 ControlRotation | 相机挂 SnakeManager，Possess 后 `SetViewTarget` 固定相机 |
| 2 | 边界/Plane/相机不匹配 | SnakeManager | 三处数值独立硬编码 | 统一由 `GridCellSize × GridCount` 推导，禁止硬编码 |
| 3 | W/S 方向反转 | SnakePlayerController | 世界 Y 轴与屏幕上方不一致 | 以相机朝向定义方向，而非直接使用世界轴 |
| 4 | 重启未清理旧对象 | SnakeManager::RestartGame | 只重置指针未调用 `Destroy()` | 重启前遍历调用 `Destroy()`，再清空数组/指针 |
| 5 | 菜单无鼠标光标 | SnakeHUD / PlayerController | 未切换 InputMode | 显示菜单时 `SetInputModeUIOnly + ShowMouseCursor(true)` |
| 6 | 游戏结束崩溃（重复绑定） | SnakeMenuUI / SnakeHUD | `AddDynamic` 多次绑定同一回调 | 按钮绑定仅在 `NativeConstruct` 执行一次 |
| 7 | 新增 Segment 材质错误 | Snake::AddSegment | 新 Segment 使用默认材质 | `AddSegment` 后调用 `ApplyCurrentEffectMaterial()` 同步所有节颜色 |

---

## 三、编辑器配置

### 1. 蓝图资产清单

| 蓝图资产 | 父 C++ 类 | 必要配置 |
|---|---|---|
| `BP_SnakeGameMode` | `SnakeGameMode` | HUD Class = `BP_SnakeHUD` |
| `BP_SnakeHUD` | `SnakeHUD` | MenuWidgetClass = `WBP_SnakeMenuUI`，GameWidgetClass = `WBP_SnakeGameUI` |
| `WBP_SnakeMenuUI` | `SnakeMenuUI` | 绑定开始按钮点击事件，绑定分数/榜单文本 |
| `WBP_SnakeGameUI` | `SnakeGameUI` | 绑定当前分数文本 |
| `BP_SnakeManager` | `SnakeManager` | 赋地面网格线材质，设置 SnakeClass / FoodClass / ObstacleClass / SegmentClass |
| `BP_Snake` | `Snake` | 赋 Normal / Invisible / Invincible 三种材质 |
| `BP_Food` | `Food` | 赋 Normal / Invisible / Invincible 三种材质 |
| `BP_SnakeSegment` | `SnakeSegment` | 赋默认材质 |
| `BP_SnakeObstacle` | `SnakeObstacle` | 赋红色材质 |

### 2. 材质资产
在 Content Browser 中创建以下无光照纯色材质（Unlit，Shading Model = Unlit）：

| 资产名 | 颜色 |
|---|---|
| `M_Orange` | 橙色 |
| `M_Blue` | 蓝色 |
| `M_Purple` | 紫色 |
| `M_Red` | 红色 |
| `M_GridFloor` | 网格线（半透明或线框，用于地面） |

### 3. 地图与场景设置

1. 新建关卡，保存为 `M_Start`。
2. **World Settings → GameMode Override** 设置为 `BP_SnakeGameMode`。
3. 在场景中放置 `BP_SnakeManager` 实例（任意位置，建议原点）。
4. 在 `BP_SnakeManager` 的 Details 面板中完成以下赋值：
   - `SnakeClass` → `BP_Snake`
   - `FoodClass` → `BP_Food`
   - `ObstacleClass` → `BP_SnakeObstacle`
   - `SegmentClass` → `BP_SnakeSegment`
   - `FloorMaterial` → `M_GridFloor`

### 4. 项目设置
- **Project Settings → Maps & Modes → Default GameMode** 可留空（由关卡 World Settings 覆盖）。
- **Project Settings → Maps & Modes → Game Default Map** 设置为 `M_Start`。
- 确认 `DefaultPlayerInputClass` 和 `DefaultInputComponentClass` 已在 `DefaultInput.ini` 中指向 Enhanced Input 类（与传统 Action Mapping 共存，不冲突）。
