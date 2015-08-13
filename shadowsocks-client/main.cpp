﻿#include <algorithm>
#include "mainform.h"
#include <QApplication>
#include <QTextCodec>
#include "libuvpp.h"
#include "tcpserver.h"
#include <botan/botan.h>
#include <array>
#include <QDateTime>
#include <DbgHelp.h>
#include <time.h>
#include "botan_wrapper.h"

#pragma execution_character_set("utf-8")

#pragma comment(lib, "Dbghelp.lib")

LONG WINAPI MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
    wchar_t szProgramPath[MAX_PATH] = {0};
    if(GetModuleFileName(NULL, szProgramPath, MAX_PATH))
    {
           LPTSTR lpSlash = wcsrchr(szProgramPath, '\\');
           if(lpSlash)
           {
               *(lpSlash + 1) = '\0';
           }
    }
    wchar_t szDumpFile[MAX_PATH] = {0};
    swprintf_s(szDumpFile, MAX_PATH, L"%s%d.dmp", szProgramPath, time(NULL));

    HANDLE lhDumpFile = CreateFile(szDumpFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL ,NULL);
    MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
    loExceptionInfo.ExceptionPointers = ExceptionInfo;
    loExceptionInfo.ThreadId = GetCurrentThreadId();
    loExceptionInfo.ClientPointers = true;

    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), lhDumpFile, MiniDumpWithIndirectlyReferencedMemory, &loExceptionInfo, NULL, NULL);
    CloseHandle(lhDumpFile);

    return EXCEPTION_EXECUTE_HANDLER;
}

void InstallDump()
{
    SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
}

class Buffer
{
public:
    explicit Buffer(QByteArray *data):
        m_data(data),
        m_pos(0)
    {

    }

    int remain()
    {
        return m_data->size() - m_pos;
    }

    QByteArray read(int n)
    {
        int pos = m_pos + n;
        if (pos>m_data->size())
        {
            pos = m_data->size();
        }
        QByteArray tmp = QByteArray::fromRawData(m_data->constData()+m_pos, pos-m_pos);
        m_pos = pos;
        return tmp;
    }

private:
    QByteArray *m_data;
    int m_pos;
};

int gSessionCount = 0;
class Session
{
public:
    Session(const uv::ConnectionPtr &conn, const QByteArray& data):
        m_local(conn),
        m_tcp(conn->loop()),
        m_remoteConnected(false)
    {
        gSessionCount++;
        qDebug()<<"Session Constructor("<<this<<")"<<gSessionCount;
        localMessage(data);
        conn->messageCallback = std::bind(&Session::localMessage, this, std::placeholders::_2);
        m_tcp.connect("45.62.109.185", 443, std::bind(&Session::remoteConnected, this, std::placeholders::_1));
        m_tcp.onClose(std::bind(&Session::onRemoteClosed, this));
    }

    ~Session()
    {
        gSessionCount--;
        qDebug()<<"Session Destructor("<<this<<")"<<gSessionCount;
    }

private:
    uv::Tcp m_tcp;
    QByteArray m_dataToWrite;
    uv::ConnectionPtr m_local;
    Botan::Encryptor m_encryptor;
    bool m_remoteConnected;

    void localMessage(const QByteArray &data)
    {
        if (data.isEmpty()) return;
        //qDebug()<<"localMessage size="<<data.length();
        QByteArray output = m_encryptor.encrypt(data);
        if (m_remoteConnected)
        {
            //qDebug()<<"WRITE TO REMOTE";
            m_tcp.write(output, nullptr);
        }
        else{
            m_dataToWrite.append(output);
        }
    }

    void remoteConnected(bool connected)
    {
        if (connected)
        {
            qDebug()<<"Session remote connected("<<this<<")";
            m_remoteConnected = true;
            if (!m_dataToWrite.isEmpty())
            {
                m_tcp.write(m_dataToWrite, nullptr);
                m_dataToWrite.clear();
            }
            m_tcp.read_start(std::bind(&Session::remoteMessage, this, std::placeholders::_1));
        }
    }

    void remoteMessage(const QByteArray &data)
    {
        QByteArray output = m_encryptor.decrypt(data);
        //qDebug()<<"remoteMessage size="<<data.size()<<" outputsize="<<output.size();
        //qDebug()<<output;
        m_local->write(output);

    }

    void onRemoteClosed()
    {
        qDebug()<<"Session onRemoteClosed("<<this<<")";
        m_local->shutdown();
    }
};

class AuthSession
{
public:
    AuthSession(const uv::ConnectionPtr &conn):
        m_status(0),
        m_urlLen(0)
    {
        m_buffer.reset(new Buffer(&m_recved));
        conn->messageCallback = std::bind(&AuthSession::localMessage, this, std::placeholders::_1, std::placeholders::_2);
    }

    ~AuthSession()
    {
    }

private:
    int m_status;
    QByteArray m_recved;
    std::unique_ptr<Buffer> m_buffer;
    int m_addrType;
    int m_urlLen;
    void localMessage(const uv::ConnectionPtr &conn, const QByteArray &data)
    {
        if (m_status==0)
        {
            QString hex = data.toHex();
            //qDebug()<<"recv:"<< hex;
            m_status = 1;
            //REQUEST:
            //client连接proxy的第一个报文信息，进行认证机制协商
            //一般是 hex: 05 01 00 即：版本5，1种认证方式，NO AUTHENTICATION REQUIRED(无需认证 0x00)
            //RESPON:
            //proxy从METHODS字段中选中一个字节(一种认证机制)，并向Client发送响应报文:
            //一般是 hex: 05 00 即：版本5，无需认证
            static QByteArray respon = QByteArray::fromHex("0500");
            conn->write(respon);
        }
        else if (m_status==1)
        {
            //05 01 00 03 0e 7777772e676f6f676c652e636f6d 01bb
            m_recved.append(data);
            if (m_urlLen==0)
            {
                if (m_buffer->remain()<5)
                {
                    return;
                }
                QByteArray d = m_buffer->read(5);
                m_addrType = d[3];
                m_urlLen = d[4];
                qDebug()<<"m_addrType="<<m_addrType<<"m_urlLen="<<m_urlLen;
            }
            if (m_buffer->remain()<m_urlLen+2)
            {
                return;
            }
            QString aurl = m_buffer->read(m_urlLen);
            qDebug()<<"go to session:"<<aurl;

            m_status = 2;

            static const char res [] = { 5, 0, 0, 1, 0, 0, 0, 0, 16, 16 };
            static const QByteArray response(res, 10);
            conn->write(response);

            std::shared_ptr<Session> session(new Session(conn, m_recved.right(m_recved.length()-3)));
            conn->context = session;
        }
    }
};


void runThread()
{
    uv::Thread* thread = new uv::Thread;
    thread->create([]()
    {
        uv::Loop loop;
        uv::TcpServer server(&loop);
        server.connectionCallback = [](const uv::ConnectionPtr &conn)
        {
            std::shared_ptr<AuthSession> authSession(new AuthSession(conn));
            conn->context = authSession;
        };

        qDebug()<<"listen:"<< server.listen("0.0.0.0", 1081);
        loop.run();
    });

}

int main(int argc, char *argv[])
{
    InstallDump();
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
    QApplication a(argc, argv);
    QFont f = a.font();
    f.setPointSize(9);
    a.setFont(f);

    MainForm w;
    w.show();
    runThread();


    return a.exec();
}
