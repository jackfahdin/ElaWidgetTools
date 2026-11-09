#ifndef ELAWORKSPACE_ELAWIDGETTOOLSEXAMPLE_EXAMPLEPAGE_T_DATAVIEW_H_
#define ELAWORKSPACE_ELAWIDGETTOOLSEXAMPLE_EXAMPLEPAGE_T_DATAVIEW_H_

#include "T_BasePage.h"

class ElaStateWidget;
class ElaLineEdit;
class ElaText;
class QStandardItemModel;
class QSortFilterProxyModel;

class T_DataView : public T_BasePage
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit T_DataView(QWidget* parent = nullptr);

private:
    enum class Result
    {
        Data,
        Empty,
        Error
    };
    ElaStateWidget* _state{nullptr};
    ElaLineEdit* _filter{nullptr};
    ElaText* _count{nullptr};
    QStandardItemModel* _model{nullptr};
    QSortFilterProxyModel* _proxy{nullptr};
    quint64 _requestId{0};
    bool _loading{false};
    bool _failed{false};
    void _load(Result result);
    void _updateResults();
};

#endif // ELAWORKSPACE_ELAWIDGETTOOLSEXAMPLE_EXAMPLEPAGE_T_DATAVIEW_H_
