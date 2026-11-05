#include "ElaSuggestBox.h"

#include <QAction>
#include <QDebug>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListView>
#include <QMap>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QTimer>
#include <QUuid>
#include <QKeyEvent>
#include <QHideEvent>
#include <QSignalBlocker>
#include <QPropertyAnimation>

#include "ElaBaseListView.h"
#include "ElaIcon.h"
#include "ElaLineEdit.h"
#include "ElaScrollBar.h"
#include "ElaSuggestBoxSearchViewContainer.h"
#include "ElaSuggestDelegate.h"
#include "ElaSuggestModel.h"
#include "ElaTheme.h"
#include "private/ElaSuggestBoxPrivate.h"

ELA_PROPERTY_Q_CPP(ElaSuggestBox, int, BorderRadius)
ELA_PROPERTY_Q_CPP(ElaSuggestBox, Qt::CaseSensitivity, CaseSensitivity)
ElaSuggestBox::SuggestData::SuggestData() : _pElaIcon(ElaIconType::None)
{
}

ElaSuggestBox::SuggestData::SuggestData(ElaIconType::IconName icon, const QString& suggestText,
                                        const QVariantMap& suggestData)
    : _pElaIcon(icon), _pSuggestText(suggestText), _pSuggestData(suggestData)
{
}

ElaSuggestBox::SuggestData::~SuggestData()
{
}

ElaSuggestBox::ElaSuggestBox(QWidget* parent) : QWidget{parent}, d_ptr(new ElaSuggestBoxPrivate())
{
    Q_D(ElaSuggestBox);
    QWidget::setFixedSize(280, 35);
    d->q_ptr = this;
    d->_pBorderRadius = 6;
    d->_pCaseSensitivity = Qt::CaseInsensitive;
    d->_searchEdit = new ElaLineEdit(this);
    d->_searchEdit->setFixedHeight(35);
    d->_searchEdit->setPlaceholderText("查找功能");
    d->_searchEdit->setClearButtonEnabled(true);
    d->_lightSearchAction =
        new QAction(ElaIcon::getInstance()->getElaIcon(ElaIconType::MagnifyingGlass), "Search", this);
    d->_darkSearchAction = new QAction(
        ElaIcon::getInstance()->getElaIcon(ElaIconType::MagnifyingGlass, QColor(0xFF, 0xFF, 0xFF)), "Search", this);

    d->_themeMode = eTheme->getThemeMode();
    connect(eTheme, &ElaTheme::themeModeChanged, d, &ElaSuggestBoxPrivate::onThemeModeChanged);
    if (d->_themeMode == ElaThemeType::Light)
    {
        d->_searchEdit->addAction(d->_lightSearchAction, QLineEdit::TrailingPosition);
    }
    else
    {
        d->_searchEdit->addAction(d->_darkSearchAction, QLineEdit::TrailingPosition);
    }

    connect(d->_lightSearchAction, &QAction::triggered, this, [=](bool checked) {
        // qDebug() << "Search";
    });

    connect(d->_darkSearchAction, &QAction::triggered, this, [=](bool checked) {
        // qDebug() << "Search";
    });
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(d->_searchEdit);
    d->_searchViewBaseWidget = new ElaSuggestBoxSearchViewContainer(window());
    d->_shadowLayout = new QVBoxLayout(d->_searchViewBaseWidget);
    d->_shadowLayout->setContentsMargins(0, 0, 0, 0);
    d->_searchView = new ElaBaseListView(d->_searchViewBaseWidget);
    ElaScrollBar* floatVScrollBar = new ElaScrollBar(d->_searchView->verticalScrollBar(), d->_searchView);
    floatVScrollBar->setIsAnimation(true);
    d->_searchViewBaseWidget->resize(292, 300);
    d->_shadowLayout->addWidget(d->_searchView);
    d->_searchModel = new ElaSuggestModel(this);
    d->_searchDelegate = new ElaSuggestDelegate(this);
    d->_searchView->setModel(d->_searchModel);
    d->_searchView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->_searchView->setItemDelegate(d->_searchDelegate);
    d->_searchViewBaseWidget->hide();
    d->_searchTimer = new QTimer(this);
    d->_searchTimer->setSingleShot(true);
    connect(d->_searchTimer, &QTimer::timeout, this, [=]() {
        if (!d->_activeRequestId || !d->_asyncSearchEnabled)
        {
            return;
        }
        d->_requestDispatched = true;
        Q_EMIT searchRequested(d->_activeRequestId, d->_searchEdit->text());
    });
    connect(d->_searchEdit, &ElaLineEdit::textChanged, d, &ElaSuggestBoxPrivate::onSearchEditTextEdit);
    connect(d->_searchEdit, &ElaLineEdit::focusIn, this, [=](const QString& text) {
        if (!d->_activeRequestId && d->_searchModel->rowCount({}) == 0)
        {
            d->onSearchEditTextEdit(text);
        }
    });
    d->_searchEdit->installEventFilter(this);
    d->_searchView->installEventFilter(this);
    connect(d->_searchView, &ElaBaseListView::clicked, d, &ElaSuggestBoxPrivate::onSearchViewClicked);

    // 焦点事件
    connect(d->_searchEdit, &ElaLineEdit::wmFocusOut, this, [=]() {
        if (!d->_searchView->underMouse())
        {
            cancelSearch();
        }
    });
}

ElaSuggestBox::~ElaSuggestBox()
{
    Q_D(ElaSuggestBox);
    // QWidget 基类析构仍可能触发输入框失焦，必须先解除访问私有状态的回调。
    d->_searchTimer->stop();
    d->_searchEdit->removeEventFilter(this);
    d->_searchView->removeEventFilter(this);
    disconnect(d->_searchEdit, nullptr, this, nullptr);
    disconnect(d->_searchEdit, nullptr, d, nullptr);
    disconnect(d->_searchView, nullptr, d, nullptr);
    if (d->_sizeAnimation)
    {
        d->_sizeAnimation->stop();
    }
    // 弹出层挂在顶层窗口，不能依赖搜索框的子对象析构顺序。
    delete d->_searchViewBaseWidget.data();
}

void ElaSuggestBox::setPlaceholderText(const QString& placeholderText)
{
    Q_D(ElaSuggestBox);
    d->_searchEdit->setPlaceholderText(placeholderText);
}

void ElaSuggestBox::setFixedSize(const QSize& size)
{
    Q_D(ElaSuggestBox);
    d->_searchEdit->setFixedHeight(size.height());
    QWidget::setFixedSize(size);
}

void ElaSuggestBox::setFixedSize(int w, int h)
{
    Q_D(ElaSuggestBox);
    d->_searchEdit->setFixedHeight(h);
    QWidget::setFixedSize(w, h);
}

void ElaSuggestBox::setFixedHeight(int h)
{
    Q_D(ElaSuggestBox);
    d->_searchEdit->setFixedHeight(h);
    QWidget::setFixedHeight(h);
}

QString ElaSuggestBox::addSuggestion(const QString& suggestText, const QVariantMap& suggestData)
{
    return addSuggestion(SuggestData(ElaIconType::None, suggestText, suggestData));
}

QString ElaSuggestBox::addSuggestion(ElaIconType::IconName icon, const QString& suggestText,
                                     const QVariantMap& suggestData)
{
    return addSuggestion(SuggestData(icon, suggestText, suggestData));
}

QString ElaSuggestBox::addSuggestion(const ElaSuggestBox::SuggestData& suggestData)
{
    Q_D(ElaSuggestBox);
    auto suggestion = suggestData;
    suggestion.setSuggestKey(QUuid::createUuid().toString(QUuid::Id128));
    d->_suggestions.append(suggestion);
    if (!d->_asyncSearchEnabled)
    {
        d->onSearchEditTextEdit(getSearchText());
    }
    return suggestion.getSuggestKey();
}

QStringList ElaSuggestBox::addSuggestion(const QList<ElaSuggestBox::SuggestData>& suggestDataList)
{
    QStringList keys;
    for (const auto& suggestion : suggestDataList)
    {
        keys.append(addSuggestion(suggestion));
    }
    return keys;
}

void ElaSuggestBox::removeSuggestion(const QString& suggestKey)
{
    Q_D(ElaSuggestBox);
    for (int i = 0; i < d->_suggestions.count(); ++i)
    {
        if (d->_suggestions[i].getSuggestKey() == suggestKey)
        {
            removeSuggestion(i);
            return;
        }
    }
}

void ElaSuggestBox::removeSuggestion(int index)
{
    Q_D(ElaSuggestBox);
    if (index < 0 || index >= d->_suggestions.count())
    {
        return;
    }
    d->_suggestions.removeAt(index);
    if (!d->_asyncSearchEnabled)
    {
        d->onSearchEditTextEdit(getSearchText());
    }
}

void ElaSuggestBox::clearSuggestion()
{
    Q_D(ElaSuggestBox);
    d->_suggestions.clear();
    if (!d->_asyncSearchEnabled)
    {
        d->onSearchEditTextEdit(getSearchText());
    }
}

void ElaSuggestBox::setSearchText(const QString& text)
{
    Q_D(ElaSuggestBox);
    d->_searchEdit->setText(text);
}

QString ElaSuggestBox::getSearchText() const
{
    Q_D(const ElaSuggestBox);
    return d->_searchEdit->text();
}

void ElaSuggestBox::setAsyncSearchEnabled(bool enabled)
{
    Q_D(ElaSuggestBox);
    if (d->_asyncSearchEnabled == enabled)
    {
        return;
    }
    d->_asyncSearchEnabled = enabled;
    ++d->_queryRevision;
    if (!d->_cancelRequest())
    {
        return;
    }
    d->onSearchEditTextEdit(getSearchText());
}

bool ElaSuggestBox::getAsyncSearchEnabled() const
{
    Q_D(const ElaSuggestBox);
    return d->_asyncSearchEnabled;
}

void ElaSuggestBox::setSearchDebounceInterval(int milliseconds)
{
    Q_D(ElaSuggestBox);
    d->_debounceInterval = qMax(0, milliseconds);
    if (d->_searchTimer->isActive())
    {
        d->_searchTimer->start(d->_debounceInterval);
    }
}

int ElaSuggestBox::getSearchDebounceInterval() const
{
    Q_D(const ElaSuggestBox);
    return d->_debounceInterval;
}

bool ElaSuggestBox::getIsSearchPending() const
{
    Q_D(const ElaSuggestBox);
    return d->_activeRequestId != 0;
}

bool ElaSuggestBox::setSearchResults(quint64 requestId, const QList<SuggestData>& results)
{
    Q_D(ElaSuggestBox);
    if (!d->_asyncSearchEnabled || !requestId || requestId != d->_activeRequestId || !d->_requestDispatched)
    {
        return false;
    }
    d->_activeRequestId = 0;
    d->_requestDispatched = false;
    auto suggestions = results;
    for (auto& suggestion : suggestions)
    {
        if (suggestion.getSuggestKey().isEmpty())
        {
            suggestion.setSuggestKey(QUuid::createUuid().toString(QUuid::Id128));
        }
    }
    const QPointer<ElaSuggestBox> alive(this);
    d->_showSuggestions(suggestions);
    if (alive)
    {
        d->_publishPending();
    }
    return true;
}

void ElaSuggestBox::cancelSearch()
{
    Q_D(ElaSuggestBox);
    ++d->_queryRevision;
    if (d->_cancelRequest())
    {
        d->_showSuggestions({});
    }
}

bool ElaSuggestBox::eventFilter(QObject* watched, QEvent* event)
{
    Q_D(ElaSuggestBox);
    if ((watched == d->_searchEdit || watched == d->_searchView) && event->type() == QEvent::KeyPress)
    {
        auto* key = static_cast<QKeyEvent*>(event);
        if (key->key() == Qt::Key_Escape)
        {
            cancelSearch();
            return true;
        }
        const int count = d->_searchModel->rowCount({});
        if (count > 0 && (key->key() == Qt::Key_Down || key->key() == Qt::Key_Up))
        {
            const int current = d->_searchView->currentIndex().row();
            const int next = current < 0 ? (key->key() == Qt::Key_Down ? 0 : count - 1)
                                         : qBound(0, current + (key->key() == Qt::Key_Down ? 1 : -1), count - 1);
            d->_searchView->setCurrentIndex(d->_searchModel->index(next, 0));
            return true;
        }
        if (count > 0 && (key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter))
        {
            const auto index = d->_searchView->currentIndex();
            d->onSearchViewClicked(index.isValid() ? index : d->_searchModel->index(0, 0));
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void ElaSuggestBox::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    cancelSearch();
}
