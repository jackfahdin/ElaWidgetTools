#include "ElaActionCommander.h"
#include "ElaApplication.h"
#include "ElaWindow.h"

#include <QSignalSpy>
#include <QtTest>

class IncrementCommand : public ElaActionCommand
{
public:
    explicit IncrementCommand(int& value) : _value(value)
    {
        setIsShowMessage(false);
    }
    void undo() override
    {
        --_value;
    }
    void redo() override
    {
        ++_value;
    }

private:
    int& _value;
};

class NavigationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        eApp->init();
        ElaActionCommander::getInstance()->setIsMessageDisplayEnable(false);
    }

    void pagesSwitchAndInvalidKeyDoesNotNavigate()
    {
        ElaWindow window;
        window.setStackSwitchMode(ElaWindowType::None);
        auto first = new QWidget;
        auto second = new QWidget;
        QCOMPARE(window.addPageNode("首页", first), ElaNavigationType::Success);
        QCOMPARE(window.addPageNode("设置", second), ElaNavigationType::Success);
        const auto firstKey = first->property("ElaPageKey").toString();
        const auto secondKey = second->property("ElaPageKey").toString();
        QVERIFY(!firstKey.isEmpty());
        QVERIFY(firstKey != secondKey);
        window.navigation(secondKey);
        QTRY_COMPARE(window.getCurrentNavigationPageKey(), secondKey);
        window.navigation("不存在的页面");
        QCOMPARE(window.getCurrentNavigationPageKey(), secondKey);
        window.navigation(firstKey);
        QTRY_COMPARE(window.getCurrentNavigationPageKey(), firstKey);
        window.removeNavigationNode(secondKey);
        window.navigation(secondKey);
        QCOMPARE(window.getCurrentNavigationPageKey(), firstKey);
        ElaActionCommander::getInstance()->clearCommand("ElaWidgetToolsAction");
    }

    void commandHistoryUndoesRedoesAndDropsFuture()
    {
        const QString domain = "QtTest.CommandHistory";
        auto commander = ElaActionCommander::getInstance();
        commander->clearCommand(domain);
        int value = 0;
        QSignalSpy state(commander, &ElaActionCommander::commanderStateChanged);
        QVERIFY(commander->recordCommand(domain, new IncrementCommand(value)));
        QVERIFY(commander->recordCommand(domain, new IncrementCommand(value)));
        QCOMPARE(value, 2);
        commander->undoCommand(domain);
        QCOMPARE(value, 1);
        QCOMPARE(commander->getCommanderRedoState(domain), ElaActionCommanderType::RedoValid);
        commander->redoCommand(domain);
        QCOMPARE(value, 2);
        commander->undoCommand(domain);
        QVERIFY(commander->recordCommand(domain, new IncrementCommand(value)));
        QCOMPARE(value, 2);
        QCOMPARE(commander->getCommanderRedoState(domain), ElaActionCommanderType::RedoInvalid);
        commander->redoCommand(domain);
        QCOMPARE(value, 2);
        QVERIFY(!state.isEmpty());
        commander->clearCommand(domain);
        QCOMPARE(commander->getCommanderUndoState(domain), ElaActionCommanderType::UndoInvalid);
    }
};

QTEST_MAIN(NavigationTest)
#include "tst_navigation.moc"
