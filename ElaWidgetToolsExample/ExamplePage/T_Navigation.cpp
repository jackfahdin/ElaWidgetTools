#include "T_Navigation.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>

#include "ElaBreadcrumbBar.h"
#include "ElaPivot.h"
#include "ElaPushButton.h"
#include "ElaScrollPageArea.h"
#include "ElaTabWidget.h"
#include "ElaText.h"
#include "ElaSuggestBox.h"
T_Navigation::T_Navigation(QWidget* parent) : T_BasePage(parent)
{
    // 预览窗口标题
    setWindowTitle("ElaNavigation");

    // 顶部元素
    createCustomWidget("一些导航组件被放置于此，可在此界面体验其效果并按需添加进项目中");

    // ElaBreadcrumbBar
    ElaText* breadcrumbBarText = new ElaText("ElaBreadcrumbBar", this);
    breadcrumbBarText->setTextPixelSize(18);
    _breadcrumbBar = new ElaBreadcrumbBar(this);
    QStringList breadcrumbBarList;
    for (int i = 0; i < 20; i++)
    {
        breadcrumbBarList << QString("Item%1").arg(i + 1);
    }
    _breadcrumbBar->setBreadcrumbList(breadcrumbBarList);

    ElaPushButton* resetButton = new ElaPushButton("还原", this);
    resetButton->setFixedSize(60, 32);
    connect(resetButton, &ElaPushButton::clicked, this, [=]() {
        _breadcrumbBar->setBreadcrumbList(breadcrumbBarList);
    });

    QHBoxLayout* breadcrumbBarTextLayout = new QHBoxLayout();
    breadcrumbBarTextLayout->addWidget(breadcrumbBarText);
    breadcrumbBarTextLayout->addSpacing(15);
    breadcrumbBarTextLayout->addWidget(resetButton);
    breadcrumbBarTextLayout->addStretch();

    ElaScrollPageArea* breadcrumbBarArea = new ElaScrollPageArea(this);
    QVBoxLayout* breadcrumbBarLayout = new QVBoxLayout(breadcrumbBarArea);
    breadcrumbBarLayout->addWidget(_breadcrumbBar);

    // ElaPivot
    ElaText* pivotText = new ElaText("ElaPivot", this);
    pivotText->setTextPixelSize(18);
    _pivot = new ElaPivot(this);
    _pivot->setPivotSpacing(8);
    _pivot->setMarkWidth(75);
    _pivot->appendPivot("本地歌曲");
    _pivot->appendPivot("下载歌曲");
    _pivot->appendPivot("下载视频");
    _pivot->appendPivot("正在下载");
    _pivot->appendPivot("本地歌曲");
    _pivot->appendPivot("下载歌曲");
    _pivot->appendPivot("下载视频");
    _pivot->appendPivot("正在下载");
    _pivot->appendPivot("本地歌曲");
    _pivot->appendPivot("下载歌曲");
    _pivot->appendPivot("下载视频");
    _pivot->appendPivot("正在下载");
    _pivot->setCurrentIndex(0);

    ElaScrollPageArea* pivotArea = new ElaScrollPageArea(this);
    QVBoxLayout* pivotLayout = new QVBoxLayout(pivotArea);
    pivotLayout->addWidget(_pivot);

    // ElaTabWidget
    ElaText* tabWidgetText = new ElaText("ElaTabWidget", this);
    tabWidgetText->setTextPixelSize(18);
    _tabWidget = new ElaTabWidget(this);
    _tabWidget->setFixedHeight(600);
    _tabWidget->setIsTabTransparent(true);
    ElaText* page1 = new ElaText("新标签页", this);
    page1->setTextPixelSize(32);
    page1->setAlignment(Qt::AlignCenter);
    _tabWidget->addTab(page1, QIcon(":/Resource/Image/Cirno.jpg"), "新标签页");
    for (int i = 0; i < 5; i++)
    {
        ElaText* page = new ElaText(QString("新标签页%1").arg(i), this);
        page->setTextPixelSize(32);
        page->setAlignment(Qt::AlignCenter);
        _tabWidget->addTab(page, QString("新标签页%1").arg(i));
    }
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

    QWidget* centralWidget = new QWidget(this);
    centralWidget->setWindowTitle("ElaNavigation");
    QVBoxLayout* centerVLayout = new QVBoxLayout(centralWidget);
    centerVLayout->setContentsMargins(0, 0, 0, 0);
    centerVLayout->addWidget(searchTitle);
    centerVLayout->addWidget(searchArea);
    centerVLayout->addSpacing(15);
    centerVLayout->addLayout(breadcrumbBarTextLayout);
    centerVLayout->addSpacing(10);
    centerVLayout->addWidget(breadcrumbBarArea);
    centerVLayout->addSpacing(15);
    centerVLayout->addWidget(pivotText);
    centerVLayout->addSpacing(10);
    centerVLayout->addWidget(pivotArea);
    centerVLayout->addSpacing(15);
    centerVLayout->addWidget(tabWidgetText);
    centerVLayout->addSpacing(10);
    centerVLayout->addWidget(_tabWidget);
    centerVLayout->addStretch();
    addCentralWidget(centralWidget, true, false, 0);
}

T_Navigation::~T_Navigation()
{
}
