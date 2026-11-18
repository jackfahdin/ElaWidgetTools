#include "T_Setting.h"

#include "ElaApplication.h"
#include "ElaComboBox.h"
#include "ElaColorDialog.h"
#include "ElaPushButton.h"
#include "ElaScrollPageArea.h"
#include "ElaText.h"
#include "ElaCheckBox.h"
#include "ElaSlider.h"
#include <QHBoxLayout>
#include "ElaLog.h"
#include "ElaTheme.h"
#include "ElaToggleSwitch.h"
#include "ElaWindow.h"

#include <QDebug>
#include <QSignalBlocker>
#include <QVBoxLayout>

T_Setting::T_Setting(QWidget* parent) : T_BasePage(parent)
{
    auto* window = qobject_cast<ElaWindow*>(parent);
    setWindowTitle("设置");
    createCustomWidget("调整展示程序的外观、导航与交互效果，设置会即时应用。", "设置");
    auto* mainLayout = createPageLayout();
    auto* appearanceSection =
        addExampleSection(mainLayout, "外观", "背景绘制与窗口材质可分别调整，系统材质效果取决于平台支持。");
    _themeComboBox = new ElaComboBox(this);
    _themeComboBox->setFixedWidth(200);
    _themeComboBox->addItems({"浅色", "深色", "跟随系统"});
    connect(_themeComboBox, QOverload<int>::of(&ElaComboBox::currentIndexChanged), this, [](int index) {
        if (index == 2)
            eTheme->setFollowSystemTheme(true);
        else
            eTheme->setThemeMode(index == 0 ? ElaThemeType::Light : ElaThemeType::Dark);
    });
    auto syncThemeSelection = [this]() {
        const QSignalBlocker blocker(_themeComboBox);
        _themeComboBox->setCurrentIndex(eTheme->getFollowSystemTheme() ? 2 : int(eTheme->getThemeMode()));
    };
    syncThemeSelection();
    connect(eTheme, &ElaTheme::themeModeChanged, this, syncThemeSelection);
    connect(eTheme, &ElaTheme::followSystemThemeChanged, this, syncThemeSelection);
    appearanceSection->addWidget(createSettingRow("明暗主题", "为整个展示程序切换主题", _themeComboBox));

    ElaScrollPageArea* accentArea = new ElaScrollPageArea(this);
    QHBoxLayout* accentLayout = new QHBoxLayout(accentArea);
    ElaText* accentText = new ElaText("强调色", 15, this);
    ElaPushButton* accentButton = new ElaPushButton(this);
    ElaPushButton* resetColorsButton = new ElaPushButton("恢复默认配色", this);
    accentLayout->addWidget(accentText);
    accentLayout->addStretch();
    accentLayout->addWidget(accentButton);
    accentLayout->addWidget(resetColorsButton);
    auto syncAccent = [=]() {
        accentButton->setText("选择颜色 · " + eTheme->getAccentColor(eTheme->getThemeMode()).name());
    };
    syncAccent();
    connect(eTheme, &ElaTheme::themeModeChanged, this, syncAccent);
    connect(eTheme, &ElaTheme::themeColorsChanged, this, syncAccent);
    connect(accentButton, &ElaPushButton::clicked, this, [=]() {
        auto* dialog = new ElaColorDialog(this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setCurrentColor(eTheme->getAccentColor(eTheme->getThemeMode()));
        connect(dialog, &ElaColorDialog::colorSelected, this, [](const QColor& color) {
            eTheme->setAccentColor(color);
        });
        dialog->open();
    });
    connect(resetColorsButton, &ElaPushButton::clicked, eTheme, &ElaTheme::resetThemeColors);

    ElaScrollPageArea* themePreviewArea = new ElaScrollPageArea(this);
    QHBoxLayout* previewLayout = new QHBoxLayout(themePreviewArea);
    ElaText* previewText = new ElaText("实时预览", 15, this);
    ElaCheckBox* previewCheckBox = new ElaCheckBox("选中状态", this);
    previewCheckBox->setChecked(true);
    ElaSlider* previewSlider = new ElaSlider(Qt::Horizontal, this);
    previewSlider->setFixedWidth(130);
    previewSlider->setValue(65);
    previewSlider->setAccessibleName("强调色预览滑块");
    ElaToggleSwitch* previewSwitch = new ElaToggleSwitch(this);
    previewSwitch->setIsToggled(true);
    previewSwitch->setAccessibleName("主题预览开关");
    previewSwitch->setToolTip("按 Tab 聚焦，按空格切换");
    previewLayout->addWidget(previewText);
    previewLayout->addStretch();
    previewLayout->addWidget(previewCheckBox);
    previewLayout->addSpacing(16);
    previewLayout->addWidget(previewSlider);
    previewLayout->addSpacing(16);
    previewLayout->addWidget(previewSwitch);
    ElaText* themeHint = new ElaText("可用 Tab 和空格操作开关；跟随系统时，标题栏的主题按钮可切回手动模式。", 13, this);

    appearanceSection->addWidget(accentArea);
    appearanceSection->addWidget(themePreviewArea);
    appearanceSection->addWidget(themeHint);

    auto* paintComboBox = new ElaComboBox(this);
    paintComboBox->setFixedWidth(200);
    paintComboBox->addItems({"纯色背景", "静态图片", "动态图片"});
    paintComboBox->setEnabled(window);
    if (window)
    {
        paintComboBox->setCurrentIndex(int(window->getWindowPaintMode()));
        connect(paintComboBox, QOverload<int>::of(&ElaComboBox::currentIndexChanged), this, [window](int index) {
            window->setWindowPaintMode(static_cast<ElaWindowType::PaintMode>(index));
        });
        connect(window, &ElaWindow::pWindowPaintModeChanged, this, [window, paintComboBox]() {
            const QSignalBlocker blocker(paintComboBox);
            paintComboBox->setCurrentIndex(int(window->getWindowPaintMode()));
        });
    }
    appearanceSection->addWidget(createSettingRow("窗口背景", "在普通背景、图片和动图之间切换", paintComboBox));

    auto* materialComboBox = new ElaComboBox(this);
    materialComboBox->setFixedWidth(200);
    materialComboBox->addItems({"默认材质", "Ela 云母"});
#ifdef Q_OS_WIN
    materialComboBox->addItems({"系统云母", "替代云母", "亚克力", "窗口模糊"});
#endif
    materialComboBox->setCurrentIndex(int(eApp->getWindowDisplayMode()));
    connect(materialComboBox, QOverload<int>::of(&ElaComboBox::currentIndexChanged), this, [](int index) {
        eApp->setWindowDisplayMode(static_cast<ElaApplicationType::WindowDisplayMode>(index));
    });
    connect(eApp, &ElaApplication::pWindowDisplayModeChanged, this, [materialComboBox]() {
        const QSignalBlocker blocker(materialComboBox);
        materialComboBox->setCurrentIndex(int(eApp->getWindowDisplayMode()));
    });
    appearanceSection->addWidget(createSettingRow("窗口材质", "选择应用背景的材质效果", materialComboBox));

    auto* navigationSection = addExampleSection(mainLayout, "导航与页面", "控制侧边栏的展开方式和页面切换动画。");
    auto* navigationComboBox = new ElaComboBox(this);
    navigationComboBox->setFixedWidth(200);
    navigationComboBox->addItems({"自动适应", "最小化", "紧凑", "完整展开"});
    navigationComboBox->setEnabled(window);
    auto* transitionComboBox = new ElaComboBox(this);
    transitionComboBox->setFixedWidth(200);
    transitionComboBox->addItems({"无动画", "滑入", "缩放", "翻转", "模糊"});
    transitionComboBox->setEnabled(window);
    if (window)
    {
        navigationComboBox->setCurrentIndex(int(window->getNavigationBarDisplayMode()));
        transitionComboBox->setCurrentIndex(int(window->getStackSwitchMode()));
        connect(navigationComboBox, QOverload<int>::of(&ElaComboBox::currentIndexChanged), this, [window](int index) {
            window->setNavigationBarDisplayMode(static_cast<ElaNavigationType::NavigationDisplayMode>(index));
        });
        connect(transitionComboBox, QOverload<int>::of(&ElaComboBox::currentIndexChanged), this, [window](int index) {
            window->setStackSwitchMode(static_cast<ElaWindowType::StackSwitchMode>(index));
        });
        connect(window, &ElaWindow::pNavigationBarDisplayModeChanged, this, [window, navigationComboBox]() {
            const QSignalBlocker blocker(navigationComboBox);
            navigationComboBox->setCurrentIndex(int(window->getNavigationBarDisplayMode()));
        });
        connect(window, &ElaWindow::pStackSwitchModeChanged, this, [window, transitionComboBox]() {
            const QSignalBlocker blocker(transitionComboBox);
            transitionComboBox->setCurrentIndex(int(window->getStackSwitchMode()));
        });
    }
    _userCardSwitchButton = new ElaToggleSwitch(this);
    _userCardSwitchButton->setIsToggled(true);
    _userCardSwitchButton->setEnabled(window);
    if (window)
    {
        connect(_userCardSwitchButton, &ElaToggleSwitch::toggled, window, &ElaWindow::setUserInfoCardVisible);
    }
    navigationSection->addWidget(createSettingRow("导航栏模式", "让侧边栏适应当前窗口宽度", navigationComboBox));
    navigationSection->addWidget(createSettingRow("页面切换", "选择页面之间的过渡方式", transitionComboBox));
    navigationSection->addWidget(createSettingRow("显示用户卡片", "显示导航栏顶部的用户信息", _userCardSwitchButton));

    auto* applicationSection = addExampleSection(mainLayout, "应用", "控制展示程序的日志记录。");
    _logSwitchButton = new ElaToggleSwitch(this);
    connect(_logSwitchButton, &ElaToggleSwitch::toggled, this, [](bool checked) {
        ElaLog::getInstance()->initMessageLog(checked);
        qDebug() << (checked ? "日志已启用!" : "日志已关闭!");
    });
    applicationSection->addWidget(createSettingRow("启用日志功能", "日志会显示在日志面板并写入文件", _logSwitchButton));
}

T_Setting::~T_Setting()
{
}
