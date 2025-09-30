#pragma once

#include <QComboBox>
#include <QList>

class QStandardItemModel;
class QStandardItem;

class CheckComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit CheckComboBox(QWidget *parent = nullptr);

    void addAllItem(const QString &text);
    void addCheckItem(const QString &text, int value, bool checked = false);

    QList<int> checkedValues() const;
    void setCheckedValues(const QList<int> &values);

    void setPlaceholderText(const QString &text);

signals:
    void selectionChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void handleItemChanged(QStandardItem *item);

private:
    void toggleIndex(const QModelIndex &index);
    void syncAllItemState();
    void updateDisplayText();

    QStandardItemModel *model_;
    int allItemIndex_ = -1;
    QString allText_;
    QString placeholder_;
    bool updating_ = false;
};
