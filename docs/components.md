# 组件目录

[返回首页](../README.md) · [维护与路线图](roadmap.md) · [参与贡献](contributing.md)

本目录按当前仓库的组件头文件整理。点击组件名可查看接口声明，组合用法可参考[示例页面](../ElaWidgetToolsExample/ExamplePage/)。宏、类型定义和内部实现不列入组件目录。

组件存在于源码中不等于已通过全部平台或 Qt 版本验证，具体维护状态见[维护与路线图](roadmap.md)。

## 应用、主题与窗口

| 组件 | 用途 |
| --- | --- |
| [ElaApplication](../ElaWidgetTools/ElaApplication.h) | 应用初始化与全局设置 |
| [ElaTheme](../ElaWidgetTools/ElaTheme.h) | 浅色、深色主题管理 |
| [ElaIcon](../ElaWidgetTools/ElaIcon.h) | 图标支持 |
| [ElaWindow](../ElaWidgetTools/ElaWindow.h) | 带导航栏的无边框主窗口 |
| [ElaWidget](../ElaWidgetTools/ElaWidget.h) | 无边框窗口 |
| [ElaDialog](../ElaWidgetTools/ElaDialog.h) | 对话框窗口 |
| [ElaAppBar](../ElaWidgetTools/ElaAppBar.h) | 自定义窗口标题栏 |
| [ElaDockWidget](../ElaWidgetTools/ElaDockWidget.h) | 停靠面板 |

## 按钮与输入

| 组件 | 用途 |
| --- | --- |
| [ElaPushButton](../ElaWidgetTools/ElaPushButton.h) | 按钮 |
| [ElaIconButton](../ElaWidgetTools/ElaIconButton.h) | 图标按钮 |
| [ElaToolButton](../ElaWidgetTools/ElaToolButton.h) | 工具按钮 |
| [ElaToggleButton](../ElaWidgetTools/ElaToggleButton.h) | 状态切换按钮 |
| [ElaToggleSwitch](../ElaWidgetTools/ElaToggleSwitch.h) | 开关 |
| [ElaCheckBox](../ElaWidgetTools/ElaCheckBox.h) | 复选框 |
| [ElaRadioButton](../ElaWidgetTools/ElaRadioButton.h) | 单选按钮 |
| [ElaComboBox](../ElaWidgetTools/ElaComboBox.h) | 下拉选择框 |
| [ElaMultiSelectComboBox](../ElaWidgetTools/ElaMultiSelectComboBox.h) | 多选下拉框 |
| [ElaLineEdit](../ElaWidgetTools/ElaLineEdit.h) | 单行输入框 |
| [ElaPlainTextEdit](../ElaWidgetTools/ElaPlainTextEdit.h) | 多行纯文本编辑框 |
| [ElaSpinBox](../ElaWidgetTools/ElaSpinBox.h) | 整数微调框 |
| [ElaDoubleSpinBox](../ElaWidgetTools/ElaDoubleSpinBox.h) | 浮点数微调框 |
| [ElaSlider](../ElaWidgetTools/ElaSlider.h) | 滑动条 |
| [ElaSuggestBox](../ElaWidgetTools/ElaSuggestBox.h) | 建议搜索框 |
| [ElaKeyBinder](../ElaWidgetTools/ElaKeyBinder.h) | 按键绑定器 |

## 导航与布局

| 组件 | 用途 |
| --- | --- |
| [ElaNavigationBar](../ElaWidgetTools/ElaNavigationBar.h) | 侧边导航栏 |
| [ElaBreadcrumbBar](../ElaWidgetTools/ElaBreadcrumbBar.h) | 面包屑导航 |
| [ElaPivot](../ElaWidgetTools/ElaPivot.h) | 页面切换导航 |
| [ElaTabBar](../ElaWidgetTools/ElaTabBar.h) | 选项卡栏 |
| [ElaTabWidget](../ElaWidgetTools/ElaTabWidget.h) | 选项卡页面容器 |
| [ElaScrollPage](../ElaWidgetTools/ElaScrollPage.h) | 带页面管理的滚动页面 |
| [ElaScrollPageArea](../ElaWidgetTools/ElaScrollPageArea.h) | 滚动页面区域 |
| [ElaScrollArea](../ElaWidgetTools/ElaScrollArea.h) | 滚动区域 |
| [ElaScrollBar](../ElaWidgetTools/ElaScrollBar.h) | 滚动条 |
| [ElaFlowLayout](../ElaWidgetTools/ElaFlowLayout.h) | 流式布局 |
| [ElaDrawerArea](../ElaWidgetTools/ElaDrawerArea.h) | 抽屉区域 |

## 数据展示与视图

| 组件 | 用途 |
| --- | --- |
| [ElaText](../ElaWidgetTools/ElaText.h) | 文本展示 |
| [ElaLCDNumber](../ElaWidgetTools/ElaLCDNumber.h) | 液晶风格数字显示 |
| [ElaProgressBar](../ElaWidgetTools/ElaProgressBar.h) | 进度条 |
| [ElaProgressRing](../ElaWidgetTools/ElaProgressRing.h) | 环形进度展示 |
| [ElaListView](../ElaWidgetTools/ElaListView.h) | 列表视图 |
| [ElaTableView](../ElaWidgetTools/ElaTableView.h) | 表格视图 |
| [ElaTreeView](../ElaWidgetTools/ElaTreeView.h) | 树形视图 |

## 卡片与图片

| 组件 | 用途 |
| --- | --- |
| [ElaAcrylicUrlCard](../ElaWidgetTools/ElaAcrylicUrlCard.h) | 支持链接跳转的亚克力卡片 |
| [ElaImageCard](../ElaWidgetTools/ElaImageCard.h) | 图片卡片 |
| [ElaInteractiveCard](../ElaWidgetTools/ElaInteractiveCard.h) | 交互式卡片 |
| [ElaPopularCard](../ElaWidgetTools/ElaPopularCard.h) | 热门内容卡片 |
| [ElaPromotionCard](../ElaWidgetTools/ElaPromotionCard.h) | 推广卡片 |
| [ElaPromotionView](../ElaWidgetTools/ElaPromotionView.h) | 推广卡片视图 |
| [ElaReminderCard](../ElaWidgetTools/ElaReminderCard.h) | 提醒卡片 |

## 菜单、提示与选择器

| 组件 | 用途 |
| --- | --- |
| [ElaMenu](../ElaWidgetTools/ElaMenu.h) | 菜单 |
| [ElaMenuBar](../ElaWidgetTools/ElaMenuBar.h) | 菜单栏 |
| [ElaToolBar](../ElaWidgetTools/ElaToolBar.h) | 工具栏 |
| [ElaRibbonBar](../ElaWidgetTools/ElaRibbonBar.h) | Ribbon 工具区 |
| [ElaStatusBar](../ElaWidgetTools/ElaStatusBar.h) | 状态栏 |
| [ElaContentDialog](../ElaWidgetTools/ElaContentDialog.h) | 带遮罩的内容对话框 |
| [ElaMessageBar](../ElaWidgetTools/ElaMessageBar.h) | 消息提示栏 |
| [ElaMessageButton](../ElaWidgetTools/ElaMessageButton.h) | 消息提示按钮 |
| [ElaToolTip](../ElaWidgetTools/ElaToolTip.h) | 工具提示 |
| [ElaColorDialog](../ElaWidgetTools/ElaColorDialog.h) | 颜色选择对话框 |
| [ElaCalendar](../ElaWidgetTools/ElaCalendar.h) | 日历视图 |
| [ElaCalendarPicker](../ElaWidgetTools/ElaCalendarPicker.h) | 日期选择器 |
| [ElaRoller](../ElaWidgetTools/ElaRoller.h) | 滚轮选择器 |
| [ElaRollerPicker](../ElaWidgetTools/ElaRollerPicker.h) | 多列滚轮选择器 |

## 辅助能力

| 组件 | 用途 |
| --- | --- |
| [ElaEventBus](../ElaWidgetTools/ElaEventBus.h) | 事件总线 |
| [ElaLog](../ElaWidgetTools/ElaLog.h) | 日志支持 |
| [ElaActionCommander](../ElaWidgetTools/ElaActionCommander.h) | 命令式操作器 |
| [ElaExponentialBlur](../ElaWidgetTools/ElaExponentialBlur.h) | 图像指数模糊 |
| [ElaDxgiManager](../ElaWidgetTools/ElaDxgiManager.h) | 基于 DXGI 的屏幕采集，仅限 Windows |

## 从示例入手

示例工程位于 [ElaWidgetToolsExample](../ElaWidgetToolsExample/)，可从以下页面了解常见组合方式：

| 示例 | 内容 |
| --- | --- |
| [基础控件](../ElaWidgetToolsExample/ExamplePage/T_BaseComponents.cpp) | 按钮、输入与选择控件 |
| [导航](../ElaWidgetToolsExample/ExamplePage/T_Navigation.cpp) | 页面导航与切换 |
| [卡片](../ElaWidgetToolsExample/ExamplePage/T_Card.cpp) | 卡片展示与交互 |
| [弹出交互](../ElaWidgetToolsExample/ExamplePage/T_Popup.cpp) | 菜单、对话框与消息提示 |
| [表格视图](../ElaWidgetToolsExample/ExamplePage/T_TableView.cpp) | 表格与数据展示 |

示例编译方式将在 Qt 构建迁移完成并验证后补充。
