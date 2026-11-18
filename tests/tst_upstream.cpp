#include "ElaApplication.h"
#include "ElaComboBox.h"
#include "ElaMultiSelectComboBox.h"
#include "ElaSplitter.h"
#include "ElaTabBar.h"
#include "ElaTabWidget.h"
#include "ElaTeachingTip.h"

#include <QLabel>
#include <QPointer>
#include <QSignalSpy>
#include <QtTest>

// 覆盖同步上游后新增组件与重构容器的外部行为。
class UpstreamTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        eApp->init();
    }

    void splitterKeepsBothPanes()
    {
        ElaSplitter splitter(Qt::Horizontal);
        auto* first = new QLabel("左侧");
        auto* second = new QLabel("右侧");
        splitter.addWidget(first);
        splitter.addWidget(second);
        splitter.setChildrenCollapsible(false);
        splitter.resize(600, 200);
        splitter.show();
        QVERIFY(QTest::qWaitForWindowExposed(&splitter));
        splitter.setSizes({200, 400});
        QCOMPARE(splitter.count(), 2);
        QCOMPARE(splitter.widget(0), first);
        QCOMPARE(splitter.widget(1), second);
        QVERIFY(splitter.sizes().at(0) > 0);
        QVERIFY(splitter.sizes().at(1) > splitter.sizes().at(0));
    }

    void tabsPinTransferAndRemoveWithoutDestroyingPage()
    {
        ElaTabWidget source;
        ElaTabWidget destination;
        auto* first = new QLabel("首页");
        QPointer<QLabel> second = new QLabel("设置");
        source.addTab(first, "首页");
        source.addTab(second, "设置");
        source.setCurrentWidget(second);
        source.setIsTabPinned(source.indexOf(second), true);
        QVERIFY(source.getIsTabPinned(source.indexOf(second)));
        QCOMPARE(source.currentWidget(), second.data());
        destination.transferTab(&source, second, 0, false);
        QCOMPARE(source.count(), 1);
        QCOMPARE(source.widget(0), first);
        QCOMPARE(destination.count(), 1);
        QCOMPARE(destination.widget(0), second.data());
        QCOMPARE(destination.tabText(0), QString("设置"));
        QVERIFY(!destination.getIsTabPinned(0));
        destination.removeTab(0);
        QCOMPARE(destination.count(), 0);
        QVERIFY(second);
        delete second.data();
    }

    void tabsTrackExternalPageRemoval_data()
    {
        QTest::addColumn<bool>("pinned");
        QTest::addColumn<bool>("current");
        QTest::addColumn<bool>("reparent");
        for (bool pinned : {false, true})
            for (bool current : {false, true})
                for (bool reparent : {false, true})
                {
                    const QByteArray name = QByteArray(pinned ? "pinned" : "normal") +
                                            (current ? "-current" : "-other") + (reparent ? "-reparent" : "-delete");
                    QTest::newRow(name.constData()) << pinned << current << reparent;
                }
    }

    void tabsTrackExternalPageRemoval()
    {
        QFETCH(bool, pinned);
        QFETCH(bool, current);
        QFETCH(bool, reparent);
        QWidget otherParent;
        ElaTabWidget tabs;
        auto* first = new QLabel("首页");
        auto* second = new QLabel("设置");
        auto* fixed = new QLabel("固定");
        tabs.addTab(first, "首页");
        tabs.addTab(second, "设置");
        // 堆栈加入顺序与固定标签的显示顺序不同。
        tabs.insertTab(0, fixed, {}, "固定", true);
        auto* removed = pinned ? fixed : first;
        tabs.setCurrentWidget(current ? removed : second);
        QSignalSpy indexChanged(&tabs, &ElaTabWidget::currentChanged);
        QSignalSpy pageChanged(&tabs, &ElaTabWidget::currentWidgetChanged);
        if (reparent)
            removed->setParent(&otherParent);
        else
            delete removed;

        QCOMPARE(tabs.count(), 2);
        QCOMPARE(tabs.getTabBar(true)->count(), pinned ? 0 : 1);
        QCOMPARE(tabs.getTabBar()->count(), pinned ? 2 : 1);
        QCOMPARE(tabs.widget(0), pinned ? first : fixed);
        QCOMPARE(tabs.widget(1), second);
        QCOMPARE(tabs.tabText(0), pinned ? QString("首页") : QString("固定"));
        QCOMPARE(tabs.tabText(1), QString("设置"));
        auto* selected = current && pinned ? first : second;
        QCOMPARE(tabs.currentWidget(), selected);
        QCOMPARE(tabs.currentIndex(), selected == first ? 0 : 1);
        QCOMPARE(tabs.getTabBar()->currentIndex(), selected == first || !pinned ? 0 : 1);
        QCOMPARE(indexChanged.count(), 1);
        QCOMPARE(pageChanged.count(), current ? 1 : 0);
        if (reparent)
        {
            QCOMPARE(removed->parentWidget(), &otherParent);
            delete removed;
            QCOMPARE(tabs.count(), 2);
            QCOMPARE(indexChanged.count(), 1);
        }
        tabs.removeTab(0);
        QCOMPARE(tabs.count(), 1);
        QCOMPARE(tabs.widget(0), second);
    }

    void tabsCanBeReusedAfterExternalLastPageRemoval()
    {
        QWidget otherParent;
        ElaTabWidget tabs;
        for (bool pinned : {false, true})
            for (bool reparent : {false, true})
            {
                auto* page = new QLabel("最后一页");
                tabs.insertTab(0, page, {}, "最后一页", pinned);
                QSignalSpy indexChanged(&tabs, &ElaTabWidget::currentChanged);
                QSignalSpy pageChanged(&tabs, &ElaTabWidget::currentWidgetChanged);
                if (reparent)
                    page->setParent(&otherParent);
                else
                    delete page;
                QCOMPARE(tabs.count(), 0);
                QCOMPARE(tabs.getTabBar(true)->count(), 0);
                QCOMPARE(tabs.getTabBar()->count(), 0);
                QCOMPARE(tabs.currentIndex(), -1);
                QCOMPARE(tabs.currentWidget(), nullptr);
                QCOMPARE(indexChanged.count(), 1);
                QCOMPARE(indexChanged.first().first().toInt(), -1);
                QCOMPARE(pageChanged.count(), 1);
                QCOMPARE(qvariant_cast<QWidget*>(pageChanged.first().first()), nullptr);
                if (reparent)
                    delete page;
            }
    }

    void comboSelectionTracksModelChanges()
    {
        ElaComboBox single;
        single.addItems({"一", "二", "三"});
        single.setCurrentIndex(1);
        QCOMPARE(single.currentText(), QString("二"));
        single.removeItem(0);
        QCOMPARE(single.currentText(), QString("二"));

        ElaMultiSelectComboBox multiple;
        multiple.addItems({"一", "二", "三"});
        multiple.setCurrentSelection(QStringList{"一", "三"});
        QCOMPARE(multiple.getCurrentSelection(), (QStringList{"一", "三"}));
        multiple.clear();
        QVERIFY(multiple.getCurrentSelection().isEmpty());
        QVERIFY(multiple.getCurrentSelectionIndex().isEmpty());
    }

    void teachingTipOwnsContentAndTracksTargetLifetime()
    {
        ElaTeachingTip tip;
        QPointer<QLabel> oldContent = new QLabel("旧内容");
        auto* newContent = new QLabel("新内容");
        tip.setCustomWidget(oldContent);
        tip.setCustomWidget(newContent);
        QCOMPARE(tip.getCustomWidget(), newContent);
        QTRY_VERIFY(oldContent.isNull());
        auto* target = new QWidget;
        tip.setTargetWidget(target);
        QCOMPARE(tip.getTargetWidget(), target);
        delete target;
        QCOMPARE(tip.getTargetWidget(), nullptr);
        delete newContent;
        QCOMPARE(tip.getCustomWidget(), nullptr);
    }
};

QTEST_MAIN(UpstreamTest)
#include "tst_upstream.moc"
