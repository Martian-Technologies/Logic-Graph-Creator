#include "gui/preferences/tabs.h"

void Tabs::Tabs(QWidget* parent) : QWidget(parent) {
    QScrollArea* scrollArea = new QScrollArea( dlg );
    scrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    scrollArea->setWidgetResizable( true );
    scrollArea->setGeometry( 10, 10, 200, 200 );

    QWidget *widget = new QWidget();
    scrollArea->setWidget( widget );

    QVBoxLayout *layout = new QVBoxLayout();
    widget->setLayout( layout );

    for (int i = 0; i < 10; i++) {
        QPushButton *button = new QPushButton( QString( "%1" ).arg( i ) );
        layout->addWidget( button );
    }
}
