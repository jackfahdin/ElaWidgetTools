#include "ElaApplication.h"
#include "ElaLineEdit.h"
#include "ElaPushButton.h"
#include "ElaStateWidget.h"
#include "ElaTableView.h"
#include "T_DataView.h"
#include <QSignalSpy>
#include <QtTest>

class DataViewTest : public QObject
{
    Q_OBJECT
private:
    ElaPushButton* button(QWidget& page, const QString& label)
    {
        for (auto* candidate : page.findChildren<ElaPushButton*>())
        {
            if (candidate->text() == label)
                return candidate;
        }
        return nullptr;
    }

private Q_SLOTS:
    void initTestCase()
    {
        eApp->init();
    }

    void loadFilterErrorAndRetry()
    {
        T_DataView page;
        page.resize(900, 700);
        page.show();
        auto* state = page.findChild<ElaStateWidget*>();
        auto* table = page.findChild<ElaTableView*>();
        auto* filter = page.findChild<ElaLineEdit*>();
        QVERIFY(state && table && filter);
        QCOMPARE(state->getState(), ElaStateWidget::State::Loading);
        QTRY_COMPARE(state->getState(), ElaStateWidget::State::Content);
        QCOMPARE(table->model()->rowCount(), 6);
        filter->setText("不会匹配的关键字");
        QCOMPARE(state->getState(), ElaStateWidget::State::Empty);
        filter->clear();
        QCOMPARE(state->getState(), ElaStateWidget::State::Content);
        auto* fail = button(page, "模拟失败");
        auto* retry = button(page, "重试");
        QVERIFY(fail && retry);
        fail->click();
        filter->setText("界面");
        QCOMPARE(state->getState(), ElaStateWidget::State::Loading);
        QTRY_COMPARE(state->getState(), ElaStateWidget::State::Error);
        filter->setText("数据");
        QCOMPARE(state->getState(), ElaStateWidget::State::Error);
        retry->click();
        QTRY_COMPARE(state->getState(), ElaStateWidget::State::Content);
        QCOMPARE(table->model()->rowCount(), 2);
        QCOMPARE(filter->text(), QString("数据"));
        auto* empty = button(page, "加载空数据");
        QVERIFY(empty);
        empty->click();
        QTRY_COMPARE(state->getState(), ElaStateWidget::State::Empty);
        QCOMPARE(table->model()->rowCount(), 0);
    }

    void newerRequestWinsAndPageCanCloseDuringLoad()
    {
        T_DataView page;
        auto* state = page.findChild<ElaStateWidget*>();
        auto* fail = button(page, "模拟失败");
        auto* load = button(page, "加载数据");
        QVERIFY(state && fail && load);
        QSignalSpy changes(state, &ElaStateWidget::stateChanged);
        fail->click();
        load->click();
        QTRY_COMPARE(state->getState(), ElaStateWidget::State::Content);
        QCOMPARE(changes.count(), 1);
        QCOMPARE(changes.first().first().value<ElaStateWidget::State>(), ElaStateWidget::State::Content);
        auto* closing = new T_DataView;
        delete closing;
        // 让另一页面的定时请求完成，期间已销毁页面的回调也会到期。
        fail->click();
        QTRY_COMPARE(state->getState(), ElaStateWidget::State::Error);
    }
};

QTEST_MAIN(DataViewTest)
#include "tst_dataview.moc"
