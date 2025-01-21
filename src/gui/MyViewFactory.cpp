/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "MyViewFactory.h"

#include <kddockwidgets/qtwidgets/views/Separator.h>
#include <kddockwidgets/qtwidgets/views/TitleBar.h>
#include <kddockwidgets/core/TitleBar.h>

#include <QApplication>
#include <QPainter>

// Inheriting from SeparatorWidget instead of Separator as it handles moving and mouse cursor
// changing
class MySeparator : public KDDockWidgets::QtWidgets::Separator {
public:
	explicit MySeparator(KDDockWidgets::Core::Separator* controller, KDDockWidgets::Core::View* parent)
		: KDDockWidgets::QtWidgets::Separator(controller, parent) { }

	~MySeparator() override = default;

	void paintEvent(QPaintEvent*) override {
		QPainter p(this);
		p.fillRect(QWidget::rect(), Qt::gray);
	}
};

KDDockWidgets::Core::View* CustomWidgetFactory::createSeparator(KDDockWidgets::Core::Separator* controller, KDDockWidgets::Core::View* parent) const {
	return new MySeparator(controller, parent);
}
