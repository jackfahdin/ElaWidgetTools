#include "ElaApplication.h"
#include "ElaTheme.h"
#include "ElaPushButton.h"
#include "ElaIconButton.h"
#include "ElaText.h"
#include "ElaMenu.h"
#include "ElaToggleSwitch.h"
#include "ElaPopularCard.h"
#include "ElaDockWidget.h"
#include "ElaScrollPageArea.h"
#include <QSignalSpy>
#include <QPropertyAnimation>
#include <QStyleHints>
#include <QtTest>

class PaintObserver : public QObject
{
public:
    int count{0};
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (event->type() == QEvent::Paint)
        {
            ++count;
        }
        return QObject::eventFilter(watched, event);
    }
};

class ThemeTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        eApp->init();
    }
    void cleanup()
    {
        eTheme->setThemeMode(ElaThemeType::Light);
        eTheme->resetThemeColors();
    }
    void systemThemeAndManualOverride()
    {
        QSignalSpy policy(eTheme, &ElaTheme::followSystemThemeChanged);
        eTheme->setFollowSystemTheme(true);
        QCOMPARE(eTheme->getFollowSystemTheme(), true);
        QCOMPARE(eTheme->getThemeMode(),
                 qApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark ? ElaThemeType::Dark : ElaThemeType::Light);
        eTheme->setFollowSystemTheme(true);
        QCOMPARE(policy.count(), 1);
        // 模拟 Qt 平台通知，不依赖 CI 桌面是否支持请求系统配色。
        auto notify = [](Qt::ColorScheme scheme) {
            return QMetaObject::invokeMethod(qApp->styleHints(), "colorSchemeChanged", Qt::DirectConnection,
                                             Q_ARG(Qt::ColorScheme, scheme));
        };
        QVERIFY(notify(Qt::ColorScheme::Dark));
        QCOMPARE(eTheme->getThemeMode(), ElaThemeType::Dark);
        QVERIFY(notify(Qt::ColorScheme::Light));
        QCOMPARE(eTheme->getThemeMode(), ElaThemeType::Light);
        QVERIFY(notify(Qt::ColorScheme::Dark));
        QVERIFY(notify(Qt::ColorScheme::Unknown));
        QCOMPARE(eTheme->getThemeMode(), ElaThemeType::Light);
        eTheme->setThemeMode(ElaThemeType::Light);
        QVERIFY(!eTheme->getFollowSystemTheme());
        QCOMPARE(policy.count(), 2);
        QVERIFY(notify(Qt::ColorScheme::Dark));
        QCOMPARE(eTheme->getThemeMode(), ElaThemeType::Light);
        eTheme->setFollowSystemTheme(true);
        QVERIFY(notify(Qt::ColorScheme::Dark));
        eTheme->setFollowSystemTheme(false);
        QVERIFY(notify(Qt::ColorScheme::Light));
        QCOMPARE(eTheme->getThemeMode(), ElaThemeType::Dark);
    }
    void accentAndResetNotifications()
    {
        const QColor defaultLight = eTheme->getAccentColor(ElaThemeType::Light);
        const QColor defaultDark = eTheme->getAccentColor(ElaThemeType::Dark);
        QSignalSpy colors(eTheme, &ElaTheme::themeColorsChanged);
        QSignalSpy modes(eTheme, &ElaTheme::themeModeChanged);
        const QColor accent("#a04080");
        eTheme->setAccentColor(accent);
        QCOMPARE(colors.count(), 2);
        for (auto mode : {ElaThemeType::Light, ElaThemeType::Dark})
        {
            QCOMPARE(eTheme->getAccentColor(mode), accent);
            QVERIFY(eTheme->getThemeColor(mode, ElaThemeType::PrimaryHover) != accent);
            QVERIFY(eTheme->getThemeColor(mode, ElaThemeType::PrimaryPress) != accent);
        }
        eTheme->setAccentColor(accent);
        QCOMPARE(colors.count(), 2);
        eTheme->setAccentColor(ElaThemeType::Dark, QColor("#e080c0"));
        QCOMPARE(eTheme->getAccentColor(ElaThemeType::Light), accent);
        QCOMPARE(colors.count(), 3);
        eTheme->setFollowSystemTheme(true);
        const auto mode = eTheme->getThemeMode();
        modes.clear();
        eTheme->resetThemeColors();
        QCOMPARE(colors.count(), 5);
        QCOMPARE(eTheme->getAccentColor(ElaThemeType::Light), defaultLight);
        QCOMPARE(eTheme->getAccentColor(ElaThemeType::Dark), defaultDark);
        QVERIFY(eTheme->getFollowSystemTheme());
        QCOMPARE(eTheme->getThemeMode(), mode);
        QCOMPARE(modes.count(), 0);
        eTheme->resetThemeColors();
        QCOMPARE(colors.count(), 5);
    }
    void invalidInputsDoNotMutateTheme()
    {
        QSignalSpy colors(eTheme, &ElaTheme::themeColorsChanged);
        QSignalSpy modes(eTheme, &ElaTheme::themeModeChanged);
        eTheme->setThemeColor(ElaThemeType::Light, ElaThemeType::PrimaryNormal, QColor());
        eTheme->setThemeColor(ElaThemeType::Light, static_cast<ElaThemeType::ThemeColor>(-1), Qt::red);
        eTheme->setThemeColor(static_cast<ElaThemeType::ThemeMode>(2), ElaThemeType::PrimaryNormal, Qt::red);
        eTheme->setAccentColor(QColor());
        eTheme->setThemeMode(static_cast<ElaThemeType::ThemeMode>(-1));
        QCOMPARE(colors.count(), 0);
        QCOMPARE(modes.count(), 0);
        QVERIFY(!eTheme->getThemeColor(ElaThemeType::Light, static_cast<ElaThemeType::ThemeColor>(43)).isValid());
        QVERIFY(!eTheme->getAccentColor(static_cast<ElaThemeType::ThemeMode>(99)).isValid());
    }
    void localButtonColorsRemainOverrides()
    {
        ElaPushButton button;
        ElaIconButton icon(ElaIconType::Heart);
        button.setLightDefaultColor(Qt::red);
        icon.setLightIconColor(Qt::green);
        eTheme->setThemeColor(ElaThemeType::Light, ElaThemeType::BasicBase, Qt::blue);
        eTheme->setThemeColor(ElaThemeType::Light, ElaThemeType::BasicText, Qt::blue);
        QCOMPARE(button.getLightDefaultColor(), QColor(Qt::red));
        QCOMPARE(icon.getLightIconColor(), QColor(Qt::green));
        button.setLightDefaultColor(QColor());
        icon.setLightIconColor(QColor());
        QCOMPARE(button.getLightDefaultColor(), QColor(Qt::blue));
        QCOMPARE(icon.getLightIconColor(), QColor(Qt::blue));
    }
    void openWidgetsRefreshWithoutModeChange()
    {
        ElaText label("配色预览");
        ElaMenu menu;
        menu.addAction("菜单选项");
        label.show();
        menu.popup(QPoint(100, 100));
        QTRY_VERIFY(label.isVisible());
        QCoreApplication::processEvents();

        PaintObserver paints;
        label.installEventFilter(&paints);
        QSignalSpy modes(eTheme, &ElaTheme::themeModeChanged);
        eTheme->setThemeColor(ElaThemeType::Light, ElaThemeType::BasicText, QColor("#e01020"));
        eTheme->setThemeColor(ElaThemeType::Light, ElaThemeType::PopupBase, QColor("#204060"));
        QTRY_COMPARE(label.palette().color(QPalette::WindowText), QColor("#e01020"));
        QTRY_VERIFY(paints.count > 0);
        // 精确检查背景色，避免展开动画自然移动造成假阳性。
        const auto menuImage = menu.grab().toImage();
        const auto ratio = menuImage.devicePixelRatio();
        QCOMPARE(menuImage.pixelColor(qRound((menu.width() / 2.0) * ratio), qRound(8 * ratio)), QColor("#204060"));
        QCOMPARE(modes.count(), 0);
    }
    void iconHoverUsesThemeAlphaAfterClearingOverride()
    {
        eTheme->setThemeColor(ElaThemeType::Light, ElaThemeType::BasicHoverAlpha, QColor(10, 20, 30, 10));
        ElaIconButton icon(ElaIconType::None);
        icon.resize(40, 40);
        icon.setAttribute(Qt::WA_TranslucentBackground);
        icon.setLightHoverColor(Qt::red);
        icon.setLightHoverColor(QColor());
        auto backgroundAlpha = [&]() {
            QImage image(icon.size(), QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
            icon.render(&image, QPoint(), QRegion(), QWidget::DrawChildren);
            return image.pixelColor(2, 2).alpha();
        };
        backgroundAlpha();
        QEnterEvent enter(QPointF(2, 2), QPointF(2, 2), QPointF(2, 2));
        QApplication::sendEvent(&icon, &enter);
        // 固定动画进度检查实际绘制，避免依赖 CI 的计时精度。
        auto* animation = icon.findChild<QPropertyAnimation*>();
        QVERIFY(animation);
        animation->setCurrentTime(animation->duration() / 2);
        QVERIFY(backgroundAlpha() > 0);
        QVERIFY(backgroundAlpha() <= 10);
    }
    void compositeButtonsRefreshTheirThemeRoles()
    {
        QWidget area;
        ElaPopularCard card(&area);
        auto* actionButton = area.findChild<ElaPushButton*>();
        QVERIFY(actionButton);
        const QColor original = actionButton->getLightDefaultColor();
        eTheme->setAccentColor(QColor("#904080"));
        QCOMPARE(actionButton->getLightDefaultColor(), QColor("#904080"));
        eTheme->resetThemeColors();
        QCOMPARE(actionButton->getLightDefaultColor(), original);

        ElaDockWidget dock;
        const auto buttons = dock.titleBarWidget()->findChildren<ElaIconButton*>();
        QCOMPARE(buttons.size(), 2);
        eTheme->setThemeColor(ElaThemeType::Light, ElaThemeType::StatusDanger, QColor("#a02030"));
        eTheme->setThemeColor(ElaThemeType::Light, ElaThemeType::BasicHoverAlpha, QColor("#405060"));
        for (const auto* button : buttons)
        {
            QCOMPARE(button->getLightHoverColor(),
                     button->getAwesome() == ElaIconType::Xmark ? QColor("#a02030") : QColor("#405060"));
        }
    }
    void colorChangesSchedulePainting()
    {
        ElaScrollPageArea area;
        area.resize(200, 80);
        PaintObserver paints;
        area.installEventFilter(&paints);
        area.show();
        QTRY_VERIFY(paints.count > 0);
        QCoreApplication::processEvents();
        paints.count = 0;
        eTheme->setThemeColor(ElaThemeType::Light, ElaThemeType::BasicBaseAlpha, Qt::red);
        QTRY_VERIFY(paints.count > 0);
    }
    void repeatedModeDoesNotNotify()
    {
        QSignalSpy changed(eTheme, &ElaTheme::themeModeChanged);
        eTheme->setThemeMode(eTheme->getThemeMode());
        QCOMPARE(changed.count(), 0);
    }
    void existingButtonFollowsDefaultColors()
    {
        ElaPushButton button;
        const QColor original = eTheme->getThemeColor(ElaThemeType::Light, ElaThemeType::BasicBase);
        eTheme->setThemeColor(ElaThemeType::Light, ElaThemeType::BasicBase, QColor("#123456"));
        const QColor actual = button.getLightDefaultColor();
        eTheme->setThemeColor(ElaThemeType::Light, ElaThemeType::BasicBase, original);
        QCOMPARE(actual, QColor("#123456"));
    }
};
QTEST_MAIN(ThemeTest)
#include "tst_theme.moc"
