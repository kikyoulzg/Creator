#pragma once

#include "bkeproject.h"
#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QList>

#define DEBUGPORT 54321
namespace details
{
	enum SocketDataType : int32_t
	{
		UNKNOWN,
		CONNECT_CONFIRM,//data=当前工作目录u16string
		RETURN_SUCCESS,	//datalen=0
		RETURN_FAIL,	//datalen=0
		NEW_BREAKPOINT,	//data=fullfilename:lineNo
		DEL_BREAKPOINT,	//data=fullfilename:lineNo
		NEW_BREAKPOINTONCE,	//data=fullfilename:lineNo
		QUERY_VAR,		//data=variable name expression
		QUERY_SP,		//datalen=2,data=(int32_t)spIndex
		QUERY_SCREEN,	//datalen=0
		QUERY_AUDIO,	//datalen=2, data=(int32_t)audio_index
		STEP_NEXT,		//datalen=0
		STEP_INTO,		//datalen=0
		STEP_OUT,		//datalen=0
		RUN,			//datalen=0
		PAUSE,			//datalen=0
		EXECUTE_BAGEL,	//data=bagel expression
		RET_NOTFOUND,	//datalen=0
		RET_BAGEL,		//data=serialized bagel data
		RET_EXCEPT,		//data=serialized bagel data
		LOG,			//data=[int32]level [u16string]msg
	};
};
typedef details::SocketDataType SocketDataType;

class DebugServer : public QObject
{
	Q_OBJECT

	QWebSocketServer *server;
	QList<QWebSocket *> connections;

public:
	DebugServer(QObject *parent = nullptr);
	~DebugServer();

signals:
	void logReceived(int32_t level, QString log);

public slots:
	void WorkproChanged(BkeProject *);

private slots:
	void onNewConnection();
	void closed();
	void processTextMessage(const QString &message);
	void processBinaryMessage(const QByteArray &message);
	void socketDisconnected();

private:
	int32_t taskcode = 0x100;
	BkeProject *workpro = nullptr;
	void reply(QWebSocket *client, SocketDataType type, int32_t taskmask);
};