#include "T_DataView.h"

#include "ElaLineEdit.h"
#include "ElaPushButton.h"
#include "ElaStateWidget.h"
#include "ElaTableView.h"
#include "ElaText.h"
#include <QHeaderView>
#include <QHBoxLayout>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTimer>
#include <QVBoxLayout>

T_DataView::T_DataView(QWidget* parent) : T_BasePage(parent)
{
    setWindowTitle("数据加载与筛选");
    createCustomWidget("体验加载、空内容、失败重试和表格筛选；点击列标题排序。切换状态不会重建表格。");
    auto* central = new QWidget(this);
    central->setWindowTitle("数据加载与筛选");
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 16, 0);
    _filter = new ElaLineEdit(central);
    _filter->setPlaceholderText("搜索名称、分类或状态；清空可恢复全部结果");
    _filter->setAccessibleName("筛选数据");
    _filter->setIsClearButtonEnable(true);
    layout->addWidget(_filter);
    auto* actions = new QHBoxLayout;
    auto* reload = new ElaPushButton("加载数据", central);
    auto* empty = new ElaPushButton("加载空数据", central);
    auto* fail = new ElaPushButton("模拟失败", central);
    actions->addWidget(reload);
    actions->addWidget(empty);
    actions->addWidget(fail);
    actions->addStretch();
    layout->addLayout(actions);
    _count = new ElaText(central);
    _count->setTextStyle(ElaTextType::Body);
    layout->addWidget(_count);
    _state = new ElaStateWidget(central);
    _state->setMinimumHeight(340);
    _state->setStateText(ElaStateWidget::State::Loading, "正在加载数据", "请稍候，可以继续修改筛选条件。");
    _state->setStateText(ElaStateWidget::State::Error, "数据加载失败",
                         "这是模拟错误。点击重试重新加载，筛选条件会保留。");
    auto* table = new ElaTableView;
    _model = new QStandardItemModel(this);
    _model->setHorizontalHeaderLabels({"名称", "分类", "状态"});
    _proxy = new QSortFilterProxyModel(this);
    _proxy->setSourceModel(_model);
    _proxy->setFilterKeyColumn(-1);
    _proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    table->setModel(_proxy);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->hide();
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSortingEnabled(true);
    table->sortByColumn(0, Qt::AscendingOrder);
    _state->setContentWidget(table);
    layout->addWidget(_state, 1);
    addCentralWidget(central, true, true, 0);

    connect(_filter, &QLineEdit::textChanged, this, [this](const QString& text) {
        _proxy->setFilterFixedString(text);
        _updateResults();
    });
    connect(reload, &ElaPushButton::clicked, this, [this] {
        _load(Result::Data);
    });
    connect(empty, &ElaPushButton::clicked, this, [this] {
        _load(Result::Empty);
    });
    connect(fail, &ElaPushButton::clicked, this, [this] {
        _load(Result::Error);
    });
    connect(_state, &ElaStateWidget::retryRequested, this, [this] {
        _load(Result::Data);
    });
    _load(Result::Data);
}

void T_DataView::_load(Result result)
{
    // 请求状态和模型由页面管理，容器只负责展示。
    const quint64 request = ++_requestId;
    _loading = true;
    _failed = false;
    _count->setText("加载中…");
    _state->setState(ElaStateWidget::State::Loading);
    QTimer::singleShot(600, this, [this, request, result] {
        // 快速连续点击时只采纳最新请求；页面销毁后回调自动取消。
        if (request != _requestId)
        {
            return;
        }
        _loading = false;
        _failed = result == Result::Error;
        if (_failed)
        {
            _count->setText("加载未完成");
            _state->setState(ElaStateWidget::State::Error);
            return;
        }
        _model->removeRows(0, _model->rowCount());
        if (result == Result::Data)
        {
            const QList<QStringList> rows{{"窗口布局", "界面", "已完成"}, {"主题配色", "界面", "已完成"},
                                          {"异步搜索", "交互", "已完成"}, {"页面状态", "交互", "进行中"},
                                          {"数据导出", "数据", "待开始"}, {"列表筛选", "数据", "已完成"}};
            for (const auto& row : rows)
            {
                QList<QStandardItem*> items;
                for (const auto& text : row)
                {
                    items.append(new QStandardItem(text));
                }
                _model->appendRow(items);
            }
        }
        _updateResults();
    });
}

void T_DataView::_updateResults()
{
    // 过滤条件变化不应把加载中或错误提示覆盖成空列表。
    if (_loading || _failed)
    {
        return;
    }
    const int matches = _proxy->rowCount();
    _count->setText(QString("显示 %1 / %2 条").arg(matches).arg(_model->rowCount()));
    const bool noData = _model->rowCount() == 0;
    _state->setStateText(ElaStateWidget::State::Empty, noData ? "暂无数据" : "没有匹配结果",
                         noData ? "点击加载数据以获取示例记录。" : "修改或清空上方筛选条件后再试。");
    _state->setState(matches ? ElaStateWidget::State::Content : ElaStateWidget::State::Empty);
}
