#include "ElaWindow.h"

#include "ElaCentralStackedWidget.h"
#include "ElaNavigationBar.h"
#include "private/ElaWindowPrivate.h"
#include <QShowEvent>

ElaNavigationType::NodeResult ElaWindow::addLazyPageNode(const QString& title, QString& pageKey,
                                                         const PageFactory& factory, ElaIconType::IconName awesome)
{
    return addLazyPageNode(title, pageKey, factory, QString(), awesome);
}

ElaNavigationType::NodeResult ElaWindow::addLazyPageNode(const QString& title, QString& pageKey,
                                                         const PageFactory& factory, const QString& targetExpanderKey,
                                                         ElaIconType::IconName awesome)
{
    Q_D(ElaWindow);
    pageKey.clear();
    if (!factory)
    {
        return ElaNavigationType::PageInvalid;
    }
    // 占位页只保留堆栈位置，昂贵的页面在真正访问时创建。
    auto* placeholder = new QWidget;
    const auto result = targetExpanderKey.isEmpty() ? addPageNode(title, placeholder, awesome)
                                                    : addPageNode(title, placeholder, targetExpanderKey, awesome);
    if (result != ElaNavigationType::Success)
    {
        delete placeholder;
        return result;
    }
    pageKey = placeholder->property("ElaPageKey").toString();
    d->_pageFactories.insert(pageKey, factory);
    d->_pendingPageKeys.insert(pageKey);
    d->_navigationBar->_setPageFactory(pageKey, factory);
    if (isVisible() && d->_navigationTargetKey == pageKey)
    {
        d->_navigationBar->tryNavigation(pageKey, false);
    }
    return result;
}

QWidget* ElaWindow::getNavigationPage(const QString& pageKey) const
{
    Q_D(const ElaWindow);
    return d->_pendingPageKeys.contains(pageKey) ? nullptr : d->_routeMap.value(pageKey).data();
}

void ElaWindow::setNavigationGuard(const NavigationGuard& guard)
{
    Q_D(ElaWindow);
    d->_navigationGuard = guard;
}

void ElaWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    Q_D(ElaWindow);
    if (d->_pendingPageKeys.contains(d->_navigationTargetKey))
    {
        d->_navigationBar->tryNavigation(d->_navigationTargetKey, false);
    }
}

bool ElaWindowPrivate::_prepareNavigation(const QString& key)
{
    Q_Q(ElaWindow);
    const auto keepAlive = q->d_ptr;
    QPointer<ElaWindow> alive(q);
    if (!_routeMap.contains(key))
    {
        return false;
    }
    const auto guard = _navigationGuard;
    if (key != _navigationTargetKey && guard && !guard(_navigationTargetKey, key))
    {
        return false;
    }
    if (!alive || !_routeMap.contains(key))
    {
        return false;
    }
    return _createLazyPage(key);
}

bool ElaWindowPrivate::_createLazyPage(const QString& key, bool recreate)
{
    Q_Q(ElaWindow);
    if (!_pageFactories.contains(key))
    {
        return _routeMap.value(key);
    }
    if (!recreate && !_pendingPageKeys.contains(key) && _routeMap.value(key))
    {
        return true;
    }
    const auto keepAlive = q->d_ptr;
    QPointer<ElaWindow> alive(q);
    auto* stack = _navigationCenterStackedWidget->getContainerStackedWidget();
    if (_creatingPageKeys.contains(key))
    {
        return false;
    }
    _creatingPageKeys.insert(key);
    const auto factory = _pageFactories.value(key);
    QWidget* page = factory(stack);
    _creatingPageKeys.remove(key);
    if (!alive)
    {
        return false;
    }
    // 只接管新页面，不能把已经注册的页面或其他容器里的控件挪走。
    if (!_pageFactories.contains(key) || !page || page == q || page == stack || page->isAncestorOf(q) ||
        (page->parentWidget() && page->parentWidget() != stack) || !page->property("ElaPageKey").toString().isEmpty())
    {
        Q_EMIT q->navigationPageCreationFailed(key);
        return false;
    }
    QPointer<QWidget> previous = _routeMap.value(key);
    QPointer<QWidget> current = stack->currentWidget();
    const int index = previous ? stack->indexOf(previous) : stack->count();
    page->setProperty("ElaPageKey", key);
    _routeMap[key] = page;
    _pendingPageKeys.remove(key);
    _pageMetaMap[key] = page->metaObject();
    stack->insertWidget(index, page);
    if (current == previous)
    {
        stack->setCurrentWidget(page);
    }
    else if (current)
    {
        stack->setCurrentWidget(current);
    }
    if (previous)
    {
        stack->removeWidget(previous);
        previous->deleteLater();
    }
    Q_EMIT q->navigationPageCreated(key, page);
    return alive && _routeMap.value(key) == page;
}
