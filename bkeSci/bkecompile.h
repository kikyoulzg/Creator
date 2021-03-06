﻿#ifndef BKECOMPILE_H
#define BKECOMPILE_H

#include <QObject>
#include <QProcess>

class BkeCompile : public QObject
{
	Q_OBJECT
public:
	explicit BkeCompile(QObject *parent = 0);
	~BkeCompile();

	void Compile(const QString &dir, bool release = false);
	void CompileLang(const QString &dir, bool release = false);
	QString Result();

signals:
	void CompliteFinish();
	void NewFileReady(int i);
	void CompliteError(QString);

	public slots:
	void StandardOutput();
	void finished(int exitCode);
	void error(QProcess::ProcessError);
private:
	QByteArray result;
	QProcess *cmd;
	QTextCodec *codec;
	QStringList list;

};

#endif // BKECOMPILE_H
