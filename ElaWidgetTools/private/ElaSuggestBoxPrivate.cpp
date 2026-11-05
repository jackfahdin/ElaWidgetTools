#include "ElaSuggestBoxPrivate.h"

#include "ElaBaseListView.h"
#include "ElaLineEdit.h"
#include "ElaSuggestBoxSearchViewContainer.h"
#include "ElaSuggestModel.h"
#include <QLayout>
#include <QPropertyAnimation>
#include <QSignalBlocker>
#include <QTimer>

ElaSuggestBoxPrivate::ElaSuggestBoxPrivate(QObject* parent) : QObject{parent}
{
}
ElaSuggestBoxPrivate::~ElaSuggestBoxPrivate() = default;

void ElaSuggestBoxPrivate::onThemeModeChanged(ElaThemeType::ThemeMode themeMode)
{
    _themeMode = themeMode;
    _searchEdit->removeAction(_themeMode == ElaThemeType::Light ? _darkSearchAction : _lightSearchAction);
    _searchEdit->addAction(_themeMode == ElaThemeType::Light ? _lightSearchAction : _darkSearchAction,
                           QLineEdit::TrailingPosition);
    _searchEdit->update();
}

void ElaSuggestBoxPrivate::_publishPending()
{
    Q_Q(ElaSuggestBox);
    const bool pending = _activeRequestId != 0;
    if (_pendingNotified != pending)
    {
        _pendingNotified = pending;
        Q_EMIT q->searchPendingChanged(pending);
    }
}

bool ElaSuggestBoxPrivate::_cancelRequest()
{
    Q_Q(ElaSuggestBox);
    const auto keepAlive = q->d_ptr;
    const QPointer<ElaSuggestBox> alive(q);
    const quint64 revision = _queryRevision;
    _searchTimer->stop();
    const quint64 id = _activeRequestId;
    const bool dispatched = _requestDispatched;
    _activeRequestId = 0;
    _requestDispatched = false;
    if (id)
    {
        if (dispatched)
        {
            Q_EMIT q->searchCancelled(id);
            if (!alive || revision != _queryRevision)
            {
                return false;
            }
        }
    }
    _publishPending();
    return alive && revision == _queryRevision;
}

void ElaSuggestBoxPrivate::onSearchEditTextEdit(const QString& searchText)
{
    Q_Q(ElaSuggestBox);
    const QString query = searchText;
    ++_queryRevision;
    if (!_cancelRequest())
    {
        return;
    }
    if (query.isEmpty())
    {
        _showSuggestions({});
        return;
    }
    if (_asyncSearchEnabled)
    {
        _showSuggestions({});
        _activeRequestId = ++_nextRequestId;
        _searchTimer->start(_debounceInterval);
        _publishPending();
        return;
    }
    QList<ElaSuggestBox::SuggestData> matches;
    for (const auto& suggestion : _suggestions)
    {
        if (suggestion.getSuggestText().contains(query, _pCaseSensitivity))
        {
            matches.append(suggestion);
        }
    }
    _showSuggestions(matches);
}

void ElaSuggestBoxPrivate::_showSuggestions(const QList<ElaSuggestBox::SuggestData>& suggestions)
{
    Q_Q(ElaSuggestBox);
    _searchModel->setSearchSuggestion(suggestions);
    _searchView->setCurrentIndex({});
    if (suggestions.isEmpty())
    {
        _startCloseAnimation();
        return;
    }
    const int rows = qMin(4, suggestions.count());
    const QSize target(q->width() + 12, 40 * rows + 16);
    const bool visible = _searchViewBaseWidget->isVisible();
    _searchViewBaseWidget->move(q->mapTo(q->window(), QPoint(-5, q->height())));
    _shadowLayout->setContentsMargins(8, 8, 8, 8);
    _searchViewBaseWidget->show();
    _searchViewBaseWidget->raise();
    _startSizeAnimation(visible ? _searchViewBaseWidget->size() : QSize(target.width(), 0), target);
}

void ElaSuggestBoxPrivate::onSearchViewClicked(const QModelIndex& index)
{
    Q_Q(ElaSuggestBox);
    const auto* suggestion = index.isValid() ? _searchModel->getSearchSuggestion(index.row()) : nullptr;
    if (!suggestion)
    {
        return;
    }
    // 模型重置前复制数据；设置选中文字不应触发另一轮搜索。
    const auto data = *suggestion;
    {
        const QSignalBlocker blocker(_searchEdit);
        _searchEdit->setText(data.getSuggestText());
    }
    const QPointer<ElaSuggestBox> alive(q);
    q->cancelSearch();
    if (alive)
    {
        Q_EMIT q->suggestionClicked(data);
    }
}

void ElaSuggestBoxPrivate::_startSizeAnimation(QSize oldSize, QSize newSize)
{
    Q_Q(ElaSuggestBox);
    if (_sizeAnimation)
    {
        _sizeAnimation->stop();
        _sizeAnimation->deleteLater();
    }
    _sizeAnimation = new QPropertyAnimation(_searchViewBaseWidget, "size", q);
    _sizeAnimation->setDuration(200);
    _sizeAnimation->setEasingCurve(QEasingCurve::InOutSine);
    _sizeAnimation->setStartValue(oldSize);
    _sizeAnimation->setEndValue(newSize);
    _sizeAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void ElaSuggestBoxPrivate::_startCloseAnimation()
{
    if (!_searchViewBaseWidget->isVisible())
    {
        return;
    }
    _startSizeAnimation(_searchViewBaseWidget->size(), QSize(_searchViewBaseWidget->width(), 0));
    connect(_sizeAnimation, &QPropertyAnimation::finished, this, [this]() {
        _searchViewBaseWidget->hide();
    });
}
