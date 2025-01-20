#include "singleItemDropWidget.h"

#include <QMimeData>
#include <QDrag>
#include <QDebug> // For debugging purposes

SingleItemDropWidget::SingleItemDropWidget(QWidget* parent)
	: QWidget(parent) {
	setAcceptDrops(true);
	setFocusPolicy( Qt::NoFocus );
}

void SingleItemDropWidget::dragEnterEvent(QDragEnterEvent* event) {
	if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
		event->acceptProposedAction();
	} else {
		event->ignore(); // Ignore other types of data
	}
}

void SingleItemDropWidget::dropEvent(QDropEvent* event) {
	if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
		// Extract text from QTreeWidget's MIME data
		QByteArray encodedData = event->mimeData()->data("application/x-qabstractitemmodeldatalist");
		QDataStream stream(&encodedData, QIODevice::ReadOnly);

		// Decode the data
		while (!stream.atEnd()) {
			int row, col;
			QMap<int, QVariant> roleDataMap;
			stream >> row >> col >> roleDataMap;

			// Assuming Qt::DisplayRole contains the text
			if (roleDataMap.contains(Qt::DisplayRole)) {
				event->acceptProposedAction();
				emit itemAccepted(roleDataMap[Qt::DisplayRole].toString());
				break;
			}
		}
	} else {
		event->ignore();
	}
}

// void SingleItemDropWidget::mousePressEvent(QMouseEvent* event) {
// 	if (!currentItemText.isEmpty() && event->button() == Qt::LeftButton) {
// 		// Start a drag operation with the current item
// 		QMimeData* mimeData = new QMimeData;
// 		mimeData->setText(currentItemText);

// 		QDrag* drag = new QDrag(this);
// 		drag->setMimeData(mimeData);

// 		Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
// 		if (dropAction == Qt::MoveAction) {
// 			// Clear the item after it has been moved
// 			clearItem();
// 			update(); // Update the widget to reflect the cleared state
// 		}
// 	}
// }
