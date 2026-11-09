#ifndef ELAWORKSPACE_ELAWIDGETTOOLS_PRIVATE_ELASTATEWIDGETPRIVATE_H_
#define ELAWORKSPACE_ELAWIDGETTOOLS_PRIVATE_ELASTATEWIDGETPRIVATE_H_

#include "ElaStateWidget.h"
#include <QPointer>
#include <array>

class ElaProgressRing;
class ElaPushButton;
class ElaText;
class QStackedWidget;
class QVBoxLayout;

class ElaStateWidgetPrivate
{
    ELA_PIMPL_D(ElaStateWidget)
private:
    ElaStateWidget::State _state{ElaStateWidget::State::Empty};
    bool _retryEnabled{true};
    bool _updating{false};
    bool _contentRemoved{false};
    QPointer<QWidget> _content;
    QMetaObject::Connection _contentDestroyed;
    QStackedWidget* _stack{nullptr};
    QWidget* _contentPage{nullptr};
    QWidget* _statusPage{nullptr};
    QVBoxLayout* _contentLayout{nullptr};
    ElaProgressRing* _ring{nullptr};
    ElaText* _title{nullptr};
    ElaText* _description{nullptr};
    ElaPushButton* _retry{nullptr};
    std::array<QString, 4> _titles;
    std::array<QString, 4> _descriptions;

    bool _refresh();
    bool _reconcileContent();
    void _updateAnimation();
};

#endif // ELAWORKSPACE_ELAWIDGETTOOLS_PRIVATE_ELASTATEWIDGETPRIVATE_H_
