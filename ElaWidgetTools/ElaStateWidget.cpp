#include "ElaStateWidget.h"

#include "ElaProgressRing.h"
#include "ElaPushButton.h"
#include "ElaText.h"
#include "private/ElaStateWidgetPrivate.h"
#include <QHideEvent>
#include <QScopedValueRollback>
#include <QShowEvent>
#include <QStackedWidget>
#include <QVBoxLayout>

ElaStateWidget::ElaStateWidget(QWidget* parent) : QWidget(parent), d_ptr(new ElaStateWidgetPrivate)
{
    Q_D(ElaStateWidget);
    d->q_ptr = this;
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    d->_stack = new QStackedWidget(this);
    layout->addWidget(d->_stack);
    d->_contentPage = new QWidget(d->_stack);
    d->_contentLayout = new QVBoxLayout(d->_contentPage);
    d->_contentLayout->setContentsMargins(0, 0, 0, 0);
    d->_stack->addWidget(d->_contentPage);
    d->_statusPage = new QWidget(d->_stack);
    auto* statusLayout = new QVBoxLayout(d->_statusPage);
    statusLayout->setContentsMargins(24, 24, 24, 24);
    statusLayout->setSpacing(12);
    statusLayout->addStretch();
    d->_ring = new ElaProgressRing(d->_statusPage);
    d->_ring->setFixedSize(40, 40);
    d->_ring->setBusyingWidth(4);
    d->_ring->setIsDisplayValue(false);
    statusLayout->addWidget(d->_ring, 0, Qt::AlignHCenter);
    d->_title = new ElaText(d->_statusPage);
    d->_title->setTextStyle(ElaTextType::Subtitle);
    d->_description = new ElaText(d->_statusPage);
    d->_description->setTextStyle(ElaTextType::Body);
    for (auto* text : {d->_title, d->_description})
    {
        text->setTextFormat(Qt::PlainText);
        text->setAlignment(Qt::AlignCenter);
        text->setWordWrap(true);
        text->setMinimumWidth(0);
        statusLayout->addWidget(text);
    }
    d->_retry = new ElaPushButton(tr("重试"), d->_statusPage);
    d->_retry->setFocusPolicy(Qt::StrongFocus);
    statusLayout->addWidget(d->_retry, 0, Qt::AlignHCenter);
    statusLayout->addStretch();
    d->_stack->addWidget(d->_statusPage);
    d->_titles = {QString(), tr("正在加载"), tr("暂无内容"), tr("加载失败")};
    connect(d->_retry, &ElaPushButton::clicked, this, [this] {
        Q_D(ElaStateWidget);
        if (d->_state == State::Error && d->_retryEnabled)
        {
            Q_EMIT retryRequested();
        }
    });
    d->_refresh();
}

ElaStateWidget::~ElaStateWidget()
{
    Q_D(ElaStateWidget);
    // QWidget 基类随后才销毁子控件，不能让 destroyed 回调访问已释放的私有对象。
    disconnect(d->_contentDestroyed);
}

void ElaStateWidget::setState(State state)
{
    Q_D(ElaStateWidget);
    if (state < State::Content || state > State::Error || d->_state == state || d->_updating)
    {
        return;
    }
    const auto keepAlive = d_ptr;
    {
        QScopedValueRollback<bool> updating(d->_updating, true);
        d->_state = state;
        if (!d->_refresh())
        {
            return;
        }
    }
    // 通知在视图更新后发出，允许应用在槽中切换状态或销毁容器。
    if (!d->_reconcileContent())
    {
        Q_EMIT stateChanged(state);
    }
}

ElaStateWidget::State ElaStateWidget::getState() const
{
    Q_D(const ElaStateWidget);
    return d->_state;
}

bool ElaStateWidget::setContentWidget(QWidget* widget)
{
    Q_D(ElaStateWidget);
    if (!widget || d->_updating)
    {
        return false;
    }
    if (widget == d->_content)
    {
        return true;
    }
    if (widget == this || widget == d->_stack || widget->isAncestorOf(this) ||
        (widget->parentWidget() && widget->parentWidget() != this))
    {
        return false;
    }
    const auto keepAlive = d_ptr;
    QPointer<ElaStateWidget> alive(this);
    QPointer<QWidget> incoming(widget);
    QPointer<QWidget> previous = d->_content;
    {
        QScopedValueRollback<bool> updating(d->_updating, true);
        disconnect(d->_contentDestroyed);
        d->_content = widget;
        d->_contentDestroyed = connect(widget, &QObject::destroyed, this, [this] {
            Q_D(ElaStateWidget);
            d->_content = nullptr;
            d->_contentRemoved = true;
            if (!d->_updating)
            {
                d->_reconcileContent();
            }
        });
        if (previous)
        {
            d->_contentLayout->removeWidget(previous);
            previous->hide();
            if (previous)
            {
                previous->deleteLater();
            }
            if (!alive)
            {
                return false;
            }
        }
        // 旧控件的隐藏／失焦回调可能销毁新控件，不能再使用原始指针。
        if (incoming)
        {
            d->_contentLayout->addWidget(incoming);
            if (!alive)
            {
                return false;
            }
            if (incoming)
            {
                incoming->show();
            }
        }
    }
    if (!alive)
    {
        return false;
    }
    d->_reconcileContent();
    return alive && incoming;
}

QWidget* ElaStateWidget::getContentWidget() const
{
    Q_D(const ElaStateWidget);
    return d->_content;
}

QWidget* ElaStateWidget::takeContentWidget()
{
    Q_D(ElaStateWidget);
    if (!d->_content || d->_updating)
    {
        return nullptr;
    }
    const auto keepAlive = d_ptr;
    QPointer<ElaStateWidget> alive(this);
    QPointer<QWidget> content = d->_content;
    {
        QScopedValueRollback<bool> updating(d->_updating, true);
        disconnect(d->_contentDestroyed);
        d->_content = nullptr;
        d->_contentLayout->removeWidget(content);
        content->hide();
        if (!alive)
        {
            return nullptr;
        }
        if (content)
        {
            content->setParent(nullptr);
        }
    }
    if (alive && d->_state == State::Content)
    {
        setState(State::Empty);
    }
    return content;
}

void ElaStateWidget::setStateText(State state, const QString& title, const QString& description)
{
    Q_D(ElaStateWidget);
    if (state <= State::Content || state > State::Error || d->_updating)
    {
        return;
    }
    const auto keepAlive = d_ptr;
    {
        QScopedValueRollback<bool> updating(d->_updating, true);
        const auto index = static_cast<size_t>(state);
        d->_titles[index] = title;
        d->_descriptions[index] = description;
        if (d->_state == state && !d->_refresh())
        {
            return;
        }
    }
    d->_reconcileContent();
}

void ElaStateWidget::setRetryEnabled(bool enabled)
{
    Q_D(ElaStateWidget);
    if (d->_updating || d->_retryEnabled == enabled)
    {
        return;
    }
    const auto keepAlive = d_ptr;
    {
        QScopedValueRollback<bool> updating(d->_updating, true);
        d->_retryEnabled = enabled;
        if (!d->_refresh())
        {
            return;
        }
    }
    d->_reconcileContent();
}

bool ElaStateWidget::getRetryEnabled() const
{
    Q_D(const ElaStateWidget);
    return d->_retryEnabled;
}

void ElaStateWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    Q_D(ElaStateWidget);
    d->_updateAnimation();
}

void ElaStateWidget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    Q_D(ElaStateWidget);
    d->_updateAnimation();
}

bool ElaStateWidgetPrivate::_refresh()
{
    Q_Q(ElaStateWidget);
    QPointer<ElaStateWidget> alive(q);
    const auto index = static_cast<size_t>(_state);
    _title->setText(_titles[index]);
    _description->setText(_descriptions[index]);
    _title->setVisible(!_titles[index].isEmpty());
    _description->setVisible(!_descriptions[index].isEmpty());
    _ring->setVisible(_state == ElaStateWidget::State::Loading);
    _retry->setVisible(_state == ElaStateWidget::State::Error && _retryEnabled);
    _stack->setCurrentWidget(_state == ElaStateWidget::State::Content ? _contentPage : _statusPage);
    if (!alive)
    {
        return false;
    }
    _updateAnimation();
    return alive;
}

void ElaStateWidgetPrivate::_updateAnimation()
{
    Q_Q(ElaStateWidget);
    const bool busy = _state == ElaStateWidget::State::Loading && q->isVisible();
    if (_ring->getIsBusying() != busy)
    {
        _ring->setIsBusying(busy);
    }
}

bool ElaStateWidgetPrivate::_reconcileContent()
{
    Q_Q(ElaStateWidget);
    if (!_contentRemoved)
    {
        return false;
    }
    _contentRemoved = false;
    // 显示／隐藏会同步触发应用回调，等本次布局更新退出后再协调空内容状态。
    if (!_content && _state == ElaStateWidget::State::Content)
    {
        q->setState(ElaStateWidget::State::Empty);
        return true;
    }
    return false;
}
