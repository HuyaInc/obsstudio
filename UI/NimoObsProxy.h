#pragma once
#include <QObject>
#include <Windows.h>
#include <QTimer>
#include <window-basic-main.hpp>

class NimoObsProxy : public QObject
{
	Q_OBJECT
public:
	HWND m_hWnd = nullptr;//初始化
	HWND m_hContainer = nullptr; //初始化
	OBSBasic *m_pObsBasic = nullptr;
	const QString kOBS_Ready = "OBS_Ready";
	const QString kOBS_HeartBeat = "OBS_HeartBeat";
	const QString kOBS_ClickStartStream = "OBS_ClickStartStream";
	const QString kOBS_ClickStopStream = "OBS_ClickStopStream";
	const QString kOBS_StreamingStart = "OBS_StreamingStart";
	const QString kOBS_StreamingStop = "OBS_StreamingStop";

	const QString kEXE_ClickStartStream = "EXE_ClickStartStream";
	const QString kEXE_ClickStopStream = "EXE_ClickStopStream";


public slots:
	// 心跳
	void SendHeart();

public:
	// 单例
	static NimoObsProxy *Instance();
	// 初始化
	void Init();	
	// 准备好
	void SendObsReady(qint64 hwnd);
	// 点击开播
	void SendClickStartStream(int fps, int cx, int cy, int ocx, int ocy,
				  int audioBitrate, int videoBitrate,
				  const QString &sEncoder,
				  const QString &sServer, const QString &sKey);
	// 点击停播
	void SendClickStopStream();
	// 流开始
	void SendStreamingStart(const QString &sServer);
	// 流停止
	void SendStreamingStop(int iCode, const QString &sDesc);

	// 处理消息
	void DoNativeMsgProc(const QString &str);
	// 处理开播
	void DoExeClickStartStream(const QJsonObject &obj);
	// 处理停播
	void DoExeClickStopStream(const QJsonObject &obj);



private:
	NimoObsProxy() = default;
	// 发送WM_COPYDATA
	void SendMessage2(const QString &str);
	void _changeStreamSetting();

private:
	static NimoObsProxy *m_pInstance;
	QTimer* m_heartTimer;
};
