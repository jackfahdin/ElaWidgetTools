#include "ElaApplication.h"
#include "ElaSuggestBox.h"
#include "ElaLineEdit.h"
#include <QListView>
#include <QVBoxLayout>
#include <QSignalSpy>
#include <QPointer>
#include <QtTest>

class SuggestTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        eApp->init();
    }
    void newestAsyncRequestWins()
    {
        ElaSuggestBox box;
        box.setAsyncSearchEnabled(true);
        box.setSearchDebounceInterval(0);
        QSignalSpy requested(&box, &ElaSuggestBox::searchRequested);
        QSignalSpy cancelled(&box, &ElaSuggestBox::searchCancelled);
        box.setSearchText("旧查询");
        QTRY_COMPARE(requested.count(), 1);
        const auto oldId = requested.at(0).at(0).toULongLong();
        box.setSearchText("新查询");
        QTRY_COMPARE(requested.count(), 2);
        QCOMPARE(cancelled.count(), 1);
        const auto newId = requested.at(1).at(0).toULongLong();
        QVERIFY(newId > oldId);
        QVERIFY(!box.setSearchResults(oldId, {ElaSuggestBox::SuggestData(ElaIconType::None, "旧结果")}));
        QVERIFY(box.setSearchResults(newId, {ElaSuggestBox::SuggestData(ElaIconType::None, "新结果")}));
        QVERIFY(!box.setSearchResults(newId, {}));
        auto* view = box.window()->findChild<QListView*>();
        QVERIFY(view);
        QCOMPARE(view->model()->rowCount(), 1);
        QCOMPARE(view->model()->index(0, 0).data().toString(), QString("新结果"));
    }
    void debounceAndClearCancelPendingWork()
    {
        ElaSuggestBox box;
        box.setAsyncSearchEnabled(true);
        box.setSearchDebounceInterval(20);
        QSignalSpy requests(&box, &ElaSuggestBox::searchRequested);
        box.setSearchText("a");
        box.setSearchText("ab");
        box.setSearchText("abc");
        QTRY_COMPARE(requests.count(), 1);
        QCOMPARE(requests.first().at(1).toString(), QString("abc"));
        const auto id = requests.first().first().toULongLong();
        box.setSearchText("");
        QVERIFY(!box.getIsSearchPending());
        QVERIFY(!box.setSearchResults(id, {ElaSuggestBox::SuggestData(ElaIconType::None, "迟到的结果")}));
        box.setSearchText("d");
        box.cancelSearch();
        QVERIFY(!box.getIsSearchPending());
    }
    void keyboardSelectsLocalResults()
    {
        // 搜索框按实际用法嵌入窗口，给弹层预留空间。
        QWidget window;
        auto* layout = new QVBoxLayout(&window);
        auto* search = new ElaSuggestBox(&window);
        layout->addWidget(search);
        layout->addStretch();
        window.resize(420, 320);
        auto& box = *search;
        box.addSuggestion("alpha");
        const auto betaKey = box.addSuggestion("beta");
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        // 直接测试 Qt 键盘路由，不依赖 macOS CI 是否允许进程抢占系统前台。
        QApplication::setActiveWindow(&window);
        auto* edit = box.findChild<ElaLineEdit*>();
        QVERIFY(edit);
        edit->setFocus();
        QTRY_VERIFY(edit->hasFocus());
        box.setSearchText("a");
        QSignalSpy clicked(&box, &ElaSuggestBox::suggestionClicked);
        QTest::keyClick(edit, Qt::Key_Down);
        QTest::keyClick(edit, Qt::Key_Down);
        auto* view = box.window()->findChild<QListView*>();
        QVERIFY(view->selectionModel()->isSelected(view->currentIndex()));
        QTest::keyClick(edit, Qt::Key_Return);
        QCOMPARE(clicked.count(), 1);
        QCOMPARE(box.getSearchText(), QString("beta"));
        QCOMPARE(qvariant_cast<ElaSuggestBox::SuggestData>(clicked.first().first()).getSuggestKey(), betaKey);
    }

    void escapeInvalidatesAsyncRequest()
    {
        // 请求取消与本地弹层选择分开验证，避免原生窗口焦点事件干扰防抖计时器。
        ElaSuggestBox box;
        auto* edit = box.findChild<ElaLineEdit*>();
        QVERIFY(edit);
        box.setAsyncSearchEnabled(true);
        box.setSearchDebounceInterval(0);
        QSignalSpy requests(&box, &ElaSuggestBox::searchRequested);
        box.setSearchText("cancel");
        QTRY_COMPARE(requests.count(), 1);
        QTest::keyClick(edit, Qt::Key_Escape);
        QVERIFY(!box.setSearchResults(requests.first().first().toULongLong(), {}));
    }
    void cancellationCallbackCanClearNewInput()
    {
        ElaSuggestBox box;
        box.setAsyncSearchEnabled(true);
        box.setSearchDebounceInterval(0);
        QSignalSpy requests(&box, &ElaSuggestBox::searchRequested);
        box.setSearchText("old");
        QTRY_COMPARE(requests.count(), 1);
        QSignalSpy pending(&box, &ElaSuggestBox::searchPendingChanged);
        connect(&box, &ElaSuggestBox::searchCancelled, &box, [&]() {
            box.setSearchText("");
        });
        box.setSearchText("new");
        QCOMPARE(box.getSearchText(), QString());
        QVERIFY(!box.getIsSearchPending());
        QCOMPARE(pending.count(), 1);
        QCOMPARE(pending.first().first().toBool(), false);
    }

    void hidingOrDisablingAsyncModeInvalidatesResults()
    {
        ElaSuggestBox box;
        box.show();
        box.setAsyncSearchEnabled(true);
        box.setSearchDebounceInterval(0);
        QSignalSpy requests(&box, &ElaSuggestBox::searchRequested);
        box.setSearchText("hide");
        QTRY_COMPARE(requests.count(), 1);
        box.hide();
        QVERIFY(!box.setSearchResults(requests.first().first().toULongLong(), {}));
        box.setSearchText("disable");
        QTRY_COMPARE(requests.count(), 2);
        box.setAsyncSearchEnabled(false);
        QVERIFY(!box.setSearchResults(requests.last().first().toULongLong(), {}));
        QVERIFY(!box.getIsSearchPending());
    }

    void cancellationCallbackMayDestroySearchBox()
    {
        QPointer<ElaSuggestBox> box = new ElaSuggestBox;
        box->setAsyncSearchEnabled(true);
        box->setSearchDebounceInterval(0);
        QSignalSpy requests(box, &ElaSuggestBox::searchRequested);
        box->setSearchText("old");
        QTRY_COMPARE(requests.count(), 1);
        connect(box, &ElaSuggestBox::searchPendingChanged, this, [&box](bool pending) {
            if (!pending)
                delete box.data();
        });
        box->cancelSearch();
        QVERIFY(!box);
    }

    void disablingAsyncModeSurvivesCancellationTextChange()
    {
        ElaSuggestBox box;
        box.setAsyncSearchEnabled(true);
        box.setSearchDebounceInterval(0);
        QSignalSpy requests(&box, &ElaSuggestBox::searchRequested);
        box.setSearchText("old");
        QTRY_COMPARE(requests.count(), 1);
        connect(&box, &ElaSuggestBox::searchCancelled, &box, [&]() {
            box.setSearchText("");
        });
        box.setAsyncSearchEnabled(false);
        QVERIFY(!box.getAsyncSearchEnabled());
        QVERIFY(!box.getIsSearchPending());
    }

    void focusedSearchBoxCanBeDestroyed()
    {
        QPointer<ElaSuggestBox> box = new ElaSuggestBox;
        box->addSuggestion("alpha");
        box->show();
        QApplication::setActiveWindow(box);
        auto* edit = box->findChild<ElaLineEdit*>();
        edit->setFocus(Qt::OtherFocusReason);
        QTRY_VERIFY(edit->hasFocus());
        box->setSearchText("a");
        delete box.data();
        QVERIFY(!box);
    }

    void deletingVisibleSuggestionsClearsModel()
    {
        ElaSuggestBox box;
        box.addSuggestion("alpha");
        box.setSearchText("a");
        auto* view = box.window()->findChild<QListView*>();
        QVERIFY(view);
        QCOMPARE(view->model()->rowCount(), 1);
        box.removeSuggestion(-1);
        QCOMPARE(view->model()->rowCount(), 1);
        box.clearSuggestion();
        QCOMPARE(view->model()->rowCount(), 0);
    }
};
QTEST_MAIN(SuggestTest)
#include "tst_suggest.moc"
