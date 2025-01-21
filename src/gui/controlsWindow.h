#ifndef controlsWindow_h
#define controlsWindow_h

#include <QWidget>

#include "backend/container/block/blockDefs.h"

namespace Ui {
	class Controls;
};

class ControlsWindow : public QWidget {
	Q_OBJECT
public:
	ControlsWindow(QWidget* parent = nullptr);
	~ControlsWindow();

private:
	void updateSimState(bool state);
	void updateSimUseSpeed(Qt::CheckState state);
	void updateSimSpeed(double speed);

	Ui::Controls* ui;

signals:
	void setSimState(bool state);
	void simUseSpeed(Qt::CheckState state);
	void setSimSpeed(double speed);
};


#endif /* controlsWindow_h */
