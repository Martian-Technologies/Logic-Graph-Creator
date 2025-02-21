#include <QGridLayout>

#include "dynamicGridWidget.h"

DynamicGridWidget::DynamicGridWidget(QWidget* parent, int minSize)
	: QWidget(parent), gridLayout(new QGridLayout(this)), minSize(minSize) {
	gridLayout->setSpacing(0); 
	gridLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(gridLayout);
}

void DynamicGridWidget::addWidget(QWidget* widget) {
	if (widget) {
		childWidgets.push_back(widget);
		gridLayout->addWidget(widget); // Temporarily add it
		updateGrid();                 // Rearrange the grid
	}
}

void DynamicGridWidget::resizeEvent(QResizeEvent* event) {
	QWidget::resizeEvent(event);
	updateGrid();
}

void DynamicGridWidget::updateGrid() {
	int widgetWidth = width();  // Current width of the widget
	int minWidgetWidth = minSize;  // Assumed minimum widget width; can adjust or make configurable
	int columns = widgetWidth / minWidgetWidth;
	if (columns < 1) columns = 1; // Ensure at least one column

	// Clear the existing layout
	while (QLayoutItem* item = gridLayout->takeAt(0)) {
		delete item;
	}

	// Re-add all widgets to the grid layout
	int row = 0, col = 0;
	for (QWidget* widget : childWidgets) {
		gridLayout->addWidget(widget, row, col);
		col++;
		if (col >= columns) {
			col = 0;
			row++;
		}
	}
}
