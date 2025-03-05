#ifndef FORM_MANAGER_H
#define FORM_MANAGER_H

#include <QWidget>
#include <QObject>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>

class FormManager : public QWidget {
	Q_OBJECT 
public:
	FormManager(QWidget* parent = nullptr);
	
	void setForm(const std::string& formType);
	const std::vector<std::pair<std::string, std::string>>& getDataEntry() const { return dataEntry; }

protected:
	// should only handle keyevents
	bool eventFilter(QObject* obj, QEvent* event) {
		QLineEdit* editor = qobject_cast<QLineEdit*>(obj);
		if (editor && event->type() == QEvent::KeyPress) {
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

			// special keys
			if ((keyEvent->key() == Qt::Key_Delete || keyEvent->key() == Qt::Key_Backspace) && !editor->text().isEmpty()) {
				logWarning("backspace todo formmanager");
				return QWidget::eventFilter(obj, event); 
			} else if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
				editor->clearFocus();
				logInfo(editor->text().toStdString());
				return QWidget::eventFilter(obj, event); 
			} else if (keyEvent->key() == Qt::Key_Escape) {
				editor->clearFocus();
				editor->setText("");
				return QWidget::eventFilter(obj, event);
			}

			// general keys
			QString keyPressed;
			if (keyEvent->key() == Qt::Key_Control) {
				keyPressed = QString("ctrl");
			} else if (keyEvent->key() == Qt::Key_Tab) {
				keyPressed = QString("tab");
			} else if (keyEvent->key() == Qt::Key_CapsLock) {
				keyPressed = QString("caps");
			} else {
				keyPressed = keyEvent->text();
			}

			// register keys for lineedit
			if (!editor->text().isEmpty()) {
				editor->setText(editor->text() + " + " + keyPressed);
			} else {	
				editor->setText(keyPressed);
			}
		}

		return QWidget::eventFilter(obj, event); 
	}
private:
	void clearForm();

	// form generation
	QWidget* generateFormWidget(const std::string& preferenceType, const std::vector<std::string> & itemization);
	QHBoxLayout* generateKeybindForm(const std::string& itemization);

	void updateConfig(const std::string& key, const std::string& value);

private:
	QVBoxLayout* form;

	std::vector<std::pair<std::string, std::string>> dataEntry;
};

#endif
