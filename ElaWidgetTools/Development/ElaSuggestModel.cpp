#include "ElaSuggestModel.h"

ElaSuggestModel::ElaSuggestModel(QObject* parent) : QAbstractListModel{parent}
{
}
ElaSuggestModel::~ElaSuggestModel() = default;

int ElaSuggestModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : _suggestions.count();
}

QVariant ElaSuggestModel::data(const QModelIndex& index, int role) const
{
    const auto* suggestion = index.isValid() ? getSearchSuggestion(index.row()) : nullptr;
    if (!suggestion)
    {
        return {};
    }
    if (role == Qt::DisplayRole)
    {
        return suggestion->getSuggestText();
    }
    if (role == Qt::UserRole)
    {
        return QVariant::fromValue(*suggestion);
    }
    return {};
}

void ElaSuggestModel::setSearchSuggestion(const QList<ElaSuggestBox::SuggestData>& suggestions)
{
    beginResetModel();
    _suggestions = suggestions;
    endResetModel();
}

void ElaSuggestModel::clearSearchNode()
{
    setSearchSuggestion({});
}

const ElaSuggestBox::SuggestData* ElaSuggestModel::getSearchSuggestion(int row) const
{
    return row < 0 || row >= _suggestions.count() ? nullptr : &_suggestions[row];
}
