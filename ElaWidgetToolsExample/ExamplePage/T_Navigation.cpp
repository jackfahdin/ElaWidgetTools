#include "T_Navigation.h"

#include "ElaBreadcrumbBar.h"
#include "ElaFlowLayout.h"
#include "ElaMenu.h"
#include "ElaPivot.h"
#include "ElaPushButton.h"
#include "ElaTabWidget.h"
#include "ElaText.h"
#include "ElaSuggestBox.h"
#include "ElaScrollPageArea.h"
#include "ElaSplitter.h"
#include <QTimer>

#include <QVBoxLayout>

T_Navigation::T_Navigation(QWidget* parent) : T_BasePage(parent)
{
    setWindowTitle("导航与标签");
    createCustomWidget("通过路径、分类与标签组织内容，点击控件查看当前导航位置。", "导航与标签");
    auto* mainLayout = createPageLayout();
    // 模拟外部查询：不同文本返回速度不同，旧请求即使返回也不会覆盖新结果。
    auto* searchArea = new ElaScrollPageArea(this);
    auto* searchLayout = new QVBoxLayout(searchArea);
    auto* searchTitle = new ElaText("异步搜索与过期结果取消", 18, this);
    auto* search = new ElaSuggestBox(searchArea);
    search->setPlaceholderText("输入页面名称，例如：主题、导航");
    search->setAsyncSearchEnabled(true);
    search->setSearchDebounceInterval(200);
    auto* status = new ElaText("等待输入", 14, searchArea);
    auto* hint = new ElaText("连续修改查询可体验过期结果处理；使用上下键选择，Enter 确认，Esc 取消。", 13, searchArea);
    searchLayout->addWidget(search);
    searchLayout->addWidget(status);
    searchLayout->addWidget(hint);
    connect(search, &ElaSuggestBox::searchPendingChanged, status, [=](bool pending) {
        if (pending)
            status->setText("正在查询…");
    });
    connect(search, &ElaSuggestBox::searchCancelled, status, [=](quint64) {
        status->setText("上一条查询已取消");
    });
    connect(search, &ElaSuggestBox::searchRequested, this, [=](quint64 requestId, const QString& text) {
        const QStringList pages{"主题设置", "导航组件", "异步搜索", "懒加载编辑器", "页面管理", "表格视图"};
        const int latency = text.size() % 2 ? 900 : 300;
        QTimer::singleShot(latency, search, [=]() {
            QList<ElaSuggestBox::SuggestData> results;
            for (const auto& page : pages)
            {
                if (page.contains(text, Qt::CaseInsensitive))
                    results.append(ElaSuggestBox::SuggestData(ElaIconType::MagnifyingGlass, page));
            }
            if (search->setSearchResults(requestId, results))
                status->setText(results.isEmpty() ? "没有匹配结果" : QString("找到 %1 条结果").arg(results.size()));
        });
    });
    connect(search, &ElaSuggestBox::suggestionClicked, status, [=](const ElaSuggestBox::SuggestData& data) {
        status->setText("已选择：" + data.getSuggestText());
    });

    mainLayout->addWidget(searchTitle);
    mainLayout->addWidget(searchArea);
    auto* breadcrumbSection = addExampleSection(mainLayout, "路径导航 · ElaBreadcrumbBar",
                                                "点击上级路径返回，使用还原按钮重新展示完整路径。");
    const QStringList path{"工作空间", "设计系统", "组件库", "导航组件"};
    _breadcrumbBar = new ElaBreadcrumbBar(this);
    _breadcrumbBar->setBreadcrumbList(path);
    auto* resetButton = new ElaPushButton("还原路径", this);
    resetButton->setFixedWidth(100);
    auto* breadcrumbResultText = new ElaText("当前位置：导航组件", this);
    connect(resetButton, &ElaPushButton::clicked, this, [this, path, breadcrumbResultText]() {
        _breadcrumbBar->setBreadcrumbList(path);
        breadcrumbResultText->setText("当前位置：导航组件");
    });
    connect(_breadcrumbBar, &ElaBreadcrumbBar::breadcrumbClicked, this,
            [breadcrumbResultText](const QString& value, const QStringList&) {
                breadcrumbResultText->setText("当前位置：" + value);
            });
    breadcrumbSection->addWidget(_breadcrumbBar);
    auto* breadcrumbActionLayout = new QHBoxLayout();
    breadcrumbActionLayout->addWidget(breadcrumbResultText, 1);
    breadcrumbActionLayout->addWidget(resetButton);
    breadcrumbSection->addLayout(breadcrumbActionLayout);

    auto* pivotSection = addExampleSection(mainLayout, "分类切换 · ElaPivot", "点击分类，观察选中标记和内容的联动。");
    _pivot = new ElaPivot(this);
    _pivot->setPivotSpacing(18);
    _pivot->setMarkWidth(28);
    const QStringList categories{"概览", "设计", "开发", "发布"};
    for (const QString& title : categories)
    {
        _pivot->appendPivot(title);
    }
    _pivot->setCurrentIndex(0);
    auto* pivotResultText = new ElaText("概览：查看项目进度与最近活动。", this);
    const QStringList descriptions{"概览：查看项目进度与最近活动。", "设计：整理视觉规范和交互说明。",
                                   "开发：浏览组件实现与接口。", "发布：检查版本信息与交付清单。"};
    connect(_pivot, &ElaPivot::pivotClicked, this, [pivotResultText, descriptions](int index) {
        pivotResultText->setText(descriptions.value(index));
    });
    pivotSection->addWidget(_pivot);
    pivotSection->addWidget(pivotResultText);

    auto* tabSection = addExampleSection(mainLayout, "多文档标签 · ElaTabWidget",
                                         "切换、拖动或关闭标签；右键可固定标签，也可以新增一个页面。");
    auto* addButton = new ElaPushButton("新增标签", this);
    _tabWidget = new ElaTabWidget(this);
    _tabWidget->setFixedHeight(280);
    _tabWidget->setIsTabTransparent(true);
    _tabWidget->setIsTabWidthAdaptive(true);
    for (const QString& title : {QString("项目概览"), QString("界面设计"), QString("开发记录")})
    {
        auto* pageText = new ElaText(title + "\n\n这是一个可以独立切换的文档页面", this);
        pageText->setTextPixelSize(18);
        pageText->setAlignment(Qt::AlignCenter);
        _tabWidget->addTab(pageText, title);
    }
    connect(addButton, &ElaPushButton::clicked, this, [this]() {
        const QString title = QString("新文档 %1").arg(_tabWidget->count() + 1);
        auto* pageText = new ElaText(title, this);
        pageText->setTextPixelSize(18);
        pageText->setAlignment(Qt::AlignCenter);
        _tabWidget->setCurrentIndex(_tabWidget->addTab(pageText, title));
    });
    connect(_tabWidget, &ElaTabWidget::tabCloseRequested, this, [this](int index) {
        QWidget* page = _tabWidget->widget(index);
        _tabWidget->removeTab(index);
        page->deleteLater();
    });
    connect(_tabWidget, &ElaTabWidget::tabContextMenuRequested, this, [this](int index, const QPoint& position) {
        ElaMenu menu(this);
        const bool isPinned = _tabWidget->getIsTabPinned(index);
        connect(menu.addAction(isPinned ? "取消固定" : "固定标签"), &QAction::triggered, this,
                [this, index, isPinned]() {
                    _tabWidget->setIsTabPinned(index, !isPinned);
                });
        menu.exec(position);
    });
    tabSection->addWidget(addButton, 0, Qt::AlignLeft);
    tabSection->addWidget(_tabWidget);

    auto* splitterSection =
        addExampleSection(mainLayout, "可调节布局 · ElaSplitter", "拖动中间分隔线，调整两侧内容宽度。");
    auto* splitter = new ElaSplitter(Qt::Horizontal, this);
    splitter->setChildrenCollapsible(false);
    splitter->setFixedHeight(120);
    for (const QString& title : {QString("导航面板"), QString("内容面板")})
    {
        auto* pane = new ElaText(title, splitter);
        pane->setAlignment(Qt::AlignCenter);
        splitter->addWidget(pane);
    }
    splitter->setSizes({160, 320});
    splitterSection->addWidget(splitter);
}

T_Navigation::~T_Navigation()
{
}
