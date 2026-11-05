#include "ElaApplication.h"
#include "ElaTheme.h"
#include "ElaToggleSwitch.h"

#include <QSignalSpy>
#include <QtTest>
#include <QPushButton>
#include <QVBoxLayout>

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

    void switchIgnoresRightClick()
    {
        ElaToggleSwitch toggle;
        toggle.show();
        QSignalSpy changed(&toggle, &ElaToggleSwitch::toggled);
        QTest::mouseClick(&toggle, Qt::RightButton);
        QVERIFY(!toggle.getIsToggled());
        QCOMPARE(changed.count(), 0);
    }

    void switchRespondsToSpace()
    {
        ElaToggleSwitch toggle;
        toggle.show();
        QSignalSpy changed(&toggle, &ElaToggleSwitch::toggled);
        QTest::keyPress(&toggle, Qt::Key_Space);
        QVERIFY(!toggle.getIsToggled());
        QTest::keyRelease(&toggle, Qt::Key_Space);
        QVERIFY(toggle.getIsToggled());
        QCOMPARE(changed.count(), 1);
        QCOMPARE(toggle.focusPolicy(), Qt::StrongFocus);
    }

    void switchParticipatesInTabFocus()
    {
        QWidget window;
        auto* layout = new QVBoxLayout(&window);
        auto* before = new QPushButton("前一个控件", &window);
        auto* toggle = new ElaToggleSwitch(&window);
        auto* after = new QPushButton("后一个控件", &window);
        layout->addWidget(before);
        layout->addWidget(toggle);
        layout->addWidget(after);
        window.show();
        window.activateWindow();
        before->setFocus();
        QTRY_VERIFY(before->hasFocus());
        QTest::keyClick(before, Qt::Key_Tab);
        QTRY_VERIFY(toggle->hasFocus());
        QTest::keyClick(toggle, Qt::Key_Space);
        QVERIFY(toggle->getIsToggled());
        QTest::keyClick(toggle, Qt::Key_Tab);
        QTRY_VERIFY(after->hasFocus());
    }

    void switchCancelsInterruptedKeyboardPress()
    {
        ElaToggleSwitch toggle;
        toggle.show();
        QSignalSpy changed(&toggle, &ElaToggleSwitch::toggled);
        QTest::keyPress(&toggle, Qt::Key_Space);
        QKeyEvent repeatPress(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, QString(), true);
        QKeyEvent repeatRelease(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, QString(), true);
        QApplication::sendEvent(&toggle, &repeatRelease);
        QApplication::sendEvent(&toggle, &repeatPress);
        QCOMPARE(changed.count(), 0);
        QTest::keyRelease(&toggle, Qt::Key_Space);
        QCOMPARE(changed.count(), 1);
        QTest::keyPress(&toggle, Qt::Key_Space);
        QFocusEvent lostFocus(QEvent::FocusOut);
        QApplication::sendEvent(&toggle, &lostFocus);
        QTest::keyRelease(&toggle, Qt::Key_Space);
        QCOMPARE(changed.count(), 1);
        QTest::keyPress(&toggle, Qt::Key_Space);
        toggle.setEnabled(false);
        QTest::keyRelease(&toggle, Qt::Key_Space);
        toggle.setEnabled(true);
        QTest::keyRelease(&toggle, Qt::Key_Space);
        QCOMPARE(changed.count(), 1);
    }

    void switchDragToSameStateDoesNotNotify()
    {
        ElaToggleSwitch toggle;
        toggle.show();
        QSignalSpy changed(&toggle, &ElaToggleSwitch::toggled);
        QTest::mousePress(&toggle, Qt::LeftButton, Qt::NoModifier, QPoint(11, 11));
        QMouseEvent move(QEvent::MouseMove, QPointF(12, 11), QPointF(toggle.mapToGlobal(QPoint(12, 11))), Qt::NoButton,
                         Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(&toggle, &move);
        QTest::mouseRelease(&toggle, Qt::LeftButton, Qt::NoModifier, QPoint(12, 11));
        QVERIFY(!toggle.getIsToggled());
        QCOMPARE(changed.count(), 0);
    }

    void switchCancelsMouseReleaseOutside()
    {
        ElaToggleSwitch toggle;
        toggle.show();
        QSignalSpy changed(&toggle, &ElaToggleSwitch::toggled);
        QTest::mousePress(&toggle, Qt::LeftButton);
        QTest::mouseRelease(&toggle, Qt::LeftButton, Qt::NoModifier, QPoint(-10, -10));
        QVERIFY(!toggle.getIsToggled());
        QCOMPARE(changed.count(), 0);
        QTest::mouseRelease(&toggle, Qt::LeftButton);
        QCOMPARE(changed.count(), 0);
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
