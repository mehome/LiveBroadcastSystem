#include "XAudioRecord.h"
#include <QAudioInput>
#include <iostream>
#include <list>
#include <QMutex>

using namespace std;
class CXAudioRecord :public XAudioRecord
{
public:
	void run()
	{
		cout << "进入音频录制线程" << endl;
		//一次读取一帧音频的字节数
		int readSize = nbSamples*channels*sampleByte;
		char *buf = new char[readSize];
		while (!isExit)
		{
			//读取已经录制的音频
			//一次读取一帧音频
			if (input->bytesReady() < readSize)
			{
				QThread::msleep(1);
				continue;
			}
			
			int size = 0;
			while (size != readSize)
			{
				//io->read(buf, sizeof(buf));
				int len = io->read(buf + size, readSize - size);
				if (len < 0)break;
				size += len;
			}
			if (size != readSize)
			{
				continue;
			}
			long long pts = GetCurTime();
			//已经读取一帧音频
			Push(XData(buf,readSize,pts));
		}
		delete buf;
		cout << "退出音频录制线程" << endl;
	}
	bool Init()
	{
		Stop();
		//1 qt音频开始录制
		QAudioFormat fmt;
		fmt.setSampleRate(sampleRate);
		fmt.setChannelCount(channels);
		fmt.setSampleSize(sampleByte * 8);
		fmt.setCodec("audio/pcm");
		fmt.setByteOrder(QAudioFormat::LittleEndian);
		fmt.setSampleType(QAudioFormat::UnSignedInt);
		QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
		if (!info.isFormatSupported(fmt))
		{
			cout << "Audio format not support" << endl;
			fmt = info.nearestFormat(fmt);
		}
		input = new QAudioInput(fmt);
		//开始录制音频
		io = input->start();
		if (!io) {
			return false;
		}
		return true;
	}
	void Stop()
	{
		//先把线程退出
		XDataThread::Stop();
		if (input)
		{
			input->stop();
		}
		if (io)
		{
			io->close();
		}
		input = NULL;
		io = NULL;
	}
	QAudioInput *input = NULL;
	QIODevice *io = NULL;
};

XAudioRecord *XAudioRecord::Get(XAUDIOTYPE type, unsigned char index)
{
	//效率高的话会用索引创建，这里暂时这样
	static CXAudioRecord record[255];
	return &record[index];
}

XAudioRecord::XAudioRecord()
{

}


XAudioRecord::~XAudioRecord()
{
}
