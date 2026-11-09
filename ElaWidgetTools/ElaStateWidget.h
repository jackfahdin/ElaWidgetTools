#ifndef ELAWORKSPACE_ELAWIDGETTOOLS_ELASTATEWIDGET_H_
#define ELAWORKSPACE_ELAWIDGETTOOLS_ELASTATEWIDGET_H_

#include <QWidget>

#include "ElaPropertyMacro.h"
#include "ElaWidgetToolsExport.h"

class ElaStateWidgetPrivate;
class ELA_EXPORT ElaStateWidget : public QWidget
{
    Q_OBJECT
    ELA_PIMPL_Q(ElaStateWidget)
public:
    enum class State
    {
        Content,
        Loading,
        Empty,
        Error
    };
    Q_ENUM(State)
    Q_PROPERTY(State state READ getState WRITE setState NOTIFY stateChanged)

    explicit ElaStateWidget(QWidget* parent = nullptr);
    ~ElaStateWidget() override;

    void setState(State state);
    Q_REQUIRED_RESULT State getState() const;

    // 成功后接管内容。替换时延迟销毁旧控件；不自动改变状态。
    bool setContentWidget(QWidget* widget);
    Q_REQUIRED_RESULT QWidget* getContentWidget() const;
    // 移出并隐藏内容，调用者接回所有权。
    Q_REQUIRED_RESULT QWidget* takeContentWidget();

    void setStateText(State state, const QString& title, const QString& description = QString());
    void setRetryEnabled(bool enabled);
    Q_REQUIRED_RESULT bool getRetryEnabled() const;

Q_SIGNALS:
    void stateChanged(ElaStateWidget::State state);
    void retryRequested();

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
};

#endif // ELAWORKSPACE_ELAWIDGETTOOLS_ELASTATEWIDGET_H_
