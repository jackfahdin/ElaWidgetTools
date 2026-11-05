#ifndef ELAWORKSPACE_ELAWIDGETTOOLS_DEVELOPERCOMPONENTS_ELASUGGESTMODEL_H_
#define ELAWORKSPACE_ELAWIDGETTOOLS_DEVELOPERCOMPONENTS_ELASUGGESTMODEL_H_

#include <QAbstractListModel>

#include "ElaSuggestBox.h"
class ElaSuggestModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ElaSuggestModel(QObject* parent = nullptr);
    ~ElaSuggestModel();
    int rowCount(const QModelIndex& parent) const;
    QVariant data(const QModelIndex& index, int role) const;
    void setSearchSuggestion(const QList<ElaSuggestBox::SuggestData>& suggestions);
    void clearSearchNode();
    const ElaSuggestBox::SuggestData* getSearchSuggestion(int row) const;

private:
    QList<ElaSuggestBox::SuggestData> _suggestions; // 模型持有结果副本，删除本地建议不会留下悬空指针。
};

#endif // ELAWORKSPACE_ELAWIDGETTOOLS_DEVELOPERCOMPONENTS_ELASUGGESTMODEL_H_
