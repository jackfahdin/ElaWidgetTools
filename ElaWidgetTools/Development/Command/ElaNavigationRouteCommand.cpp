#include "ElaNavigationRouteCommand.h"

#include "ElaNavigationBar.h"

ElaNavigationRouteCommand::ElaNavigationRouteCommand(QObject* parent) : ElaActionCommand(parent)
{
}

ElaNavigationRouteCommand::~ElaNavigationRouteCommand()
{
}

void ElaNavigationRouteCommand::undo()
{
    tryUndo();
    setCommandDesc("撤销路由跳转操作");
}

void ElaNavigationRouteCommand::redo()
{
    tryRedo();
    setCommandDesc("执行路由跳转操作");
}
bool ElaNavigationRouteCommand::tryUndo()
{
    const QPointer<ElaNavigationRouteCommand> alive(this);
    const bool result = _pNavigationBar && _pNavigationBar->tryNavigation(_pUndoPageKey, false, true);
    if (alive && result)
        setCommandDesc("撤销路由跳转操作");
    return result;
}

bool ElaNavigationRouteCommand::tryRedo()
{
    const QPointer<ElaNavigationRouteCommand> alive(this);
    const bool result = _pNavigationBar && _pNavigationBar->tryNavigation(_pRedoPageKey, false, false);
    if (alive && result)
        setCommandDesc("执行路由跳转操作");
    return result;
}
