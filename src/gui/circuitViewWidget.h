#ifndef logicGridWindow_h
#define logicGridWindow_h

#include <QApplication>
#include <QToolButton>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QComboBox>
#include <QPainter>
#include <QWidget>
#include <QTimer>

#include "circuitView/circuitView.h"
#include "gpu/renderer/vulkanRenderer.h"
#include "gpu/vulkanQtWindow.h"
#include "computerAPI/circuits/circuitFileManager.h"
#include "ui_circuitViewUi.h"
#include "util/vec2.h"

class CircuitViewWidget : public QWidget {
	Q_OBJECT
public:
    CircuitViewWidget(QWidget* parent, Ui::CircuitViewUi* ui, CircuitFileManager* fileManager);

	// vulkan
	void createVulkanWindow();
	void destroyVulkanWindow();

	// setup
	inline CircuitView<VulkanRenderer>* getCircuitView() { return &circuitView; }
	void setSimState(bool state);
	void simUseSpeed(Qt::CheckState state);
	void setSimSpeed(double speed);

protected:
	// events overrides
	void showEvent(QShowEvent* event) override;
	void hideEvent(QHideEvent* event) override;
	void paintEvent(QPaintEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void enterEvent(QEnterEvent* event) override;
	void leaveEvent(QEvent* event) override;
	bool event(QEvent* event) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dropEvent(QDropEvent* event) override;

private:
	void save();
	void load(const QString& filePath);

	// utility functions
	inline Vec2 pixelsToView(QPointF point) { return Vec2((float)point.x() / (float)rect().width(), (float)point.y() / (float)rect().height()); }
	inline bool insideWindow(const QPoint& point) const { return point.x() >= 0 && point.y() >= 0 && point.x() < size().width() && point.y() < size().height(); }
	inline float getPixelsWidth() { return (float)rect().width(); }
	inline float getPixelsHeight() { return (float)rect().height(); }

	CircuitView<VulkanRenderer> circuitView;
    CircuitFileManager* fileManager;

	// update loop
	QTimer* updateLoopTimer;
	const float updateInterval = 0.008f;
	void updateLoop();

	// framerate statistics
	std::list<float> pastFrameTimes;
	const int numTimesInAverage = 20;

	// vulkan
	bool vulkanWindowOpen = false;
	VulkanQtWindow* vulkanWindow;
	QWidget* vulkanWindowWrapper;

	// settings (temp)
	bool mouseControls;

	QComboBox* circuitSelector;
	QComboBox* evaluatorSelector;
};

#endif /* logicGridWindow_h */
