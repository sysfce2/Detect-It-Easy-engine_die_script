// copyright (c) 2019 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#ifndef BINARY_SCRIPT_H
#define BINARY_SCRIPT_H

#include <QObject>
#include <QIODevice>
#include "xbinary.h"

class Binary_Script : public QObject
{
    Q_OBJECT

public:
    explicit Binary_Script(QIODevice *pDevice);
    ~Binary_Script();

public slots:
    qint64 getSize();
    bool compare(QString sSignature,qint64 nOffset=0);
    quint8 readByte(qint64 nOffset);
    quint16 readWord(qint64 nOffset);
    quint32 readDword(qint64 nOffset);
    quint64 readQword(qint64 nOffset);
    QString getString(qint64 nOffset,qint64 nMaxSize=50);
    qint64 findSignature(qint64 nOffset,qint64 nSize,QString sSignature);

private:
    XBinary *pBinary;
};

#endif // BINARY_SCRIPT_H
