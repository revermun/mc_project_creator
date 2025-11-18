#ifndef QRADIOBUTTONTREE_H
#define QRADIOBUTTONTREE_H

#include <QTreeWidget>
#include <QCheckBox>
#include <QRadioButton>

class QRadioButtonTree : public QTreeWidget
{
    Q_OBJECT
public:

   explicit QRadioButtonTree(QWidget *parent);
   QTreeWidgetItem* addItem(QString name);
   QTreeWidgetItem* addRadio(QTreeWidgetItem* item, QString name);

};

#endif // QRADIOBUTTONTREE_H
