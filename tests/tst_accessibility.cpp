#include "ElaCheckBox.h"
#include "ElaSlider.h"
#include "ElaToggleButton.h"
#include "ElaToggleSwitch.h"

#include <QAccessible>
#include <QPointer>
#include <QSignalSpy>
#include <QLineEdit>
#include <QVBoxLayout>
#include <functional>
#include <QtTest>

namespace
{
std::function<void(QAccessibleEvent*)> updateCallback;
void recordAccessibilityUpdate(QAccessibleEvent* event)
{
    if (updateCallback)
        updateCallback(event);
}
class AccessibilityObserver
{
public:
    explicit AccessibilityObserver(std::function<void(QAccessibleEvent*)> callback)
    {
        updateCallback = std::move(callback);
        _previous = QAccessible::installUpdateHandler(recordAccessibilityUpdate);
    }
    ~AccessibilityObserver()
    {
        QAccessible::installUpdateHandler(_previous);
        updateCallback = {};
    }

private:
    QAccessible::UpdateHandler _previous;
};
} // namespace

class AccessibilityTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void switchExposesRoleStateAndActions()
    {
        ElaToggleSwitch toggle;
        toggle.setAccessibleName("启用通知");
        toggle.setAccessibleDescription("控制是否显示桌面通知");
        toggle.show();
        auto* iface = QAccessible::queryAccessibleInterface(&toggle);
        QVERIFY(iface);
        QCOMPARE(iface->role(), QAccessible::Switch);
        QCOMPARE(iface->text(QAccessible::Name), QString("启用通知"));
        QCOMPARE(iface->text(QAccessible::Description), QString("控制是否显示桌面通知"));
        QVERIFY(iface->state().checkable);
        QVERIFY(iface->state().focusable);
        QVERIFY(!iface->state().checked);
        auto* actions = iface->actionInterface();
        QVERIFY(actions);
        QCOMPARE(actions->actionNames().first(), QAccessibleActionInterface::toggleAction());
        QVERIFY(actions->actionNames().contains(QAccessibleActionInterface::pressAction()));
        QVERIFY(!actions->keyBindingsForAction(QAccessibleActionInterface::toggleAction()).isEmpty());
        QSignalSpy changed(&toggle, &ElaToggleSwitch::toggled);
        actions->doAction(QAccessibleActionInterface::toggleAction());
        QVERIFY(toggle.getIsToggled());
        QVERIFY(iface->state().checked);
        QCOMPARE(changed.count(), 1);
        toggle.setEnabled(false);
        QVERIFY(iface->state().disabled);
        actions->doAction(QAccessibleActionInterface::toggleAction());
        QCOMPARE(changed.count(), 1);
        toggle.setEnabled(true);
        toggle.hide();
        actions->doAction(QAccessibleActionInterface::toggleAction());
        QCOMPARE(changed.count(), 1);
    }

    void buttonUsesTextUnlessNameIsExplicit()
    {
        ElaToggleButton button("固定工具栏");
        button.show();
        auto* iface = QAccessible::queryAccessibleInterface(&button);
        QVERIFY(iface);
        QCOMPARE(iface->role(), QAccessible::Button);
        QCOMPARE(iface->text(QAccessible::Name), QString("固定工具栏"));
        QVERIFY(iface->state().checkable);
        button.setText("锁定工具栏");
        QCOMPARE(iface->text(QAccessible::Name), QString("锁定工具栏"));
        button.setAccessibleName("固定顶部工具栏");
        button.setText("固定");
        QCOMPARE(iface->text(QAccessible::Name), QString("固定顶部工具栏"));
        button.setAccessibleName("");
        QCOMPARE(iface->text(QAccessible::Name), QString("固定"));
        auto* actions = iface->actionInterface();
        QVERIFY(actions);
        actions->doAction(QAccessibleActionInterface::toggleAction());
        QVERIFY(button.getIsToggled());
        QVERIFY(iface->state().checked);
        actions->doAction(QAccessibleActionInterface::pressAction());
        QVERIFY(!button.getIsToggled());
        button.setEnabled(false);
        actions->doAction(QAccessibleActionInterface::toggleAction());
        QVERIFY(!button.getIsToggled());
    }

    void accessibleActionCanDestroyWidget()
    {
        QPointer<ElaToggleButton> button = new ElaToggleButton("切换");
        button->show();
        auto* iface = QAccessible::queryAccessibleInterface(button);
        QVERIFY(iface && iface->actionInterface());
        connect(button, &ElaToggleButton::toggled, this, [button] {
            delete button.data();
        });
        iface->actionInterface()->doAction(QAccessibleActionInterface::toggleAction());
        QVERIFY(!button);
    }

    void checkedAndNameChangesNotifyAccessibilityClients()
    {
        ElaToggleButton button("初始名称");
        ElaToggleSwitch toggle;
        int buttonChanges = 0;
        int switchChanges = 0;
        int nameChanges = 0;
        AccessibilityObserver observer([&](QAccessibleEvent* event) {
            if (event->type() == QAccessible::StateChanged &&
                static_cast<QAccessibleStateChangeEvent*>(event)->changedStates().checked)
            {
                if (event->object() == &button)
                    ++buttonChanges;
                if (event->object() == &toggle)
                    ++switchChanges;
            }
            if (event->object() == &button && event->type() == QAccessible::NameChanged)
                ++nameChanges;
        });
        button.setIsToggled(true);
        button.setIsToggled(true);
        toggle.setIsToggled(true);
        toggle.setIsToggled(true);
        QCOMPARE(buttonChanges, 1);
        QCOMPARE(switchChanges, 1);
        button.setText("新名称");
        button.setText("新名称");
        QCOMPARE(nameChanges, 1);
        button.setAccessibleName("明确名称");
        const int afterExplicitName = nameChanges;
        button.setText("仅改变显示文字");
        QCOMPARE(nameChanges, afterExplicitName);
    }

    void accessibilityCallbackPreservesPublicSignalOrder()
    {
        ElaToggleButton button;
        ElaToggleSwitch toggle;
        QSignalSpy buttonChanges(&button, &ElaToggleButton::toggled);
        QSignalSpy switchChanges(&toggle, &ElaToggleSwitch::toggled);
        bool changedButton = false;
        bool changedSwitch = false;
        AccessibilityObserver observer([&](QAccessibleEvent* event) {
            if (event->type() != QAccessible::StateChanged)
                return;
            if (event->object() == &button && !changedButton)
            {
                changedButton = true;
                button.setIsToggled(false);
            }
            if (event->object() == &toggle && !changedSwitch)
            {
                changedSwitch = true;
                toggle.setIsToggled(false);
            }
        });
        button.setIsToggled(true);
        toggle.setIsToggled(true);
        QVERIFY(!button.getIsToggled());
        QVERIFY(!toggle.getIsToggled());
        for (const auto* spy : {&buttonChanges, &switchChanges})
        {
            QCOMPARE(spy->count(), 2);
            QCOMPARE(spy->at(0).at(0).toBool(), true);
            QCOMPARE(spy->at(1).at(0).toBool(), false);
        }
    }

    void focusActionMovesKeyboardFocus()
    {
        QWidget window;
        auto* layout = new QVBoxLayout(&window);
        auto* before = new QLineEdit(&window);
        auto* button = new ElaToggleButton("切换", &window);
        auto* toggle = new ElaToggleSwitch(&window);
        layout->addWidget(before);
        layout->addWidget(button);
        layout->addWidget(toggle);
        window.show();
        window.activateWindow();
        before->setFocus();
        QTRY_VERIFY(before->hasFocus());
        for (QWidget* widget : {static_cast<QWidget*>(button), static_cast<QWidget*>(toggle)})
        {
            auto* iface = QAccessible::queryAccessibleInterface(widget);
            QVERIFY(iface && iface->actionInterface());
            iface->actionInterface()->doAction(QAccessibleActionInterface::setFocusAction());
            QTRY_VERIFY(widget->hasFocus());
            QVERIFY(iface->state().focused);
        }
    }

    void nativeQtControlsKeepTheirInterfaces()
    {
        ElaCheckBox checkbox("自动保存");
        auto* check = QAccessible::queryAccessibleInterface(&checkbox);
        QVERIFY(check);
        QCOMPARE(check->role(), QAccessible::CheckBox);
        checkbox.setChecked(true);
        QVERIFY(check->state().checked);
        ElaSlider slider;
        slider.setRange(0, 100);
        slider.setValue(42);
        auto* range = QAccessible::queryAccessibleInterface(&slider);
        QVERIFY(range && range->valueInterface());
        QCOMPARE(range->role(), QAccessible::Slider);
        QCOMPARE(range->valueInterface()->currentValue().toInt(), 42);
    }
};

QTEST_MAIN(AccessibilityTest)
#include "tst_accessibility.moc"
