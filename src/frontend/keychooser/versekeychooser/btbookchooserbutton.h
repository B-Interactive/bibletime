/*********
*
* In the name of the Father, and of the Son, and of the Holy Spirit.
*
* This file is part of BibleTime's source code, https://bibletime.info/
*
* Copyright 1999-2025 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License
* version 2.0.
*
**********/

#pragma once

#include <QToolButton>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

class BtBibleKeyWidget;

/**
* Custom popup widget that displays books in two columns.
*/
class BtBookPopup : public QWidget {
    Q_OBJECT

public:
    explicit BtBookPopup(QWidget* parent = nullptr);
    void setBooks(const QStringList& books);
    void popup(const QPoint& pos);

Q_SIGNALS:
    void bookSelected(const QString& bookName);

protected:
    void focusOutEvent(QFocusEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_columnsLayout;
    QVBoxLayout* m_leftColumn;
    QVBoxLayout* m_rightColumn;
    
    void clear();
    QPushButton* createBookButton(const QString& bookName);
};

/**
* Specialized dropdown button for book selection with two-column layout.
*/
class BtBookChooserButton : public QToolButton {
    Q_OBJECT

public:
    BtBookChooserButton(BtBibleKeyWidget& parent);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

Q_SIGNALS:
    void stepItem(int step);
    void bookSelected(const QString& bookName);

private:
    BtBibleKeyWidget& m_parent;
    BtBookPopup* m_popup;
};