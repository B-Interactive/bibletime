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

#include "btbookchooserbutton.h"
#include "btbiblekeywidget.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWheelEvent>
#include <QScrollArea>

const unsigned int ARROW_HEIGHT = 15;

// BtBookPopup implementation
BtBookPopup::BtBookPopup(QWidget* parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setFocusPolicy(Qt::StrongFocus);
    
    // Main layout - no scroll area for now to simplify
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    m_mainLayout->setSpacing(5);
    
    // Two-column layout directly in main layout
    m_columnsLayout = new QHBoxLayout();
    m_columnsLayout->setContentsMargins(0, 0, 0, 0);
    m_columnsLayout->setSpacing(15);
    
    // Left and right columns with equal stretch
    m_leftColumn = new QVBoxLayout();
    m_leftColumn->setSpacing(2);
    m_leftColumn->setAlignment(Qt::AlignTop);
    
    m_rightColumn = new QVBoxLayout();
    m_rightColumn->setSpacing(2);
    m_rightColumn->setAlignment(Qt::AlignTop);
    
    m_columnsLayout->addLayout(m_leftColumn, 1);  // Equal stretch
    m_columnsLayout->addLayout(m_rightColumn, 1); // Equal stretch
    
    m_mainLayout->addLayout(m_columnsLayout);
    
    // Set larger size to ensure columns are visible
    setFixedSize(500, 400);
    
    // Apply better styling
    setStyleSheet(
        "BtBookPopup {"
        "   background-color: white;"
        "   border: 2px solid black;"
        "   border-radius: 4px;"
        "}"
        "QPushButton {"
        "   text-align: left;"
        "   padding: 6px 12px;"
        "   border: none;"
        "   background: transparent;"
        "   min-width: 140px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #3498db;"
        "   color: white;"
        "}"
    );
}

void BtBookPopup::setBooks(const QStringList& books) {
    clear();
    
    if (books.isEmpty()) {
        return;  // No books to display
    }
    
    // Distribute books between columns for better balance
    int leftCount = 0;
    int rightCount = 0;
    int totalBooks = books.size();
    int booksPerColumn = (totalBooks + 1) / 2;  // Round up for left column
    
    for (const QString& bookName : books) {
        QPushButton* button = createBookButton(bookName);
        
        // Put first half in left column, rest in right column
        if (leftCount < booksPerColumn) {
            m_leftColumn->addWidget(button);
            leftCount++;
        } else {
            m_rightColumn->addWidget(button);
            rightCount++;
        }
    }
}

void BtBookPopup::popup(const QPoint& pos) {
    // Ensure the popup appears on screen
    QRect screenGeometry = QApplication::desktop()->availableGeometry();
    QPoint adjustedPos = pos;
    
    // Adjust position if it would go off-screen
    if (adjustedPos.x() + width() > screenGeometry.right()) {
        adjustedPos.setX(screenGeometry.right() - width());
    }
    if (adjustedPos.y() + height() > screenGeometry.bottom()) {
        adjustedPos.setY(pos.y() - height());
    }
    
    move(adjustedPos);
    show();
    raise();
    activateWindow();
    setFocus();
}

void BtBookPopup::clear() {
    // Clear both columns
    QLayoutItem* item;
    while ((item = m_leftColumn->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    while ((item = m_rightColumn->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
}

QPushButton* BtBookPopup::createBookButton(const QString& bookName) {
    QPushButton* button = new QPushButton(bookName, this);
    button->setFlat(true);
    
    connect(button, &QPushButton::clicked, [this, bookName]() {
        Q_EMIT bookSelected(bookName);
        hide();
    });
    
    return button;
}

void BtBookPopup::focusOutEvent(QFocusEvent* event) {
    Q_UNUSED(event)
    hide();
}

void BtBookPopup::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        hide();
    }
    QWidget::keyPressEvent(event);
}

// BtBookChooserButton implementation
BtBookChooserButton::BtBookChooserButton(BtBibleKeyWidget& parent)
    : QToolButton(&parent)
    , m_parent(parent)
    , m_popup(nullptr)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAutoRaise(false);
    setArrowType(Qt::NoArrow);
    setFixedHeight(ARROW_HEIGHT);
    setFocusPolicy(Qt::NoFocus);
    setStyleSheet(
        "QToolButton{margin:0px;}"
        "QToolButton::menu-indicator{"
        "   subcontrol-position:center center"
        "}"
    );
    
    // Create popup
    m_popup = new BtBookPopup(this);
    connect(m_popup, &BtBookPopup::bookSelected,
            this, &BtBookChooserButton::bookSelected);
}

void BtBookChooserButton::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        // Get books from parent and populate popup
        auto books = m_parent.module()->books();
        m_popup->setBooks(books);
        
        // Position popup below button
        QPoint pos = mapToGlobal(QPoint(0, height()));
        m_popup->popup(pos);
        
        e->accept();
    } else {
        QToolButton::mousePressEvent(e);
    }
}

void BtBookChooserButton::wheelEvent(QWheelEvent* e) {
    int const delta = e->angleDelta().y();
    if (delta == 0) {
        e->ignore();
    } else {
        Q_EMIT stepItem((delta > 0) ? -1 : 1);
        e->accept();
    }
}