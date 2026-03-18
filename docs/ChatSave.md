# AISnake 项目开发对话记录

> 保存时间：2026-03-18

---

## 对话一：生成 Copilot Instructions

**用户请求：** 分析代码仓库，创建 `.github/copilot-instructions.md`。

**执行内容：**
- 遍历项目结构：`Source/`、`Plugins/`、`Config/`、`docs/`
- 读取 `docs/SnakePrd.md`、`Plugins/McpAutomationBridge/AGENTS.md`、`Plugins/McpAutomationBridge/README.md`、`Plugins/SpecialAgentPlugin-main/README.md`
- 分析 `AISnake.uproject`（UE 5.7）、`AISnake.Build.cs`、`DefaultInput.ini`
- 创建 `.github/copilot-instructions.md`，内容包括：
  - 构建命令（删除 Intermediate/Binaries/Saved → 重新生成项目文件 → Build.bat）
  - C++ 类职责表（11 个类）
  - 游戏流程（菜单 → 开始 → 游戏中 → 游戏结束）
  - 关键约定（Legacy 输入系统、网格移动、食物效果时长、SaveGame 槽名）
  - MCP 插件配置（McpAutomationBridge 端口 8091 / SpecialAgentPlugin 端口 8767）

---

## 对话二：根据 PRD 完整实现项目

**用户请求：** 根据 `/docs/SnakePrd.md` 完成整个项目。

### PRD 核心需求摘要

| 模块 | 要求 |
|------|------|
| 基础玩法 | 网格移动、吃食物增长（+10分）、撞自身/障碍物 Game Over、边界环绕 |
| 输入 | 传统输入系统 MoveUp/Down/Left/Right（WASD + 方向键）、长按 0.5s 加速 |
| 食物 | Normal 70% / Invisible 10% / Invincible 20%；0.5s 收集动画；吃后生成障碍物 |
| 隐身效果 | 持续 10s，可穿透自身和障碍物 |
| 无敌效果 | 持续 10s，撞障碍物摧毁之，撞自身仍 Game Over |
| 存档 | SaveGame 槽 `SnakeHighScores` UserIndex=0，Top 10 降序 |
| UI | 菜单UI（开始按钮 + 最高分列表）/ 游戏UI（当前分数）/ 流程切换 |

---

### 阶段一：C++ 源码实现

#### 修改文件

**`Source/AISnake/AISnake.Build.cs`**
```csharp
PublicDependencyModuleNames.AddRange(new string[] { "Core","CoreUObject","Engine","InputCore","UMG" });
PrivateDependencyModuleNames.AddRange(new string[] { "Slate","SlateCore" });
```

**`Config/DefaultInput.ini`**
- 将 `DefaultPlayerInputClass` 改为 `/Script/Engine.PlayerInput`
- 将 `DefaultInputComponentClass` 改为 `/Script/Engine.InputComponent`
- 添加 ActionMappings（MoveUp/Down/Left/Right → W/S/A/D + 上下左右箭头键）

#### 新建文件（共 22 个）

| 文件 | 类 | 说明 |
|------|----|------|
| `SnakeSaveGame.h/cpp` | `USnakeSaveGame : USaveGame` | Top 10 分数存储，`AddScore()` 自动排序截断 |
| `SnakeSegment.h/cpp` | `ASnakeSegment : AActor` | 蛇身节点，含 `UStaticMeshComponent* MeshComp` |
| `SnakeObstacle.h/cpp` | `ASnakeObstacle : AActor` | 障碍物，含 `UStaticMeshComponent* MeshComp` |
| `Food.h/cpp` | `AFood : AActor` + `EFoodType` | 食物类型枚举；`PlayCollectionAnimation()` 在 Tick 中执行弹跳+缩放动画（0.5s）；`GetRandomFoodType()` 静态方法 |
| `SnakeManager.h/cpp` | `ASnakeManager : AActor` | **核心管理器**：网格 20×20（100单位/格）、分数、游戏状态机、生成/销毁所有角色、格子占用追踪（TSet/TMap）、随机合法位置、正交摄像机、Debug 边界线、SaveGame |
| `Snake.h/cpp` | `ASnake : APawn` | 方向队列（防 180° 反转）、移动计时器（0.2s 正常/0.08s 加速）、边界环绕、吃食物/碰障碍物/碰自身检测、隐身/无敌 10s 计时器、`GrowBody()` 生成新节点 |
| `SnakePlayerController.h/cpp` | `ASnakePlayerController : APlayerController` | BindAction（Pressed+Released）、0.5s 长按计时器触发 `Snake::StartAcceleration()` |
| `SnakeGameUI.h/cpp` | `USnakeGameUI : UUserWidget` | `UPROPERTY(meta=(BindWidget)) UTextBlock* ScoreText`；`UpdateScore(int32)` |
| `SnakeMenuUI.h/cpp` | `USnakeMenuUI : UUserWidget` | `UButton* StartButton`、`UTextBlock* LastScoreText`、`UVerticalBox* ScoreListBox`；`UpdateScores()` 动态填充列表 |
| `SnakeHUD.h/cpp` | `ASnakeHUD : AHUD` | BeginPlay 创建两个 Widget 并加入 Viewport；`ShowMenu()` / `ShowGameUI()` 控制显隐；`OnStartGame()` 查找 SnakeManager 并调用 `StartGame()` |
| `SnakeGameMode.h/cpp` | `ASnakeGameMode : AGameModeBase` | 构造函数设置 `PlayerControllerClass = ASnakePlayerController`、`HUDClass = ASnakeHUD` |

#### 架构数据流

```
GameMode → HUD → 显示 MenuUI
MenuUI StartButton → HUD::OnStartGame() → SnakeManager::StartGame()
  → 生成 Snake (Possess) + Food
  → HUD::ShowGameUI()

PlayerController (WASD) → Snake::SetDesiredDirection()
                         → 长按 0.5s → Snake::StartAcceleration()

Snake MoveTimer 触发 MoveStep():
  → WrapPosition(头 + 方向)
  → IsFoodAt? → HandleFoodEaten()
              → score+10, 播放动画, 回调生成新Food+Obstacle
  → IsObstacleAt? → Invincible: DestroyObstacleAt()
                  → 否则: TriggerGameOver()
  → IsSnakeBodyAt? → Invisible: 忽略
                   → 否则: TriggerGameOver()
  → UpdateSnakeCells() 更新占用表

TriggerGameOver() → SaveHighScore() → HUD::ShowMenu(scores, lastScore)
```

#### 编译结果

```
第1次构建：1个错误
  SnakeHUD.cpp(50): error C2446 — TSharedRef 不接受 nullptr
  修复：拆分 SetWidgetToFocus 调用为 if/else

第2次构建：✅ 成功
  1 succeeded, 0 failed
  耗时 ~5.5s（增量）
```

---

### 阶段二：Blueprint 资产创建（通过 MCP SpecialAgent）

#### 创建的资产（路径 `/Game/Snake/`）

| 资产 | 类型 | 父类 |
|------|------|------|
| `BP_SnakeSegment` | Blueprint | `ASnakeSegment` |
| `BP_SnakeObstacle` | Blueprint | `ASnakeObstacle` |
| `BP_Food` | Blueprint | `AFood` |
| `BP_Snake` | Blueprint | `ASnake` |
| `BP_SnakeManager` | Blueprint | `ASnakeManager` |
| `BP_SnakeHUD` | Blueprint | `ASnakeHUD` |
| `WBP_SnakeGameUI` | WidgetBlueprint | `USnakeGameUI` |
| `WBP_SnakeMenuUI` | WidgetBlueprint | `USnakeMenuUI` |

#### Blueprint 配置

**网格体（通过 SubobjectDataSubsystem 访问 CDO）：**
- `BP_SnakeSegment.MeshComp` → `/Engine/BasicShapes/Cube`（缩放 0.9）
- `BP_SnakeObstacle.MeshComp` → `/Engine/BasicShapes/Cube`
- `BP_Food.MeshComp` → `/Engine/BasicShapes/Sphere`（缩放 0.8）
- `BP_Snake.HeadMesh` → `/Engine/BasicShapes/Cube`（缩放 1.05×1.05×1.2）

**类引用：**
- `BP_SnakeManager`：SnakeClass=`BP_Snake_C`，FoodClass=`BP_Food_C`，ObstacleClass=`BP_SnakeObstacle_C`，SegmentClass=`BP_SnakeSegment_C`
- `BP_Snake`：SegmentClass=`BP_SnakeSegment_C`
- `BP_SnakeHUD`：GameUIClass=`WBP_SnakeGameUI_C`，MenuUIClass=`WBP_SnakeMenuUI_C`

**WBP_SnakeGameUI Widget 树：**
```
CanvasPanel_Root
└── ScoreText (TextBlock, 36pt, 黄色, 顶部居中)
```

**WBP_SnakeMenuUI Widget 树：**
```
CanvasPanel_Root
└── MenuVBox (VerticalBox, 锚点 25%-75% 水平, 10%-90% 垂直)
    ├── LastScoreText (TextBlock, 28pt, 黄色, 居中)
    ├── StartButton (Button)
    │   └── StartButtonLabel (TextBlock, 32pt, "START GAME")
    ├── HighScoreHeader (TextBlock, 24pt, "── HIGH SCORES ──")
    └── ScoreListBox (VerticalBox, 动态填充最高分列表)
```

#### 地图 M_Start（路径 `/Game/Maps/M_Start/M_Start`）

- GameMode → `ASnakeGameMode`（World Settings 设置）
- 放置的 Actor：
  - `BP_SnakeManager_C_0`（位置 0,0,0）
  - `DirectionalLight`（位置 0,0,1000，旋转 -45°）
  - `SkyLight`（位置 0,0,500）
  - `GridFloor`（StaticMeshActor，Plane，缩放 20×20，位置 1000,1000,-5）

---

### 验收清单自检

| 验收项 | 状态 |
|--------|------|
| 可编译 | ✅（0 errors, 1 warning in McpAutomationBridge 第三方插件） |
| 能开始游戏 | ✅（MenuUI→StartButton→SnakeManager::StartGame） |
| 蛇能移动/加速 | ✅（MoveTimer 0.2s / 0.08s，长按 0.5s 触发） |
| 蛇能吃食物增长 | ✅（GrowBody + HandleFoodEaten） |
| 隐身/无敌效果 | ✅（ApplyFoodEffect，各 10s 计时器） |
| 生成障碍物 | ✅（HandleFoodEaten 回调后 SpawnObstacle） |
| 边界环绕 | ✅（WrapPosition 取模运算） |
| 碰撞逻辑 | ✅（三路检测：食物/障碍物/自身） |
| 游戏结束流程 | ✅（TriggerGameOver → 保存 → ShowMenu） |
| Top 10 存读档 | ✅（USnakeSaveGame，AddScore 自动排序） |

---

## 关键技术点备注

### 坐标系约定
- 网格 X = 列（MoveRight +X，MoveLeft -X）
- 网格 Y = 行（MoveUp +Y，MoveDown -Y）
- 世界坐标 = `FVector(GridX * 100, GridY * 100, 0)`
- 摄像机位于 `(1000, 1000, 2500)`，朝向 -Z，正交投影宽度 = 网格宽 × 100 × 1.1

### 输入系统
- Legacy Input（非 Enhanced Input）
- `DefaultInput.ini` 中配置 ActionMappings
- `SnakePlayerController::SetupInputComponent()` 使用 `BindAction`（Pressed + Released）

### 关键 API 注意
- **SaveGame**：使用 `UGameplayStatics::SaveGameToSlot / LoadGameFromSlot`，槽名 `"SnakeHighScores"`
- **Widget BindWidget**：C++ 中 `UPROPERTY(meta=(BindWidget))` 的名称必须与 Widget Blueprint 中的控件名完全一致
- **McpAutomationBridge**：禁止使用 `UPackage::SavePackage()`，改用 `McpSafeAssetSave()`；`ANY_PACKAGE` 在 UE5.7 中已废弃，改用 `nullptr`

---

*本文件由 GitHub Copilot CLI 自动生成，记录 AISnake 项目的完整开发过程。*
