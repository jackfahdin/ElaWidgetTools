#ifndef ELAWORKSPACE_ELAWIDGETTOOLS_ELASUGGESTBOX_H_
#define ELAWORKSPACE_ELAWIDGETTOOLS_ELASUGGESTBOX_H_

#include <QVariantMap>
#include <QWidget>

#include "ElaWidgetToolsDef.h"
#include "ElaWidgetToolsExport.h"
#include "ElaPropertyMacro.h"

class ElaSuggestBoxPrivate;
class ELA_EXPORT ElaSuggestBox : public QWidget
{
    Q_OBJECT
    ELA_PIMPL_Q(ElaSuggestBox)
    ELA_PROPERTY_Q_H(int, BorderRadius)
    ELA_PROPERTY_Q_H(Qt::CaseSensitivity, CaseSensitivity)
public:
    explicit ElaSuggestBox(QWidget* parent = nullptr);
    ~ElaSuggestBox() override;
    void setPlaceholderText(const QString& placeholderText);
    void setFixedSize(const QSize& size);
    void setFixedSize(int w, int h);
    void setFixedHeight(int h);

    struct ELA_EXPORT SuggestData
    {
        ELA_VARIABLE(ElaIconType::IconName, ElaIcon)
        ELA_VARIABLE(QString, SuggestText)
        ELA_VARIABLE(QString, SuggestKey)
        ELA_VARIABLE(QVariantMap, SuggestData)
    public:
        explicit SuggestData();
        explicit SuggestData(ElaIconType::IconName icon, const QString& suggestText,
                             const QVariantMap& suggestData = {});
        ~SuggestData();
    };
    QString addSuggestion(const QString& suggestText, const QVariantMap& suggestData = {});
    QString addSuggestion(ElaIconType::IconName icon, const QString& suggestText, const QVariantMap& suggestData = {});
    QString addSuggestion(const ElaSuggestBox::SuggestData& suggestData);
    QStringList addSuggestion(const QList<ElaSuggestBox::SuggestData>& suggestDataList);

    void removeSuggestion(const QString& suggestKey);
    void removeSuggestion(int index);
    void clearSuggestion();

    void setSearchText(const QString& text);
    QString getSearchText() const;
    void setAsyncSearchEnabled(bool enabled);
    bool getAsyncSearchEnabled() const;
    void setSearchDebounceInterval(int milliseconds);
    int getSearchDebounceInterval() const;
    bool getIsSearchPending() const;
    bool setSearchResults(quint64 requestId, const QList<SuggestData>& results);
    void cancelSearch();
Q_SIGNALS:
    Q_SIGNAL void searchRequested(quint64 requestId, const QString& text);
    Q_SIGNAL void searchCancelled(quint64 requestId);
    Q_SIGNAL void searchPendingChanged(bool pending);
    Q_SIGNAL void suggestionClicked(const ElaSuggestBox::SuggestData& suggestData);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void hideEvent(QHideEvent* event) override;
};

Q_DECLARE_METATYPE(ElaSuggestBox::SuggestData)

#endif // ELAWORKSPACE_ELAWIDGETTOOLS_ELASUGGESTBOX_H_
