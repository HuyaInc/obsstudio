#include "NimoObsProxy.h"
#include "obs-app.hpp"
#include "qt-wrappers.hpp"
#include "window-basic-main.hpp"

#include <WinUser.h>
#include <QJsonDocument>
#include <QJsonObject>

namespace
{
	OBSApp *GetOBSApp()
	{
		return App();
	}

	OBSBasic *GetOBSBasic()
	{
		return static_cast<OBSBasic *>(GetOBSApp()->GetMainWindow());
	}
}
//unsigned int NimoObsProxy::m_hWnd = 0;
//HWND NimoObsProxy::m_hWnd = NULL;
NimoObsProxy* NimoObsProxy::m_pInstance = NULL;

NimoObsProxy* NimoObsProxy::Instance()
{
	if (m_pInstance == nullptr)
		m_pInstance = new NimoObsProxy();
	return m_pInstance;
}

void NimoObsProxy::Init()
{
	m_heartTimer = new QTimer(this);
	connect(m_heartTimer, SIGNAL(timeout()), this, SLOT(SendHeart()));
	m_heartTimer->start(3000);
}

void NimoObsProxy::SendMessage2(const QString &str)
{
	QString str2 = QString("tcj: SendMessage2 obs ---  m_hWnd=%1 str=%2 ")
			       .arg((quint64)m_hWnd)
			       .arg(str);

	OutputDebugStringA(str2.toStdString().c_str());

	if (m_hWnd == 0)
		return;

	//wchar_t *szSendBuf = new wchar_t[MAX_PATH];
	//int s1 = str.toWCharArray(szSendBuf);

	//COPYDATASTRUCT CopyData;
	//CopyData.dwData = 1;
	//CopyData.cbData = MAX_PATH*2;
	//CopyData.lpData = szSendBuf;

	QByteArray ba2;
	ba2.append(str); 
	char *szSendBuf = ba2.data();

	COPYDATASTRUCT CopyData;
	CopyData.dwData = 1;
	CopyData.cbData = strlen(szSendBuf) + 1;
	szSendBuf[strlen(szSendBuf)] = '\0';
	CopyData.lpData = szSendBuf;

	::SendMessage(m_hWnd, WM_COPYDATA, (WPARAM)m_hWnd, (LPARAM)&CopyData);
}

void NimoObsProxy::_changeStreamSetting()
{
	auto basic = GetOBSBasic();
	auto service = basic->GetService();

	obs_service_t *oldService = service;
	OBSData hotkeyData = obs_hotkeys_save_service(oldService);
	obs_data_release(hotkeyData);

	OBSData settings = obs_data_create();
	obs_data_release(settings);

	obs_data_set_string(settings, "server", QT_TO_UTF8(QString("111")));
	obs_data_set_bool(settings, "use_auth", false);

	obs_data_set_string(settings, "key", QT_TO_UTF8(QString("123")));

	OBSService newService = obs_service_create(
		"rtmp_custom", "nimo_service", settings, hotkeyData);
	obs_service_release(newService);

	if (!newService)
		return;

	basic->SetService(newService);
	basic->SaveService();
}

void NimoObsProxy::SendObsReady(qint64 hwnd) {
	static bool bCreate = false;
	if (bCreate == false) {
		bCreate = true;
		QJsonObject obj;
		obj.insert("sWnd", QString("%1").arg(hwnd));

		QJsonObject root;
		root.insert("sUri", kOBS_Ready);
		root.insert("objData", obj);

		QJsonDocument builddoc;
		builddoc.setObject(root);
		QString strBuildJson(builddoc.toJson(QJsonDocument::Compact));
		SendMessage2(strBuildJson);

		QString str = QString("tcj: obs SendObsReady ---  hwnd=%1 ").arg(hwnd);
		OutputDebugStringA(str.toStdString().c_str());
	}
}

void NimoObsProxy::SendHeart()
{
	QJsonObject obj;
	obj.insert("iRet", 0);

	QJsonObject root;
	root.insert("sUri", kOBS_HeartBeat);
	root.insert("objData", obj);

	QJsonDocument builddoc;
	builddoc.setObject(root);
	QString strBuildJson(builddoc.toJson(QJsonDocument::Compact));

	SendMessage2(strBuildJson);
}

void NimoObsProxy::SendClickStartStream(int fps, int cx, int cy, int ocx,
					int ocy, int audioBitrate,
					int videoBitrate,
					const QString &sEncoder,
					const QString &sServer,
					const QString &sKey)
{
	QJsonObject obj;
	obj.insert("iFPSInt", fps);
	obj.insert("iBaseCX", cx);
	obj.insert("iBaseCY", cy);
	obj.insert("iOutputCX", ocx);
	obj.insert("iOutputCY", ocy);
	obj.insert("iAudioBitrate", audioBitrate);
	obj.insert("iVideoBitrate", videoBitrate);
	obj.insert("sEncoder", sEncoder);
	obj.insert("sServer", sServer);
	obj.insert("sKey", sKey);

	QJsonObject root;
	root.insert("sUri", kOBS_ClickStartStream);
	root.insert("objData", obj);

	QJsonDocument builddoc;
	builddoc.setObject(root);
	QString strBuildJson(builddoc.toJson(QJsonDocument::Compact));

	SendMessage2(strBuildJson);
}

void NimoObsProxy::SendClickStopStream()
{
	QJsonObject obj;
	obj.insert("iCode", 0);

	QJsonObject root;
	root.insert("sUri", kOBS_ClickStopStream);
	root.insert("objData", obj);

	QJsonDocument builddoc;
	builddoc.setObject(root);
	QString strBuildJson(builddoc.toJson(QJsonDocument::Compact));

	SendMessage2(strBuildJson);
}

void NimoObsProxy::SendStreamingStart(const QString &sServer)
{
	QJsonObject obj;	
	obj.insert("sServer", sServer);	

	QJsonObject root;
	root.insert("sUri", kOBS_StreamingStart);
	root.insert("objData", obj);

	QJsonDocument builddoc;
	builddoc.setObject(root);
	QString strBuildJson(builddoc.toJson(QJsonDocument::Compact));

	SendMessage2(strBuildJson);
}

void NimoObsProxy::SendStreamingStop(int iCode, const QString &sDesc)
{
	QJsonObject obj;
	obj.insert("iCode", iCode);
	obj.insert("sErrorDesc", sDesc);

	QJsonObject root;
	root.insert("sUri", kOBS_StreamingStop);
	root.insert("objData", obj);

	QJsonDocument builddoc;
	builddoc.setObject(root);
	QString strBuildJson(builddoc.toJson(QJsonDocument::Compact));

	SendMessage2(strBuildJson);
}

void NimoObsProxy::DoNativeMsgProc(const QString &str)
{
	QJsonDocument root_Doc = QJsonDocument::fromJson(str.toUtf8());
	QJsonObject jsonProtocal = root_Doc.object();
	QString sUri = jsonProtocal.value("sUri").toString();
	QJsonObject objValue = jsonProtocal.value("objData").toObject();

	if (sUri == kEXE_ClickStartStream) {
		DoExeClickStartStream(objValue);
	} else if (sUri == kEXE_ClickStopStream) {
		DoExeClickStopStream(objValue);
	}

}

void NimoObsProxy::DoExeClickStartStream(const QJsonObject &obj)
{
	QString sServer = obj.value("sServer").toString();
	QString sKey = obj.value("sKey").toString();

	auto basic = GetOBSBasic();
	auto service = basic->GetService();

	obs_service_t *oldService = service;
	OBSData hotkeyData = obs_hotkeys_save_service(oldService);
	obs_data_release(hotkeyData);

	OBSData settings = obs_data_create();
	obs_data_release(settings);

	obs_data_set_string(settings, "server", QT_TO_UTF8(sServer));
	obs_data_set_bool(settings, "use_auth", false);

	obs_data_set_string(settings, "key", QT_TO_UTF8(sKey));

	OBSService newService = obs_service_create(
		"rtmp_custom", "nimo_service", settings, hotkeyData);
	obs_service_release(newService);

	if (!newService)
		return;

	basic->SetService(newService);
	basic->SaveService();


}

void NimoObsProxy::DoExeClickStopStream(const QJsonObject &obj) {
	if (m_pObsBasic != nullptr) {
		m_pObsBasic->StopObsStream();
	}
}
