#ifndef ELAWORKSPACE_ELAWIDGETTOOLS_PRIVATE_ELASUGGESTBOXPRIVATE_H_
#define ELAWORKSPACE_ELAWIDGETTOOLS_PRIVATE_ELASUGGESTBOXPRIVATE_H_

#include "ElaPropertyMacro.h"
#include <QAction>
#include <QObject>
#include <QSize>
#include <QVariantMap>
#include <QVector>

#include "ElaWidgetToolsDef.h"
#include "ElaSuggestBox.h"
#include <QPointer>
class QVBoxLayout;
class ElaLineEdit;
class ElaNavigationNode;
class ElaSuggestModel;
class ElaBaseListView;
class ElaSuggestDelegate;
class ElaSuggestBox;
class ElaSuggestBoxSearchViewContainer;
class QTimer;
class QPropertyAnimation;
class ElaSuggestBoxPrivate : public QObject
{
    Q_OBJECT
    ELA_PIMPL_D(ElaSuggestBox)
    ELA_PROPERTY_D(int, BorderRadius)
    ELA_PROPERTY_D(Qt::CaseSensitivity, CaseSensitivity)
public:
    explicit ElaSuggestBoxPrivate(QObject* parent = nullptr);
    ~ElaSuggestBoxPrivate() override;
    Q_SLOT void onThemeModeChanged(ElaThemeType::ThemeMode themeMode);
    Q_SLOT void onSearchEditTextEdit(const QString& searchText);
    Q_SLOT void onSearchViewClicked(const QModelIndex& index);

private:
    ElaThemeType::ThemeMode _themeMode;
    QAction* _lightSearchAction{nullptr};
    QAction* _darkSearchAction{nullptr};
    QList<ElaSuggestBox::SuggestData> _suggestions;
    QTimer* _searchTimer{nullptr};
    bool _asyncSearchEnabled{false};
    int _debounceInterval{250};
    quint64 _nextRequestId{0};
    quint64 _queryRevision{0};
    quint64 _activeRequestId{0};
    bool _requestDispatched{false};
    bool _pendingNotified{false};
    void _publishPending();
    QPointer<QPropertyAnimation> _sizeAnimation;
    bool _cancelRequest();
    void _showSuggestions(const QList<ElaSuggestBox::SuggestData>& suggestions);
    QPointer<ElaSuggestBoxSearchViewContainer> _searchViewBaseWidget;
    ElaLineEdit* _searchEdit{nullptr};
    ElaSuggestModel* _searchModel{nullptr};
    ElaBaseListView* _searchView{nullptr};
    ElaSuggestDelegate* _searchDelegate{nullptr};
    QVBoxLayout* _shadowLayout{nullptr};
    void _startSizeAnimation(QSize oldSize, QSize newSize);
    void _startCloseAnimation();
};

#endif // ELAWORKSPACE_ELAWIDGETTOOLS_PRIVATE_ELASUGGESTBOXPRIVATE_H_
