#include "fileLoader.h"

#include <QFile>
#include <QString>
#include <QTextStream>

std::vector<char> readFileAsBytes(const std::string& path) {
	QFile file(QString::fromStdString(path));
    file.open(QIODevice::ReadOnly);
	QByteArray qBytes = file.readAll();
	file.close();
	
	std::vector<char> bytes(qBytes.begin(), qBytes.end());
	return bytes;
}
