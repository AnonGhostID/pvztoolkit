#include "../inc/checkcombobox.h"

#include <QAbstractItemView>
#include <QEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QModelIndex>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QSet>
#include <QStringList>

CheckComboBox::CheckComboBox(QWidget *parent)
    : QComboBox(parent)
    , model_(new QStandardItemModel(this))
{
    setModel(model_);
    setEditable(true);
    setInsertPolicy(QComboBox::NoInsert);
    lineEdit()->setReadOnly(true);
    placeholder_ = tr("Select");
    lineEdit()->setPlaceholderText(placeholder_);
    setSizeAdjustPolicy(QComboBox::AdjustToContents);
    view()->setSelectionMode(QAbstractItemView::NoSelection);
    view()->viewport()->installEventFilter(this);

    connect(model_, &QStandardItemModel::itemChanged, this, &CheckComboBox::handleItemChanged);
}

void CheckComboBox::addAllItem(const QString &text)
{
    allItemIndex_ = model_->rowCount();
    allText_ = text;

    auto *item = new QStandardItem(text);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    item->setData(-1, Qt::UserRole);
    item->setCheckable(true);
    item->setCheckState(Qt::Unchecked);
    model_->appendRow(item);

    updateDisplayText();
}

void CheckComboBox::addCheckItem(const QString &text, int value, bool checked)
{
    auto *item = new QStandardItem(text);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    item->setData(value, Qt::UserRole);
    item->setCheckable(true);
    item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    model_->appendRow(item);

    if (checked)
        updateDisplayText();
}

QList<int> CheckComboBox::checkedValues() const
{
    QList<int> values;

    if (allItemIndex_ >= 0)
    {
        QStandardItem *allItem = model_->item(allItemIndex_);
        if (allItem && allItem->checkState() == Qt::Checked)
        {
            for (int row = 0; row < model_->rowCount(); ++row)
            {
                if (row == allItemIndex_)
                    continue;
                QStandardItem *item = model_->item(row);
                if (item)
                    values.append(item->data(Qt::UserRole).toInt());
            }
            return values;
        }
    }

    for (int row = 0; row < model_->rowCount(); ++row)
    {
        if (row == allItemIndex_)
            continue;
        QStandardItem *item = model_->item(row);
        if (item && item->checkState() == Qt::Checked)
            values.append(item->data(Qt::UserRole).toInt());
    }

    return values;
}

void CheckComboBox::setCheckedValues(const QList<int> &values)
{
    updating_ = true;

    if (allItemIndex_ >= 0)
    {
        if (QStandardItem *allItem = model_->item(allItemIndex_))
            allItem->setCheckState(Qt::Unchecked);
    }

    QSet<int> valueSet = QSet<int>(values.begin(), values.end());

    for (int row = 0; row < model_->rowCount(); ++row)
    {
        if (row == allItemIndex_)
            continue;

        QStandardItem *item = model_->item(row);
        if (!item)
            continue;

        int value = item->data(Qt::UserRole).toInt();
        item->setCheckState(valueSet.contains(value) ? Qt::Checked : Qt::Unchecked);
    }

    syncAllItemState();

    updating_ = false;
    updateDisplayText();
    emit selectionChanged();
}

void CheckComboBox::setPlaceholderText(const QString &text)
{
    placeholder_ = text;
    if (lineEdit())
        lineEdit()->setPlaceholderText(text);
}

bool CheckComboBox::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == view()->viewport())
    {
        if (event->type() == QEvent::MouseButtonRelease)
        {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            QModelIndex index = view()->indexAt(mouseEvent->pos());
            if (index.isValid())
                toggleIndex(index);
            return true;
        }
        else if (event->type() == QEvent::MouseButtonPress)
        {
            return true;
        }
    }

    return QComboBox::eventFilter(watched, event);
}

void CheckComboBox::handleItemChanged(QStandardItem *item)
{
    if (updating_ || !item)
        return;

    int row = item->row();

    if (row == allItemIndex_)
    {
        updating_ = true;
        Qt::CheckState state = item->checkState();
        for (int i = 0; i < model_->rowCount(); ++i)
        {
            if (i == allItemIndex_)
                continue;
            if (QStandardItem *it = model_->item(i))
                it->setCheckState(state);
        }
        updating_ = false;
    }
    else if (allItemIndex_ >= 0)
    {
        updating_ = true;
        syncAllItemState();
        updating_ = false;
    }

    updateDisplayText();
    emit selectionChanged();
}

void CheckComboBox::toggleIndex(const QModelIndex &index)
{
    QStandardItem *item = model_->itemFromIndex(index);
    if (!item || !(item->flags() & Qt::ItemIsEnabled))
        return;

    Qt::CheckState state = item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked;
    item->setCheckState(state);
}

void CheckComboBox::syncAllItemState()
{
    if (allItemIndex_ < 0)
        return;

    QStandardItem *allItem = model_->item(allItemIndex_);
    if (!allItem)
        return;

    bool allChecked = true;
    for (int row = 0; row < model_->rowCount(); ++row)
    {
        if (row == allItemIndex_)
            continue;
        QStandardItem *item = model_->item(row);
        if (!item)
            continue;
        if (item->checkState() != Qt::Checked)
        {
            allChecked = false;
            break;
        }
    }

    allItem->setCheckState(allChecked ? Qt::Checked : Qt::Unchecked);
}

void CheckComboBox::updateDisplayText()
{
    if (!lineEdit())
        return;

    QString displayText;

    if (allItemIndex_ >= 0)
    {
        if (QStandardItem *allItem = model_->item(allItemIndex_))
        {
            if (allItem->checkState() == Qt::Checked)
                displayText = allText_;
        }
    }

    if (displayText.isEmpty())
    {
        QStringList parts;
        for (int row = 0; row < model_->rowCount(); ++row)
        {
            if (row == allItemIndex_)
                continue;
            QStandardItem *item = model_->item(row);
            if (item && item->checkState() == Qt::Checked)
                parts.append(item->text());
        }

        displayText = parts.join(", ");
    }

    if (displayText.isEmpty())
        displayText = placeholder_;

    lineEdit()->setText(displayText);
    lineEdit()->setCursorPosition(0);
    lineEdit()->setToolTip(displayText);
}
