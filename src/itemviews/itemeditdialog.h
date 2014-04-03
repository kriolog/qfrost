/*
 * Copyright (C) 2012  Denis Pesotsky
 *
 * This file is part of QFrost.
 *
 * QFrost is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef QFGUI_ITEMEDITDIALOG_H
#define QFGUI_ITEMEDITDIALOG_H

#include <QtWidgets/QDialog>

QT_FORWARD_DECLARE_CLASS(QDataWidgetMapper)
QT_FORWARD_DECLARE_CLASS(QVBoxLayout)
QT_FORWARD_DECLARE_CLASS(QDialogButtonBox)
QT_FORWARD_DECLARE_CLASS(QFormLayout)

namespace qfgui
{

class SoilEditDialog;

QT_FORWARD_DECLARE_CLASS(ItemsModel)

class ItemEditDialog : public QDialog
{
    Q_OBJECT
public:
    ItemEditDialog(ItemsModel *model,
                   const QStringList &forbiddenNames,
                   QWidget *parent = NULL);

    /// Блокирует все виджеты, позволяющие ввод, а также кнопку "Ok"
    void disableChanges();

protected:
    /// Добавляет @p widget с подписью @p labelText к нашим виджетам, идущим
    /// после строки редактирования имени.
    void addRow(const QString &labelText, QWidget *widget);

    /// Добавляет @p widget к нашим виджетам, идущим перед кнопками диалога.
    void addWidget(QWidget *widget);
    /// Добавляет @p layout к нашим layouts, идущим перед кнопками диалога.
    void addLayout(QLayout *layout);

    QDataWidgetMapper *mapper() const {
        return mMapper;
    }

    ItemsModel *model() const {
        return mModel;
    }

private:
    ItemsModel *const mModel;
    const QStringList mForbiddenNames;

    QDataWidgetMapper *const mMapper;
    QVBoxLayout *const mLayout;
    QDialogButtonBox *const mButtons;

    QFormLayout *const mTopFormLayout;

private slots:
    /// Изменяет доступность кнопки ok в зависимости от @p name.
    void updateOkButton(const QString &name);
};

}

#endif // QFGUI_ITEMEDITDIALOG_H
