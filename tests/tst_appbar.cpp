#include "ElaAppBar.h"
#include "ElaApplication.h"
#include "ElaIconButton.h"
#include "ElaMenuBar.h"
#include "ElaText.h"
#include "ElaToolButton.h"

#include <QSignalSpy>
#include <QToolButton>
#include <QVBoxLayout>
#include <QtTest>

class AppBarTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        eApp->init();
    }

    void customAreasRoundTrip()
    {
        QWidget window;
        ElaAppBar bar(&window);
        const auto areas = {ElaAppBarType::LeftArea, ElaAppBarType::MiddleArea, ElaAppBarType::RightArea};
        QSignalSpy changed(&bar, &ElaAppBar::customWidgetChanged);
        for (auto area : areas)
        {
            auto widget = new QWidget;
            bar.setCustomWidget(area, widget);
            QCOMPARE(bar.getCustomWidget(area), widget);
            QCOMPARE(widget->parentWidget(), &bar);
        }
        QCOMPARE(changed.count(), 3);
        QCOMPARE(bar.getCustomWidget(static_cast<ElaAppBarType::CustomArea>(-1)), nullptr);
        QCOMPARE(bar.getCustomWidget(static_cast<ElaAppBarType::CustomArea>(3)), nullptr);
    }

    void replacingAndRejectingCustomWidgets()
    {
        QWidget window;
        ElaAppBar bar(&window);
        auto first = new QWidget;
        auto second = new QWidget;
        bar.setCustomWidget(ElaAppBarType::LeftArea, first);
        bar.setCustomWidget(ElaAppBarType::LeftArea, second);
        QCOMPARE(bar.getCustomWidget(ElaAppBarType::LeftArea), second);
        QVERIFY(first->isHidden());
        QSignalSpy changed(&bar, &ElaAppBar::customWidgetChanged);
        auto middle = bar.getCustomWidget(ElaAppBarType::MiddleArea);
        bar.setCustomWidget(ElaAppBarType::MiddleArea, second);
        bar.setCustomWidget(ElaAppBarType::LeftArea, &window);
        bar.setCustomWidget(ElaAppBarType::LeftArea, &bar);
        bar.setCustomWidget(ElaAppBarType::LeftArea, nullptr);
        QCOMPARE(bar.getCustomWidget(ElaAppBarType::MiddleArea), middle);
        QCOMPARE(bar.getCustomWidget(ElaAppBarType::LeftArea), second);
        QCOMPARE(changed.count(), 0);
        bar.setAppBarHeight(60);
        QCOMPARE(second->maximumHeight(), 60);
    }

    void titleFollowsWindowAndRemainingSpace()
    {
        QWidget window;
        window.setWindowTitle("标题回归检查");
        ElaAppBar bar(&window);
        bar.setWindowButtonFlags(ElaAppBarType::CloseButtonHint);
        auto custom = new QWidget;
        custom->setFixedWidth(180);
        bar.setCustomWidget(ElaAppBarType::LeftArea, custom);
        window.resize(800, 300);
        window.show();
        QTRY_VERIFY(window.isVisible());
        auto title = bar.findChild<ElaText*>();
        QVERIFY(title);
        QTRY_COMPARE(title->text(), window.windowTitle());
        auto rightButtons = bar.layout()->itemAt(bar.layout()->count() - 1);
        QVERIFY(rightButtons);
        auto centered = [&]() {
            const int left = custom->geometry().right() + 1;
            const int right = rightButtons->geometry().left();
            return qAbs(title->geometry().center().x() - (left + right - 1) / 2) <= 1;
        };
        QTRY_VERIFY(centered());
        custom->setFixedWidth(260);
        window.resize(1000, 300);
        QTRY_VERIFY(centered());
        window.setWindowTitle("更新后的标题");
        QTRY_COMPARE(title->text(), window.windowTitle());
        QCOMPARE(title->toolTip(), window.windowTitle());
        window.setWindowTitle({});
        QTRY_VERIFY(title->isHidden());
    }

    void menuShrinksAndRestores()
    {
        QWidget window;
        ElaAppBar bar(&window);
        bar.setWindowButtonFlags(ElaAppBarType::CloseButtonHint);
        auto container = new QWidget;
        auto layout = new QVBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0);
        auto menu = new ElaMenuBar(container);
        // 这里是嵌入标题栏的控件，不能迁移至 macOS 的系统菜单栏。
        menu->setNativeMenuBar(false);
        menu->setFixedHeight(30);
        layout->addWidget(menu);
        layout->addStretch();
        for (int i = 0; i < 8; ++i)
        {
            menu->addMenu(QString("菜单项目 %1").arg(i));
        }
        bar.setCustomWidget(ElaAppBarType::LeftArea, container);
        const int fullWidth = container->sizeHint().width();
        QVERIFY(fullWidth > 0);
        window.resize(fullWidth + 400, 300);
        window.show();
        auto overflow = menu->findChild<QToolButton*>("qt_menubar_ext_button");
        QVERIFY(overflow);
        QTRY_VERIFY(!overflow->isVisible());
        QTRY_VERIFY(container->width() >= fullWidth);
        window.resize(container->minimumSizeHint().width() + 120, 300);
        QTRY_VERIFY(overflow->isVisible());
        QVERIFY(container->width() < fullWidth);
        window.resize(fullWidth + 400, 300);
        QTRY_VERIFY(!overflow->isVisible());
        QTRY_VERIFY(container->width() >= fullWidth);
    }

    void externalWindowStateUpdatesMaximizeIcon()
    {
        QWidget window;
        ElaAppBar bar(&window);
        ElaToolButton* maximize = nullptr;
        for (auto button : bar.findChildren<ElaToolButton*>())
        {
            if (button->property("ElaIconType").toChar() == QChar(static_cast<char16_t>(ElaIconType::Square)))
            {
                maximize = button;
                break;
            }
        }
        QVERIFY(maximize);
        // 模拟窗口系统发来的状态变化，不依赖虚拟显示器是否运行窗口管理器。
        window.setWindowState(Qt::WindowMaximized);
        QTRY_COMPARE(maximize->property("ElaIconType").toChar(),
                     QChar(static_cast<char16_t>(ElaIconType::WindowRestore)));
        window.setWindowState(Qt::WindowNoState);
        QTRY_COMPARE(maximize->property("ElaIconType").toChar(), QChar(static_cast<char16_t>(ElaIconType::Square)));
    }

    void closeCanBeIntercepted()
    {
        QWidget window;
        ElaAppBar bar(&window);
        bar.setIsDefaultClosed(false);
        QSignalSpy closed(&bar, &ElaAppBar::closeButtonClicked);
        window.show();
        auto close = bar.findChild<ElaIconButton*>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(close);
        close->click();
        QCOMPARE(closed.count(), 1);
        QVERIFY(window.isVisible());
    }

    void linuxStayTopIsIgnored()
    {
#ifdef Q_OS_LINUX
        QWidget window;
        ElaAppBar bar(&window);
        QSignalSpy changed(&bar, &ElaAppBar::pIsStayTopChanged);
        bar.setIsStayTop(true);
        QVERIFY(!bar.getIsStayTop());
        QVERIFY(!window.windowFlags().testFlag(Qt::WindowStaysOnTopHint));
        QCOMPARE(changed.count(), 0);
        bar.setWindowButtonFlags(ElaAppBarType::StayTopButtonHint | ElaAppBarType::CloseButtonHint);
        QCOMPARE(bar.getWindowButtonFlags(), ElaAppBarType::ButtonFlags(ElaAppBarType::CloseButtonHint));
        bar.setWindowButtonFlag(ElaAppBarType::StayTopButtonHint);
        QVERIFY(!bar.getWindowButtonFlags().testFlag(ElaAppBarType::StayTopButtonHint));
#else
        QSKIP("仅验证 Linux 已明确移除的置顶功能");
#endif
    }
};

QTEST_MAIN(AppBarTest)
#include "tst_appbar.moc"
