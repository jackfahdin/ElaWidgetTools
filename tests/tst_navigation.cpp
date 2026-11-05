#include "ElaActionCommander.h"
#include "ElaApplication.h"
#include "ElaWindow.h"

#include <QSignalSpy>
#include <QPointer>
#include <QAbstractItemView>
#include <QStackedWidget>
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

    void emptyWindowHasNoCurrentPage()
    {
        ElaWindow window;
        QVERIFY(window.getCurrentNavigationPageKey().isEmpty());
    }

    void lazyPagesCreateOnceAndGuardBeforeCreation()
    {
        ElaWindow window;
        window.setStackSwitchMode(ElaWindowType::None);
        auto* home = new QWidget;
        window.addPageNode("首页", home);
        const QString homeKey = home->property("ElaPageKey").toString();
        QString lazyKey;
        int created = 0;
        QCOMPARE(window.addLazyPageNode("编辑器", lazyKey,
                                        [&](QWidget* parent) {
                                            ++created;
                                            return new QWidget(parent);
                                        }),
                 ElaNavigationType::Success);
        QCOMPARE(created, 0);
        QVERIFY(!window.getNavigationPage(lazyKey));
        window.setNavigationGuard([](const QString&, const QString&) {
            return false;
        });
        window.navigation(lazyKey);
        QCOMPARE(created, 0);
        QCOMPARE(window.getCurrentNavigationPageKey(), homeKey);
        window.setNavigationGuard({});
        QSignalSpy ready(&window, &ElaWindow::navigationPageCreated);
        window.navigation(lazyKey);
        QCOMPARE(created, 1);
        QCOMPARE(ready.count(), 1);
        QCOMPARE(window.getCurrentNavigationPageKey(), lazyKey);
        auto* page = window.getNavigationPage(lazyKey);
        QVERIFY(page);
        window.navigation(homeKey);
        window.navigation(lazyKey);
        QCOMPARE(created, 1);
        QCOMPARE(window.getNavigationPage(lazyKey), page);
        window.removeNavigationNode(lazyKey);
        QVERIFY(!window.getNavigationPage(lazyKey));
        window.navigation(lazyKey);
        QCOMPARE(created, 1);
    }

    void lazyCreationFailureCanRetry()
    {
        ElaWindow window;
        window.setStackSwitchMode(ElaWindowType::None);
        auto* home = new QWidget;
        window.addPageNode("首页", home);
        QString key;
        int attempts = 0;
        window.addLazyPageNode("延迟页", key, [&](QWidget* parent) -> QWidget* {
            return ++attempts == 1 ? nullptr : new QWidget(parent);
        });
        QSignalSpy failed(&window, &ElaWindow::navigationPageCreationFailed);
        window.navigation(key);
        QCOMPARE(failed.count(), 1);
        QCOMPARE(window.getCurrentNavigationPageKey(), home->property("ElaPageKey").toString());
        window.navigation(key);
        QCOMPARE(attempts, 2);
        QCOMPARE(window.getCurrentNavigationPageKey(), key);
    }

    void rejectedHistoryDoesNotMoveCursor()
    {
        auto* commander = ElaActionCommander::getInstance();
        commander->clearCommand("ElaWidgetToolsAction");
        ElaWindow window;
        window.setStackSwitchMode(ElaWindowType::None);
        auto* home = new QWidget;
        auto* editor = new QWidget;
        window.addPageNode("首页", home);
        window.addPageNode("编辑器", editor);
        const auto homeKey = home->property("ElaPageKey").toString();
        const auto editorKey = editor->property("ElaPageKey").toString();
        window.navigation(editorKey);
        bool allow = false;
        window.setNavigationGuard([&](const QString&, const QString&) {
            return allow;
        });
        commander->undoCommand("ElaWidgetToolsAction");
        QCOMPARE(window.getCurrentNavigationPageKey(), editorKey);
        QCOMPARE(commander->getCommanderRedoState("ElaWidgetToolsAction"), ElaActionCommanderType::RedoInvalid);
        allow = true;
        commander->undoCommand("ElaWidgetToolsAction");
        QCOMPARE(window.getCurrentNavigationPageKey(), homeKey);
        allow = false;
        commander->redoCommand("ElaWidgetToolsAction");
        QCOMPARE(window.getCurrentNavigationPageKey(), homeKey);
        allow = true;
        commander->redoCommand("ElaWidgetToolsAction");
        QCOMPARE(window.getCurrentNavigationPageKey(), editorKey);
        commander->clearCommand("ElaWidgetToolsAction");
    }

    void newFirstLazyPageLoadsAfterLastPageRemoval()
    {
        ElaWindow window;
        auto* initial = new QWidget;
        window.addPageNode("首页", initial);
        window.removeNavigationNode(initial->property("ElaPageKey").toString());
        QString key;
        int created = 0;
        window.addLazyPageNode("新首页", key, [&](QWidget* parent) {
            ++created;
            return new QWidget(parent);
        });
        window.show();
        QCOMPARE(created, 1);
        QCOMPARE(window.getCurrentNavigationPageKey(), key);
    }

    void historyMutationInsideGuardCannotDeleteExecutingCommand()
    {
        auto* commander = ElaActionCommander::getInstance();
        commander->clearCommand("ElaWidgetToolsAction");
        ElaWindow window;
        window.setStackSwitchMode(ElaWindowType::None);
        auto* first = new QWidget;
        auto* second = new QWidget;
        window.addPageNode("首页", first);
        window.addPageNode("编辑器", second);
        window.navigation(second->property("ElaPageKey").toString());
        window.setNavigationGuard([=](const QString&, const QString&) {
            commander->clearCommand("ElaWidgetToolsAction");
            return true;
        });
        commander->undoCommand("ElaWidgetToolsAction");
        QCOMPARE(window.getCurrentNavigationPageKey(), first->property("ElaPageKey").toString());
        QCOMPARE(commander->getCommanderRedoState("ElaWidgetToolsAction"), ElaActionCommanderType::RedoValid);
        commander->redoCommand("ElaWidgetToolsAction");
        QCOMPARE(window.getCurrentNavigationPageKey(), second->property("ElaPageKey").toString());
        commander->clearCommand("ElaWidgetToolsAction");
    }

    void newWindowFactoryCannotStealRegisteredPage()
    {
        ElaWindow window;
        QString key;
        QWidget* cached = nullptr;
        window.addLazyPageNode("缓存工厂", key, [&](QWidget* parent) {
            if (!cached)
                cached = new QWidget(parent);
            return cached;
        });
        window.navigation(key);
        auto* stack = qobject_cast<QStackedWidget*>(cached->parentWidget());
        QVERIFY(stack);
        QSignalSpy opened(&window, &ElaWindow::pageOpenInNewWindow);
        bool invoked = false;
        for (auto* view : window.findChildren<QAbstractItemView*>())
        {
            if (view->metaObject()->indexOfSignal("navigationOpenNewWindow(QString)") >= 0)
            {
                invoked = QMetaObject::invokeMethod(view, "navigationOpenNewWindow", Q_ARG(QString, key));
                break;
            }
        }
        QVERIFY(invoked);
        QVERIFY(stack->indexOf(cached) >= 0);
        QCOMPARE(opened.count(), 0);
    }

    void newWindowFactoryMayDestroyOwner()
    {
        QPointer<ElaWindow> window = new ElaWindow;
        QString key;
        window->addLazyPageNode("销毁测试", key, [&](QWidget* parent) {
            auto* page = new QWidget(parent);
            delete window.data();
            return page;
        });
        QAbstractItemView* navigationView = nullptr;
        for (auto* view : window->findChildren<QAbstractItemView*>())
        {
            if (view->metaObject()->indexOfSignal("navigationOpenNewWindow(QString)") >= 0)
            {
                navigationView = view;
                break;
            }
        }
        QVERIFY(navigationView);
        QVERIFY(QMetaObject::invokeMethod(navigationView, "navigationOpenNewWindow", Q_ARG(QString, key)));
        QVERIFY(!window);
    }

    void groupedLazyPageAndFooterRespectGuard()
    {
        ElaWindow window;
        window.setStackSwitchMode(ElaWindowType::None);
        auto* home = new QWidget;
        window.addPageNode("首页", home);
        QString groupKey, pageKey, footerKey;
        window.addExpanderNode("分组", groupKey);
        window.addLazyPageNode(
            "延迟页", pageKey,
            [](QWidget* parent) {
                return new QWidget(parent);
            },
            groupKey);
        window.addFooterNode("页脚页", new QWidget, footerKey);
        window.navigation(pageKey);
        QCOMPARE(window.getCurrentNavigationPageKey(), pageKey);
        window.setNavigationGuard([](const QString&, const QString&) {
            return false;
        });
        window.navigation(footerKey);
        QCOMPARE(window.getCurrentNavigationPageKey(), pageKey);
        window.setNavigationGuard({});
        window.navigation(footerKey);
        QCOMPARE(window.getCurrentNavigationPageKey(), footerKey);
        window.removeNavigationNode(groupKey);
        QVERIFY(!window.getNavigationPage(pageKey));
    }

    void guardCannotReenterNavigation()
    {
        ElaWindow window;
        window.setStackSwitchMode(ElaWindowType::None);
        auto* home = new QWidget;
        auto* target = new QWidget;
        window.addPageNode("首页", home);
        window.addPageNode("目标页", target);
        const QString targetKey = target->property("ElaPageKey").toString();
        int attempts = 0;
        window.setNavigationGuard([&](const QString&, const QString&) {
            ++attempts;
            window.navigation(targetKey);
            return false;
        });
        window.navigation(targetKey);
        QCOMPARE(attempts, 1);
        QCOMPARE(window.getCurrentNavigationPageKey(), home->property("ElaPageKey").toString());
    }

    void historyStateSignalsCannotReenterExecution()
    {
        const QString domain = "QtTest.ReentrantHistory";
        auto* commander = ElaActionCommander::getInstance();
        commander->clearCommand(domain);
        int value = 0;
        commander->recordCommand(domain, new IncrementCommand(value));
        auto connection = connect(commander, &ElaActionCommander::commanderStateChanged, this,
                                  [=](const QString& changedDomain, ElaActionCommanderType::CommanderState state) {
                                      if (changedDomain == domain && state == ElaActionCommanderType::RedoValid)
                                          commander->redoCommand(domain);
                                  });
        commander->undoCommand(domain);
        QCOMPARE(value, 0);
        commander->redoCommand(domain);
        QCOMPARE(value, 1);
        disconnect(connection);
        commander->clearCommand(domain);
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
