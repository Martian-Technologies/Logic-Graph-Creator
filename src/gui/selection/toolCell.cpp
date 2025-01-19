#include <QMimeData>

#include "toolCell.h"

ToolCell::ToolCell(QWidget* parent) : QToolButton(parent) {
	setCheckable(true);
	// setIcon(QIcon(":/toolAndBlockIcons/defaultIcon.png"));
	setMinimumSize(48, 48);
	setMaximumSize(48, 48);
	setIconSize(QSize(32, 32));
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
				setIcon(iconLoader.getIcon(value.toStdString()));
				emit itemAccepted(value);
				break;
			}
		}
	} else {
		event->ignore();
	}
}
