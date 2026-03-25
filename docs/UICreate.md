# UICreate — UMG Widget Blueprint 创建指南

> 基于 McpAutomationBridge (端口 8091) + SpecialAgentPlugin (端口 8767) 的实战经验总结。

---

## 一、标准创建流程

### Step 1：创建 Widget Blueprint

```
工具: manage_widget_authoring → create_widget_blueprint
必填参数:
  name        = "WBP_XxxYyy"               # Widget 名称
  folder      = "/Game/UI"                  # 内容浏览器路径
  parentClass = "/Script/AISnake.XxxYyy"   # C++ 父类完整路径
```

> ⚠️ `parentClass` 必须用完整脚本路径格式 `/Script/<模块名>.<类名>`，不能只写类名。

---

### Step 2：添加根节点（CanvasPanel）

```
工具: manage_widget_authoring → add_canvas_panel
必填参数:
  widgetPath = "/Game/UI/WBP_XxxYyy"
  slotName   = "CanvasPanel_Root"    # ← 这是 widget 自身的名字，不是 name 字段！
```

> ⚠️ **关键易错点**：所有 `add_*` 动作中，widget 的名字由 `slotName` 决定，`name` 字段会被忽略。

---

### Step 3：添加子 Widget

所有子 widget 的创建规则一致：

| 参数 | 说明 |
|---|---|
| `widgetPath` | WBP 资产路径 |
| `slotName` | **此 widget 自身的名字** |
| `parentSlot` | **父 widget 的名字**（即父的 slotName） |

**示例 — VerticalBox：**
```
manage_widget_authoring → add_vertical_box
  widgetPath = "/Game/UI/WBP_XxxYyy"
  slotName   = "MenuVBox"
  parentSlot = "CanvasPanel_Root"
```

**示例 — TextBlock：**
```
manage_widget_authoring → add_text_block
  widgetPath       = "/Game/UI/WBP_XxxYyy"
  slotName         = "LastScoreText"
  parentSlot       = "MenuVBox"
  text             = "Welcome!"
  fontSize         = 28
  justification    = "Center"
  colorAndOpacity  = {"r":1, "g":1, "b":0, "a":1}   # 黄色
```

**示例 — Button + 内部 Label：**
```
# 先创建按钮
manage_widget_authoring → add_button
  slotName   = "StartButton"
  parentSlot = "MenuVBox"

# 再把 TextBlock 作为按钮子节点
manage_widget_authoring → add_text_block
  slotName   = "StartButtonLabel"
  parentSlot = "StartButton"      # ← 父是按钮名
  text       = "START GAME"
  fontSize   = 32
  justification = "Center"
```

---

### Step 4：设置 CanvasPanel 子节点的锚点/对齐（Python 绕过 Bug）

> ⚠️ **已知 Bug**：`manage_widget_authoring → set_anchor` / `set_alignment` / `set_position`  
> 工具 schema 发送 `slotName`，但 C++ handler 读取 `widgetName`，永远匹配失败。  
> **必须用 SpecialAgent Python 直接操作 `CanvasPanelSlot` 对象。**

```python
# SpecialAgent python-execute
import unreal

asset_path = "/Game/UI/WBP_XxxYyy"
widget_bp = unreal.load_asset(asset_path)

# 找到目标 widget（遍历 ObjectIterator）
target = None
for it in unreal.ObjectIterator(unreal.Object):
    try:
        if it.get_path_name().endswith(':WidgetTree.MenuVBox'):
            target = it
            break
    except:
        pass

slot = target.get_editor_property('Slot')  # 获取 CanvasPanelSlot

# 设置锚点
anchors = unreal.Anchors()
anchors.minimum = unreal.Vector2D(0.5, 0.5)
anchors.maximum = unreal.Vector2D(0.5, 0.5)
slot.set_anchors(anchors)

# 设置对齐（pivot）
slot.set_alignment(unreal.Vector2D(0.5, 0.5))

# 开启 AutoSize（内容自适应宽高）
slot.set_auto_size(True)

# 设置位置偏移（相对于锚点）
slot.set_position(unreal.Vector2D(0.0, -50.0))   # 向上偏移 50px

widget_bp.modify()
unreal.EditorAssetLibrary.save_asset(asset_path)
```

**常用锚点预设：**

| 位置 | minimum | maximum |
|---|---|---|
| 左上角 | (0, 0) | (0, 0) |
| 正中心 | (0.5, 0.5) | (0.5, 0.5) |
| 全屏填充 | (0, 0) | (1, 1) |
| 顶部居中 | (0.5, 0) | (0.5, 0) |
| 底部居中 | (0.5, 1) | (0.5, 1) |

---

### Step 5：验证 Widget 树结构

```
工具: manage_widget_authoring → get_widget_info
  widgetPath = "/Game/UI/WBP_XxxYyy"

返回字段:
  parentClass  → 确认父类正确
  slots[]      → 列出所有 widget 名称，验证结构完整
```

---

### Step 6：编译 + 保存

```python
# SpecialAgent python-execute
import unreal
from unreal import BlueprintEditorLibrary

asset_path = "/Game/UI/WBP_XxxYyy"
widget_bp = unreal.load_asset(asset_path)

BlueprintEditorLibrary.compile_blueprint(widget_bp)
unreal.EditorAssetLibrary.save_asset(asset_path)
```

---

## 二、参数速查表

### add_text_block 全量参数

| 参数 | 类型 | 说明 |
|---|---|---|
| `slotName` | string | widget 名 |
| `parentSlot` | string | 父 widget 名 |
| `text` | string | 显示文字 |
| `fontSize` | number | 字号 |
| `justification` | "Left"/"Center"/"Right" | 文字对齐 |
| `colorAndOpacity` | `{r,g,b,a}` | 文字颜色 |
| `horizontalAlignment` | "Left"/"Center"/"Right"/"Fill" | 水平对齐 |
| `verticalAlignment` | "Top"/"Center"/"Bottom"/"Fill" | 垂直对齐 |
| `autoWrap` | bool | 自动换行 |

### add_button 全量参数

| 参数 | 类型 | 说明 |
|---|---|---|
| `slotName` | string | widget 名 |
| `parentSlot` | string | 父 widget 名 |
| `horizontalAlignment` | string | 内容水平对齐 |
| `verticalAlignment` | string | 内容垂直对齐 |

### add_vertical_box / add_horizontal_box

| 参数 | 类型 | 说明 |
|---|---|---|
| `slotName` | string | widget 名 |
| `parentSlot` | string | 父 widget 名 |
| `horizontalAlignment` | string | 子项水平对齐 |
| `verticalAlignment` | string | 子项垂直对齐 |

---

## 三、BindWidget 与 C++ 父类的关系

C++ 父类中用 `UPROPERTY(meta=(BindWidget))` 声明的属性，在 Blueprint 子类中必须存在**同名** widget，否则编译报错。

```cpp
// C++ 父类声明
UPROPERTY(meta=(BindWidget))
UButton* StartButton;           // Blueprint 中必须有名为 StartButton 的 Button

UPROPERTY(meta=(BindWidget))
UTextBlock* LastScoreText;      // Blueprint 中必须有名为 LastScoreText 的 TextBlock

UPROPERTY(meta=(BindWidget))
UVerticalBox* ScoreListBox;     // Blueprint 中必须有名为 ScoreListBox 的 VerticalBox
```

> 创建 widget 时 `slotName` 必须与 C++ 中的变量名**完全一致**（大小写敏感）。

---

## 四、已知 Bug 与绕过方案

| 工具动作 | Bug 描述 | 绕过方案 |
|---|---|---|
| `set_anchor` | schema 发 `slotName`，handler 读 `widgetName`，永远找不到 widget | SpecialAgent Python: `slot.set_anchors(...)` |
| `set_alignment` | 同上 | SpecialAgent Python: `slot.set_alignment(...)` |
| `set_position` | 同上 | SpecialAgent Python: `slot.set_position(...)` |
| `set_size` | 同上 | SpecialAgent Python: `slot.set_size(...)` |

**Python 查找任意 widget 的通用代码：**

```python
import unreal

def find_widget(blueprint_path: str, widget_name: str):
    suffix = f':WidgetTree.{widget_name}'
    for it in unreal.ObjectIterator(unreal.Object):
        try:
            if blueprint_path in it.get_path_name() and it.get_path_name().endswith(suffix):
                return it
        except:
            pass
    return None
```

---

## 五、完整示例：重建 WBP_SnakeMenuUI

```
1. create_widget_blueprint  name=WBP_SnakeMenuUI  parentClass=/Script/AISnake.SnakeMenuUI
2. add_canvas_panel         slotName=CanvasPanel_Root
3. add_vertical_box         slotName=MenuVBox          parentSlot=CanvasPanel_Root
4. [Python] 设置 MenuVBox 锚点 (0.5/0.5), 对齐 (0.5/0.5), AutoSize=true, 位置 y=-50
5. add_text_block           slotName=LastScoreText     parentSlot=MenuVBox  text="Welcome!"  fontSize=28  color=yellow
6. add_button               slotName=StartButton       parentSlot=MenuVBox
7. add_text_block           slotName=StartButtonLabel  parentSlot=StartButton  text="START GAME"  fontSize=32
8. add_text_block           slotName=HighScoreHeader   parentSlot=MenuVBox  text="── HIGH SCORES ──"  justification=Center
9. add_vertical_box         slotName=ScoreListBox      parentSlot=MenuVBox
10. get_widget_info         → 验证 slots 列表
11. [Python] BlueprintEditorLibrary.compile_blueprint() + save_asset()
```

---

## 六、注意事项

- **删除资产**：重建前用 `EditorAssetLibrary.delete_asset(path)` 清除旧资产，避免残留脏数据。
- **保存时机**：每次大批量修改后调用 `save_asset`；`compile_blueprint` 必须在 save 之前或同时调用。
- **父类查找失败**：`create_widget_blueprint` 找不到父类时会静默降级为 `UserWidget`，需用 `get_widget_info` 的 `parentClass` 字段验证。
- **widget 命名**：所有名称大小写敏感，与 C++ `BindWidget` 变量名保持一致。
