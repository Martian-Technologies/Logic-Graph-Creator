#ifndef UNDOWINDOW_H
#define UNDOWINDOW_H

#include <QDialog>
#include <QWidget>

class UndoWindow : public QDialog {
public:
	UndoWindow(QWidget* parent = nullptr);


private:
	void setupUI();

	void showGates();
	void addGate();
	void removeGate();

	// select branch, then press n or p to go through that branch for what you want to see
	void selectBranch();
	void nextNode();
	void prevNode();

private:
	QWidget* parent;
};

#endif
