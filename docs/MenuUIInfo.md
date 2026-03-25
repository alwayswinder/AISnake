  遵循UICreate.md,创建WBP_SnakeMenuUI,继承自SnakeMenuUI
  如果在执行中遇到新的可值得下次参考的信息，例如新的子widget，新的参数设置，请补充到UICreate.md中

● 以下是 WBP_SnakeMenuUI 的完整布局结构：

   [CanvasPanel] CanvasPanel_Root  ← RootWidget
   └── [VerticalBox] MenuVBox
       │  (Anchor: 中心 0.5/0.5, Alignment: 0.5/0.5, OffsetTop: -50, AutoSize)
       │
       ├── [0] [TextBlock] LastScoreText
       │         Text: "Welcome!"
       │         Color: 黄色 (R=1, G=1, B=0)
       │         FontSize: 28, Justification: Center
       │
       ├── [1] [Button] StartButton
       │    └── [TextBlock] StartButtonLabel
       │              Text: "START GAME"
       │              FontSize: 32, Justification: Center
       │
       ├── [2] [TextBlock] HighScoreHeader
       │         Text: "── HIGH SCORES ──"
       │         Justification: Center
       │         HAlign: Center, VAlign: Center
       │
       └── [3] [VerticalBox] ScoreListBox
                 HAlign: Center, VAlign: Center
                 (空，由 C++ 动态填充排行榜条目)

  关键说明：
   - CanvasPanel_Root 为根节点，MenuVBox 居中锚定（向上偏移 50px）
   - LastScoreText 显示上局得分（默认 "Welcome!"），ScoreListBox 动态生成 Top-10 排行榜行
   - 布局顺序：得分显示 → 开始按钮 → 排行榜标题 → 排行榜列表