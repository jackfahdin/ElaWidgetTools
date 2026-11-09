#include "ElaApplication.h"
#include "ElaProgressRing.h"
#include "ElaPushButton.h"
#include "ElaStateWidget.h"
#include "ElaTableView.h"
#include "ElaText.h"

#include <QLineEdit>
#include <functional>
#include <QPointer>
#include <QSignalSpy>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QtTest>

using State = ElaStateWidget::State;

class HideCallbackWidget : public QWidget
{
public:
    std::function<void()> onHide;

protected:
    void hideEvent(QHideEvent* event) override
    {
        QWidget::hideEvent(event);
        if (onHide)
            onHide();
    }
};

class StateWidgetTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        eApp->init();
    }

    void statesPreserveContentAndNotifyOnce()
    {
        ElaStateWidget container;
        auto* edit = new QLineEdit;
        edit->setText("保留编辑内容");
        QVERIFY(container.setContentWidget(edit));
        QCOMPARE(container.getState(), State::Empty);
        container.resize(480, 300);
        container.show();
        QVERIFY(!edit->isVisible());
        QSignalSpy changed(&container, &ElaStateWidget::stateChanged);
        for (State state : {State::Content, State::Loading, State::Empty, State::Error, State::Content})
        {
            container.setState(state);
            QCOMPARE(edit->isVisible(), state == State::Content);
            QCOMPARE(container.getContentWidget(), edit);
            QCOMPARE(edit->text(), QString("保留编辑内容"));
        }
        QCOMPARE(changed.count(), 5);
        container.setState(State::Content);
        container.setState(static_cast<State>(99));
        QCOMPARE(changed.count(), 5);
        QCOMPARE(container.getState(), State::Content);
    }

    void replacementAndTakeHaveExplicitOwnership()
    {
        auto* container = new ElaStateWidget;
        QPointer<QWidget> first = new QWidget;
        QVERIFY(container->setContentWidget(first));
        QVERIFY(container->setContentWidget(first));
        QPointer<QWidget> second = new QWidget(container);
        QVERIFY(container->setContentWidget(second));
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QVERIFY(first.isNull());
        container->setState(State::Content);
        QWidget* taken = container->takeContentWidget();
        QCOMPARE(taken, second.data());
        QVERIFY(!taken->parentWidget());
        QVERIFY(taken->isHidden());
        QVERIFY(!container->getContentWidget());
        QCOMPARE(container->getState(), State::Empty);
        delete container;
        QVERIFY(second);
        delete taken;
        QVERIFY(second.isNull());
    }

    void rejectsForeignAndInternalWidgets()
    {
        QWidget root;
        auto* container = new ElaStateWidget(&root);
        auto* content = new QWidget;
        QVERIFY(container->setContentWidget(content));
        QWidget foreignParent;
        auto* foreign = new QWidget(&foreignParent);
        QVERIFY(!container->setContentWidget(nullptr));
        QVERIFY(!container->setContentWidget(container));
        QVERIFY(!container->setContentWidget(&root));
        QVERIFY(!container->setContentWidget(foreign));
        for (auto* child : container->findChildren<QWidget*>())
        {
            if (child != content)
                QVERIFY(!container->setContentWidget(child));
        }
        QCOMPARE(container->getContentWidget(), content);
        QCOMPARE(foreign->parentWidget(), &foreignParent);
    }

    void deletingContentClearsPointerAndShowsEmpty()
    {
        ElaStateWidget container;
        auto* content = new QWidget;
        QVERIFY(container.setContentWidget(content));
        container.setState(State::Content);
        delete content;
        QVERIFY(!container.getContentWidget());
        QCOMPARE(container.getState(), State::Empty);
        auto* replacement = new QWidget;
        QVERIFY(container.setContentWidget(replacement));
        container.setState(State::Loading);
        delete replacement;
        QCOMPARE(container.getState(), State::Loading);
    }

    void retryIsKeyboardAccessibleAndApplicationControlled()
    {
        ElaStateWidget container;
        container.resize(420, 300);
        container.setState(State::Error);
        container.show();
        QVERIFY(QTest::qWaitForWindowExposed(&container));
        auto* retry = container.findChild<ElaPushButton*>();
        QVERIFY(retry);
        QVERIFY(retry->isVisible());
        QVERIFY(retry->focusPolicy() & Qt::TabFocus);
        QSignalSpy requested(&container, &ElaStateWidget::retryRequested);
        retry->setFocus();
        QTest::keyClick(retry, Qt::Key_Space);
        QCOMPARE(requested.count(), 1);
        QCOMPARE(container.getState(), State::Error);
        container.setRetryEnabled(false);
        QVERIFY(!retry->isVisible());
        container.setRetryEnabled(true);
        QVERIFY(retry->isVisible());
        container.setState(State::Empty);
        QVERIFY(!retry->isVisible());
    }

    void loadingAnimationRunsOnlyWhileVisible()
    {
        QWidget parent;
        ElaStateWidget container(&parent);
        auto* ring = container.findChild<ElaProgressRing*>();
        QVERIFY(ring);
        container.setState(State::Loading);
        QVERIFY(!ring->getIsBusying());
        parent.show();
        QVERIFY(ring->getIsBusying());
        parent.hide();
        QVERIFY(!ring->getIsBusying());
        parent.show();
        QVERIFY(ring->getIsBusying());
        container.setState(State::Error);
        QVERIFY(!ring->getIsBusying());
    }

    void statusTextIsPlainAndUpdatesWithoutChangingState()
    {
        ElaStateWidget container;
        container.setState(State::Error);
        QSignalSpy changed(&container, &ElaStateWidget::stateChanged);
        container.setStateText(State::Error, "<b>加载失败</b>", "请重试\n不会自动发起请求");
        container.resize(260, 300);
        container.show();
        bool foundTitle = false;
        bool foundDescription = false;
        for (auto* text : container.findChildren<ElaText*>())
        {
            if (text->text() == "<b>加载失败</b>")
            {
                foundTitle = text->isVisible();
                QCOMPARE(text->textFormat(), Qt::PlainText);
                QVERIFY(text->wordWrap());
            }
            if (text->text() == "请重试\n不会自动发起请求")
                foundDescription = text->isVisible();
        }
        QVERIFY(foundTitle);
        QVERIFY(foundDescription);
        QCOMPARE(changed.count(), 0);
    }

    void stateNotificationMayReenterOrDestroyContainer()
    {
        QPointer<ElaStateWidget> container = new ElaStateWidget;
        connect(container, &ElaStateWidget::stateChanged, this, [container](State state) {
            if (state == State::Loading)
                container->setState(State::Error);
        });
        container->setState(State::Loading);
        QCOMPARE(container->getState(), State::Error);
        connect(container, &ElaStateWidget::stateChanged, this, [container](State) {
            delete container.data();
        });
        container->setState(State::Empty);
        QVERIFY(!container);
    }

    void retryCallbackMayDestroyContainer()
    {
        QPointer<ElaStateWidget> container = new ElaStateWidget;
        container->setState(State::Error);
        auto* retry = container->findChild<ElaPushButton*>();
        QVERIFY(retry);
        connect(container, &ElaStateWidget::retryRequested, this, [container] {
            delete container.data();
        });
        retry->click();
        QVERIFY(!container);
    }

    void replacementMayBeDestroyedByOldContentHide()
    {
        ElaStateWidget container;
        auto* old = new HideCallbackWidget;
        container.setContentWidget(old);
        container.setState(State::Content);
        container.show();
        QPointer<QWidget> incoming = new QWidget;
        old->onHide = [&] {
            delete incoming.data();
        };
        QVERIFY(!container.setContentWidget(incoming));
        QVERIFY(!container.getContentWidget());
        QCOMPARE(container.getState(), State::Empty);
        old->onHide = {};
    }

    void focusChangeMayDeleteContentDuringTransition()
    {
        ElaStateWidget container;
        QPointer<QWidget> content = new QWidget;
        container.setContentWidget(content);
        container.setState(State::Error);
        container.show();
        QVERIFY(QTest::qWaitForWindowExposed(&container));
        auto* retry = container.findChild<ElaPushButton*>();
        QVERIFY(retry);
        // 直接分发焦点变化通知，避免平台窗口激活策略影响销毁回调覆盖。
        auto connection = connect(qApp, &QApplication::focusChanged, &container, [&](QWidget* old, QWidget*) {
            if (old == retry)
                delete content.data();
        });
        class FocusNotifier : public QObject
        {
        public:
            ElaPushButton* retry{nullptr};

        protected:
            bool eventFilter(QObject* watched, QEvent* event) override
            {
                if (event->type() == QEvent::Hide)
                    Q_EMIT qApp->focusChanged(retry, nullptr);
                return QObject::eventFilter(watched, event);
            }
        } notifier;
        notifier.retry = retry;
        retry->installEventFilter(&notifier);
        container.setState(State::Content);
        QVERIFY(!container.getContentWidget());
        QCOMPARE(container.getState(), State::Empty);
        disconnect(connection);
    }

    void tableFilteringAndSortingComposeWithState()
    {
        ElaStateWidget container;
        auto* table = new ElaTableView;
        QStandardItemModel model;
        model.appendRow(new QStandardItem("Bravo"));
        model.appendRow(new QStandardItem("Alpha"));
        QSortFilterProxyModel proxy;
        proxy.setSourceModel(&model);
        table->setModel(&proxy);
        table->setSortingEnabled(true);
        table->sortByColumn(0, Qt::AscendingOrder);
        QVERIFY(container.setContentWidget(table));
        auto updateState = [&] {
            container.setState(proxy.rowCount() ? State::Content : State::Empty);
        };
        container.show();
        updateState();
        QCOMPARE(proxy.index(0, 0).data().toString(), QString("Alpha"));
        table->selectRow(0);
        container.setState(State::Loading);
        container.setState(State::Content);
        QCOMPARE(table->selectionModel()->selectedRows().count(), 1);
        proxy.setFilterFixedString("missing");
        updateState();
        QCOMPARE(container.getState(), State::Empty);
        QVERIFY(!table->isVisible());
        proxy.setFilterFixedString("");
        updateState();
        QVERIFY(table->isVisible());
        QCOMPARE(proxy.rowCount(), 2);
        QCOMPARE(table->model(), &proxy);
    }
};

QTEST_MAIN(StateWidgetTest)
#include "tst_statewidget.moc"
