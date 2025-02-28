#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

#include "toolCell.h"

ToolCell::ToolCell(QWidget* parent, int size) : QToolButton(parent) {
	setCheckable(true);
	// setIcon(QIcon(":/toolAndBlockIcons/defaultIcon.png"));
	setMinimumSize(size, size);
	setMaximumSize(size, size);
	setIconSize(QSize(size-16, size-16));
	setAcceptDrops(true);
	setFocusPolicy(Qt::NoFocus);
}

void ToolCell::dragEnterEvent(QDragEnterEvent* event) {
	if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
		event->acceptProposedAction();
	} else {
		event->ignore(); // Ignore other types of data
	}
}

void ToolCell::dropEvent(QDropEvent* event) {
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
				QString value = roleDataMap[Qt::DisplayRole].toString();
				if (value != "NONE") {
					setToolTip(value);
				}
				setIcon(iconLoader.getIcon(value.toStdString()));
				emit itemAccepted(value);
				break;
			}
		}
	} else {
		event->ignore();
	}
}
