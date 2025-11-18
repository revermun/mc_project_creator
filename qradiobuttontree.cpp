#include "qradiobuttontree.h"

QRadioButtonTree::QRadioButtonTree(QWidget *parent) : QTreeWidget(parent)
{

}



QTreeWidgetItem* QRadioButtonTree::addItem(QString name)
{
    QStringList lst;
    lst << name;
    QTreeWidgetItem* pItem = new QTreeWidgetItem(this, lst, 0);
    return pItem;
}

QTreeWidgetItem* QRadioButtonTree::addRadio(QTreeWidgetItem* item, QString name)
{
    QStringList lst;
//    lst << name;
    QTreeWidgetItem* pItem = new QTreeWidgetItem(item, lst, 0);
    this->setItemWidget(pItem, 0, new QRadioButton(name));
    return pItem;
}
