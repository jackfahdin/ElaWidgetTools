#include "ElaToggleAccessible.h"

#include "ElaToggleButton.h"
#include "ElaToggleSwitch.h"
#include <QAccessibleWidget>

#if QT_CONFIG(accessibility)
namespace
{
class ElaAccessibleToggle : public QAccessibleWidget
{
public:
    explicit ElaAccessibleToggle(QWidget* control, bool isSwitch)
        : QAccessibleWidget(control, isSwitch ? QAccessible::Switch : QAccessible::Button), _isSwitch(isSwitch)
    {
    }

    QString text(QAccessible::Text type) const override
    {
        if (type == QAccessible::Name && widget() && widget()->accessibleName().isEmpty() && !_isSwitch)
        {
            return static_cast<ElaToggleButton*>(widget())->getText();
        }
        return QAccessibleWidget::text(type);
    }

    QAccessible::State state() const override
    {
        auto result = QAccessibleWidget::state();
        result.checkable = true;
        if (widget())
        {
            result.checked = _isSwitch ? static_cast<ElaToggleSwitch*>(widget())->getIsToggled()
                                       : static_cast<ElaToggleButton*>(widget())->getIsToggled();
        }
        return result;
    }

    QStringList actionNames() const override
    {
        QStringList names;
        if (widget() && widget()->isEnabled() && widget()->isVisible())
        {
            names.append(QAccessibleActionInterface::toggleAction());
            names.append(QAccessibleActionInterface::pressAction());
        }
        names.append(QAccessibleWidget::actionNames());
        return names;
    }

    void doAction(const QString& action) override
    {
        auto* control = widget();
        if (!control || !control->isEnabled() || !control->isVisible())
        {
            return;
        }
        if (action == QAccessibleActionInterface::toggleAction() || action == QAccessibleActionInterface::pressAction())
        {
            // 切换可能通过 toggled 槽销毁控件与本接口；调用后不再访问成员。
            if (_isSwitch)
            {
                auto* toggle = static_cast<ElaToggleSwitch*>(control);
                toggle->setIsToggled(!toggle->getIsToggled());
            }
            else
            {
                auto* toggle = static_cast<ElaToggleButton*>(control);
                toggle->setIsToggled(!toggle->getIsToggled());
            }
            return;
        }
        QAccessibleWidget::doAction(action);
    }

    QStringList keyBindingsForAction(const QString& action) const override
    {
        if (action == QAccessibleActionInterface::toggleAction() || action == QAccessibleActionInterface::pressAction())
        {
            return {QStringLiteral("Space")};
        }
        return QAccessibleWidget::keyBindingsForAction(action);
    }

private:
    bool _isSwitch;
};

QAccessibleInterface* toggleFactory(const QString& className, QObject* object)
{
    if (className == QStringLiteral("ElaToggleSwitch"))
    {
        if (auto* toggle = qobject_cast<ElaToggleSwitch*>(object))
            return new ElaAccessibleToggle(toggle, true);
    }
    if (className == QStringLiteral("ElaToggleButton"))
    {
        if (auto* toggle = qobject_cast<ElaToggleButton*>(object))
            return new ElaAccessibleToggle(toggle, false);
    }
    return nullptr;
}
} // namespace
#endif

void elaEnsureToggleAccessibility()
{
#if QT_CONFIG(accessibility)
    static const bool installed = [] {
        QAccessible::installFactory(toggleFactory);
        return true;
    }();
    Q_UNUSED(installed)
#endif
}

void elaNotifyToggleStateChanged(QWidget* widget)
{
#if QT_CONFIG(accessibility)
    QAccessible::State changes;
    changes.checked = true;
    QAccessibleStateChangeEvent event(widget, changes);
    QAccessible::updateAccessibility(&event);
#else
    Q_UNUSED(widget)
#endif
}

void elaNotifyToggleNameChanged(QWidget* widget)
{
#if QT_CONFIG(accessibility)
    if (widget->accessibleName().isEmpty())
    {
        QAccessibleEvent event(widget, QAccessible::NameChanged);
        QAccessible::updateAccessibility(&event);
    }
#else
    Q_UNUSED(widget)
#endif
}
