#include "singleItemDropWidget.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDrag>

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
