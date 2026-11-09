#include "ElaApplication.h"
#include "ElaTheme.h"
#include "ElaToggleSwitch.h"
#include "ElaToggleButton.h"
#include <QPointer>
#include <QHideEvent>
#include <memory>

#include <QSignalSpy>
#include <QtTest>
#include <QLineEdit>
#include <QVBoxLayout>

class EscapeReceiver : public QWidget
{
public:
    int escapes{0};

protected:
    void keyPressEvent(QKeyEvent* event) override
    {
        if (event->key() == Qt::Key_Escape)
        {
            ++escapes;
            event->accept();
        }
        else
            QWidget::keyPressEvent(event);
    }
};

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
        // macOS 可按系统设置跳过原生按钮；文本框作为跨平台焦点参照。
        auto* before = new QLineEdit(&window);
        auto* toggle = new ElaToggleSwitch(&window);
        auto* after = new QLineEdit(&window);
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

    void toggleButtonEmitsOnlyOnChange()
    {
        ElaToggleButton button;
        QSignalSpy changed(&button, &ElaToggleButton::toggled);
        button.setIsToggled(false);
        QCOMPARE(changed.count(), 0);
        button.setIsToggled(true);
        button.setIsToggled(true);
        QCOMPARE(changed.count(), 1);
        button.setIsToggled(false);
        QCOMPARE(changed.count(), 2);
    }

    void toggleButtonRequiresPairedLeftClickInside()
    {
        ElaToggleButton button;
        button.show();
        QSignalSpy changed(&button, &ElaToggleButton::toggled);
        QTest::mouseClick(&button, Qt::RightButton);
        QTest::mouseRelease(&button, Qt::LeftButton);
        QCOMPARE(changed.count(), 0);
        QTest::mousePress(&button, Qt::LeftButton);
        QTest::mouseRelease(&button, Qt::LeftButton, Qt::NoModifier, QPoint(-10, -10));
        QCOMPARE(changed.count(), 0);
        QTest::mouseClick(&button, Qt::LeftButton);
        QCOMPARE(changed.count(), 1);
        button.setEnabled(false);
        QTest::mouseClick(&button, Qt::LeftButton);
        QCOMPARE(changed.count(), 1);
    }

    void toggleButtonParticipatesInKeyboardNavigation()
    {
        QWidget window;
        auto* layout = new QVBoxLayout(&window);
        auto* before = new QLineEdit(&window);
        auto* button = new ElaToggleButton("切换", &window);
        auto* after = new QLineEdit(&window);
        layout->addWidget(before);
        layout->addWidget(button);
        layout->addWidget(after);
        window.show();
        window.activateWindow();
        before->setFocus();
        QTRY_VERIFY(before->hasFocus());
        QTest::keyClick(before, Qt::Key_Tab);
        QTRY_VERIFY(button->hasFocus());
        QSignalSpy changed(button, &ElaToggleButton::toggled);
        QTest::keyPress(button, Qt::Key_Space);
        QCOMPARE(changed.count(), 0);
        QTest::keyRelease(button, Qt::Key_Space);
        QCOMPARE(changed.count(), 1);
        QTest::keyClick(button, Qt::Key_Tab);
        QTRY_VERIFY(after->hasFocus());
        QTest::keyClick(after, Qt::Key_Backtab);
        QTRY_VERIFY(button->hasFocus());
    }

    void toggleButtonCancelsInterruptedInput()
    {
        ElaToggleButton button;
        button.show();
        QSignalSpy changed(&button, &ElaToggleButton::toggled);
        QTest::keyPress(&button, Qt::Key_Space);
        QKeyEvent repeatRelease(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, QString(), true);
        QKeyEvent repeatPress(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, QString(), true);
        QApplication::sendEvent(&button, &repeatRelease);
        QApplication::sendEvent(&button, &repeatPress);
        QCOMPARE(changed.count(), 0);
        QTest::keyRelease(&button, Qt::Key_Space);
        QCOMPARE(changed.count(), 1);
        for (auto reason : {QEvent::FocusOut, QEvent::Hide, QEvent::WindowDeactivate})
        {
            QTest::keyPress(&button, Qt::Key_Space);
            std::unique_ptr<QEvent> event;
            if (reason == QEvent::FocusOut)
                event = std::make_unique<QFocusEvent>(reason);
            else if (reason == QEvent::Hide)
                event = std::make_unique<QHideEvent>();
            else
                event = std::make_unique<QEvent>(reason);
            QApplication::sendEvent(&button, event.get());
            QTest::keyRelease(&button, Qt::Key_Space);
            QCOMPARE(changed.count(), 1);
        }
        QTest::keyPress(&button, Qt::Key_Space);
        QTest::keyClick(&button, Qt::Key_Escape);
        QTest::keyRelease(&button, Qt::Key_Space);
        QCOMPARE(changed.count(), 1);
        QTest::mousePress(&button, Qt::LeftButton);
        button.setEnabled(false);
        button.setEnabled(true);
        QTest::mouseRelease(&button, Qt::LeftButton);
        QCOMPARE(changed.count(), 1);
        QTest::keyPress(&button, Qt::Key_Space);
        button.setEnabled(false);
        button.setEnabled(true);
        QTest::keyRelease(&button, Qt::Key_Space);
        QCOMPARE(changed.count(), 1);
    }

    void switchHideAndEscapeCancelPendingInput()
    {
        ElaToggleSwitch toggle;
        toggle.show();
        QSignalSpy changed(&toggle, &ElaToggleSwitch::toggled);
        QTest::keyPress(&toggle, Qt::Key_Space);
        toggle.hide();
        toggle.show();
        QTest::keyRelease(&toggle, Qt::Key_Space);
        QCOMPARE(changed.count(), 0);
        QTest::keyPress(&toggle, Qt::Key_Space);
        QTest::keyClick(&toggle, Qt::Key_Escape);
        QTest::keyRelease(&toggle, Qt::Key_Space);
        QCOMPARE(changed.count(), 0);
    }

    void switchCancelledMousePressRestoresRadius()
    {
        ElaToggleSwitch toggle;
        toggle.show();
        QVERIFY(QTest::qWaitForWindowExposed(&toggle));
        QSignalSpy changed(&toggle, &ElaToggleSwitch::toggled);
        for (bool escape : {true, false})
        {
            QTest::mousePress(&toggle, Qt::LeftButton);
            QTRY_VERIFY(qAbs(toggle.property("circleRadius").toReal() - toggle.height() * 0.25) < 0.01);
            if (escape)
                QTest::keyClick(&toggle, Qt::Key_Escape);
            else
                toggle.setEnabled(false);
            QTest::mouseRelease(&toggle, Qt::LeftButton);
            const qreal radius = toggle.height() * (toggle.isEnabled() && toggle.underMouse() ? 0.35 : 0.3);
            QTRY_VERIFY(qAbs(toggle.property("circleRadius").toReal() - radius) < 0.01);
            QCOMPARE(changed.count(), 0);
            toggle.setEnabled(true);
        }
    }

    void idleEscapeRemainsAvailableToParent()
    {
        EscapeReceiver parent;
        auto* button = new ElaToggleButton(&parent);
        auto* toggle = new ElaToggleSwitch(&parent);
        parent.show();
        for (QWidget* control : {static_cast<QWidget*>(button), static_cast<QWidget*>(toggle)})
        {
            int before = parent.escapes;
            QTest::keyClick(control, Qt::Key_Escape);
            QCOMPARE(parent.escapes, before + 1);
            QTest::keyPress(control, Qt::Key_Space);
            QTest::keyClick(control, Qt::Key_Escape);
            QCOMPARE(parent.escapes, before + 1);
            QTest::keyRelease(control, Qt::Key_Space);
        }
        QVERIFY(!button->getIsToggled());
        QVERIFY(!toggle->getIsToggled());
    }

    void stateNotificationMayDestroyControls()
    {
        QPointer<ElaToggleButton> button = new ElaToggleButton;
        button->show();
        connect(button, &ElaToggleButton::toggled, this, [button] {
            delete button.data();
        });
        QTest::mouseClick(button, Qt::LeftButton);
        QVERIFY(!button);
        QPointer<ElaToggleSwitch> toggle = new ElaToggleSwitch;
        toggle->show();
        connect(toggle, &ElaToggleSwitch::toggled, this, [toggle] {
            delete toggle.data();
        });
        QTest::mouseClick(toggle, Qt::LeftButton);
        QVERIFY(!toggle);
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
