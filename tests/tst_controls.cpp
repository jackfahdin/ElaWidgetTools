#include "ElaApplication.h"
#include "ElaTheme.h"
#include "ElaToggleSwitch.h"

#include <QSignalSpy>
#include <QtTest>

class ControlsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        eApp->init();
    }

    void switchEmitsOnlyOnChange()
    {
        ElaToggleSwitch toggle;
        QSignalSpy changed(&toggle, &ElaToggleSwitch::toggled);
        toggle.setIsToggled(false);
        QCOMPARE(changed.count(), 0);
        toggle.setIsToggled(true);
        QTRY_VERIFY(toggle.getIsToggled());
        QCOMPARE(changed.count(), 1);
        QCOMPARE(changed.at(0).at(0).toBool(), true);
        toggle.setIsToggled(true);
        QCOMPARE(changed.count(), 1);
        toggle.setIsToggled(false);
        QTRY_VERIFY(!toggle.getIsToggled());
        QCOMPARE(changed.count(), 2);
        QCOMPARE(changed.at(1).at(0).toBool(), false);
    }

    void switchRespondsToClickAndRespectsDisabledState()
    {
        ElaToggleSwitch toggle;
        toggle.show();
        QSignalSpy changed(&toggle, &ElaToggleSwitch::toggled);
        QTest::mouseClick(&toggle, Qt::LeftButton);
        QTRY_VERIFY(toggle.getIsToggled());
        QCOMPARE(changed.count(), 1);
        toggle.setEnabled(false);
        QTest::mouseClick(&toggle, Qt::LeftButton);
        QVERIFY(toggle.getIsToggled());
        QCOMPARE(changed.count(), 1);
    }

    void themeChangeEmitsSelectedMode()
    {
        const auto original = eTheme->getThemeMode();
        const auto other = original == ElaThemeType::Light ? ElaThemeType::Dark : ElaThemeType::Light;
        QSignalSpy changed(eTheme, &ElaTheme::themeModeChanged);
        eTheme->setThemeMode(other);
        QCOMPARE(eTheme->getThemeMode(), other);
        QCOMPARE(changed.count(), 1);
        QCOMPARE(qvariant_cast<ElaThemeType::ThemeMode>(changed.first().first()), other);
        eTheme->setThemeMode(original);
        QCOMPARE(changed.count(), 2);
        QCOMPARE(qvariant_cast<ElaThemeType::ThemeMode>(changed.last().first()), original);
    }
};

QTEST_MAIN(ControlsTest)
#include "tst_controls.moc"
