# Copilot Instructions — AISnake

## Project Overview

3D Snake game for **Unreal Engine 5.7**, implemented in C++ with UMG UI. Core game logic lives in C++; Blueprints are used only for asset binding and scene configuration. The `Source/AISnake/` module currently contains only the empty module stub — the game classes are being built from the PRD in `docs/SnakePrd.md`.

## Build

This is a standard UE5 C++ project built with Unreal Build Tool (UBT).

**Rebuild from scratch** (after source changes or plugin issues):
1. Close Visual Studio and Unreal Editor
2. Delete `Intermediate/`, `Binaries/`, `Saved/`
3. Right-click `AISnake.uproject` → **Generate Visual Studio project files**
4. Open `AISnake.sln` and build the `Development Editor` configuration

**From command line (UBT):**
```bat
Engine\Build\BatchFiles\Build.bat AISnakeEditor Win64 Development "D:\Unreal\FromGithub\AIProject\AISnake\AISnake.uproject"
```

There are no automated tests or lint scripts in this project.

## Architecture

### C++ Class Responsibilities

| Class | Base | Role |
|---|---|---|
| `SnakeGameMode` | `AGameModeBase` | Sets default PlayerController and HUD |
| `SnakeManager` | `AActor` | Master manager: score, game state, spawning/destroying Snake/Food/Obstacle, valid random positions, camera, boundary visualization, SaveGame |
| `Snake` | `APawn` | Movement tick, direction queue, anti-reverse, body segments, boundary wrap, collision, acceleration, invisibility/invincibility effects |
| `SnakePlayerController` | `APlayerController` | Legacy input → passes direction to Snake |
| `SnakeHUD` | `AHUD` | Creates and manages `SnakeMenuUI` and `SnakeGameUI` widgets |
| `SnakeMenuUI` | `UUserWidget` | Start button, current score, top-10 leaderboard |
| `SnakeGameUI` | `UUserWidget` | In-game score display |
| `Food` | `AActor` | Spawnable food with `EFoodType` (Normal 70%, Invisible 10%, Invincible 20%) |
| `SnakeSegment` | `AActor` | Single body segment, follows the segment ahead |
| `SnakeObstacle` | `AActor` | Static obstacle, spawned one per food eaten |
| `SnakeSaveGame` | `USaveGame` | Top-10 high scores, slot `SnakeHighScores`, UserIndex 0 |

### Game Flow
```
Editor launch → SnakeGameMode spawns HUD → HUD shows SnakeMenuUI
Player clicks Start → HUD hides menu, shows SnakeGameUI → SnakeManager spawns Snake + Food
Tick → Snake moves one grid cell per interval → SnakePlayerController feeds input
Snake eats Food → score +10, body +1, 0.5s collect animation, Food respawns, +1 Obstacle spawned
Collision with self or obstacle → GameOver → HUD hides SnakeGameUI, shows SnakeMenuUI with final score
```

### Blueprint Counterparts
Each C++ class has a Blueprint child for asset assignment:
`BP_SnakeManager`, `BP_Snake`, `BP_Food`, `BP_SnakeSegment`, `BP_SnakeObstacle`, `BP_SnakeHUD`, `WBP_SnakeMenuUI`, `WBP_SnakeGameUI`

The map is `M_Start`; place `BP_SnakeManager` in the scene and set GameMode to `SnakeGameMode` in World Settings.

## Key Conventions

### Input System
The project uses the **Legacy Input System** (not Enhanced Input for gameplay actions). `DefaultInput.ini` defines `MoveUp`/`MoveDown`/`MoveLeft`/`MoveRight` mapped to WASD + arrow keys. However, `DefaultPlayerInputClass` and `DefaultInputComponentClass` are set to `EnhancedInput` classes in `DefaultInput.ini` — if adding new input actions, keep this distinction in mind.

### Build Module
`AISnake.Build.cs` dependencies: `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`. The PRD requires adding `UMG`, `Slate`, `SlateCore` when implementing widgets.

### Grid / Boundary Wrapping
Snake movement is grid-based. Boundary collision wraps the snake head to the opposite side (no death at border). Food and Obstacle spawn positions must be validated against all occupied cells (snake body, other food, existing obstacles).

### Food Effects
- **Invisible** (10s): snake can pass through its own body and obstacles
- **Invincible** (20s): colliding with an obstacle destroys it; colliding with own body still ends game
- Acceleration: hold any direction key for 0.5 s to trigger; release to restore normal speed

### SaveGame
Use `UGameplayStatics::SaveGameToSlot` / `LoadGameFromSlot` with slot `SnakeHighScores`, `UserIndex = 0`. Maintain a sorted descending Top-10 list.

## MCP / AI Plugins

This project ships two AI-control plugins used during development:

### McpAutomationBridge
- WebSocket server on **port 8091** (configurable in Project Settings → Plugins → MCP Automation Bridge)
- Configure AI client (`claude_desktop_config.json`):
  ```json
  { "mcpServers": { "unreal-engine": { "command": "npx", "args": ["unreal-engine-mcp-server"], "env": { "UE_PROJECT_PATH": "D:/Unreal/FromGithub/AIProject/AISnake" } } } }
  ```
- **Never** call `UPackage::SavePackage()` directly — use `McpSafeAssetSave()` from `McpAutomationBridgeHelpers.h`
- `ANY_PACKAGE` is deprecated in UE 5.7 — use `nullptr` for path-based object lookups
- Handlers run on the game thread; never block it in WebSocket frame processing

### SpecialAgentPlugin
- HTTP/SSE MCP server on **port 8767**
- Configure AI client:
  ```json
  { "mcpServers": { "SpecialAgent": { "url": "http://localhost:8767/sse", "transport": "sse" } } }
  ```
- Provides Python execution with full `unreal` module access + 71+ level design tools
- Requires UE 5.6+

Both plugins are editor-only and require the editor to be running before connecting an AI client.
