#ifndef ELAWORKSPACE_ELAWIDGETTOOLS_DEVELOPERCOMPONENTS_COMMAND_ELANAVIGATIONROUTECOMMAND_H_
#define ELAWORKSPACE_ELAWIDGETTOOLS_DEVELOPERCOMPONENTS_COMMAND_ELANAVIGATIONROUTECOMMAND_H_

#include "ElaActionCommander.h"
#include "ElaPropertyMacro.h"
#include <QPointer>
class ElaNavigationBar;
class ElaNavigationRouteCommand : public ElaActionCommand
{
    Q_OBJECT
    ELA_VARIABLE(QPointer<ElaNavigationBar>, NavigationBar);
    ELA_VARIABLE(QString, UndoPageKey);
    ELA_VARIABLE(QString, RedoPageKey);

public:
    explicit ElaNavigationRouteCommand(QObject* parent = nullptr);
    ~ElaNavigationRouteCommand() override;

    void undo() override;
    void redo() override;
    bool tryUndo() override;
    bool tryRedo() override;
};

#endif // ELAWORKSPACE_ELAWIDGETTOOLS_DEVELOPERCOMPONENTS_COMMAND_ELANAVIGATIONROUTECOMMAND_H_
