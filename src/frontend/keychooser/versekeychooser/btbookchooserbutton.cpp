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
    
    // Main layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    m_mainLayout->setSpacing(0);
    
    // Create scroll area for content
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameStyle(QFrame::Box);
    
    // Content widget for columns
    QWidget* contentWidget = new QWidget();
    scrollArea->setWidget(contentWidget);
    
    // Two-column layout
    m_columnsLayout = new QHBoxLayout(contentWidget);
    m_columnsLayout->setContentsMargins(5, 5, 5, 5);
    m_columnsLayout->setSpacing(10);
    
    // Left and right columns
    m_leftColumn = new QVBoxLayout();
    m_leftColumn->setSpacing(2);
    m_leftColumn->setAlignment(Qt::AlignTop);
    
    m_rightColumn = new QVBoxLayout();
    m_rightColumn->setSpacing(2);
    m_rightColumn->setAlignment(Qt::AlignTop);
    
    m_columnsLayout->addLayout(m_leftColumn);
    m_columnsLayout->addLayout(m_rightColumn);
    
    m_mainLayout->addWidget(scrollArea);
    
    // Set reasonable size
    setFixedSize(400, 350);
    
    // Apply styling
    setStyleSheet(
        "BtBookPopup {"
        "   background-color: palette(window);"
        "   border: 1px solid palette(mid);"
        "}"
    );
}

void BtBookPopup::setBooks(const QStringList& books) {
    clear();
    
    // Distribute books between columns
    bool useLeftColumn = true;
    for (const QString& bookName : books) {
        QPushButton* button = createBookButton(bookName);
        
        if (useLeftColumn) {
            m_leftColumn->addWidget(button);
        } else {
            m_rightColumn->addWidget(button);
        }
        useLeftColumn = !useLeftColumn;
    }
}

void BtBookPopup::popup(const QPoint& pos) {
    move(pos);
    show();
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
    button->setStyleSheet(
        "QPushButton {"
        "   text-align: left;"
        "   padding: 4px 8px;"
        "   border: none;"
        "   background: transparent;"
        "   min-width: 120px;"
        "}"
        "QPushButton:hover {"
        "   background-color: palette(highlight);"
        "   color: palette(highlighted-text);"
        "}"
        "QPushButton:pressed {"
        "   background-color: palette(dark);"
        "}"
    );
    
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