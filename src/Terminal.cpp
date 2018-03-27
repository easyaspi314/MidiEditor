// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * MidiEditor
 * Copyright (C) 2010  Markus Schwenk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Terminal.h"

#include <QtGlobal>
#include <QByteArray>
#include <QProcess>
#include <QScrollBar>
#include <QStringBuilder>
#include <QStringList>
#include <QTextEdit>
#include <QTimer>

#include "midi/MidiInput.h"
#include "midi/MidiOutput.h"

Terminal *Terminal::_terminal = qnullptr;

Terminal::Terminal(QObject *parent) : QObject(parent) {
    _process = qnullptr;
    _textEdit = new QTextEdit();
    _textEdit->setReadOnly(true);

    _inPort = QString();
    _outPort = QString();
}

void Terminal::initTerminal(const QString &startString, const QString &inPort,
                            const QString &outPort) {
    _terminal = new Terminal();
    _terminal->execute(startString, inPort, outPort);
}

Terminal *Terminal::terminal() {
    return _terminal;
}

void Terminal::writeString(const QString &message) {
    _textEdit->setText(_textEdit->toPlainText() % message % "\n");
    _textEdit->verticalScrollBar()->setValue(
        _textEdit->verticalScrollBar()->maximum());
    qWarning("%s", message.toUtf8().constData());
}

void Terminal::execute(const QString &startString, const QString &inPort,
                        const QString &outPort) {
    _inPort = inPort;
    _outPort = outPort;

    if (!startString.isEmpty()) {
        if (_process) {
            _process->kill();
        }
        _process = new QProcess();

        connect(_process, &QProcess::readyReadStandardOutput,
                this, &Terminal::printToTerminal);
        connect(_process, &QProcess::readyReadStandardError,
                this, &Terminal::printErrorToTerminal);
        connect(_process, &QProcess::started,
                this, &Terminal::processStarted);

        _process->start(startString);
    } else {
        processStarted();
    }
}

ubyte Terminal::retries = 0;
void Terminal::processStarted() {
    // We don't want to loop this forever if we keep failing.
    if (retries > 10) {
        writeString("Connecting to MIDI I/O failed more than 10 times in a row. Giving up.");
        return;
    }
    QStringList inputVariants;
    QString inPort = _inPort;
    inputVariants.append(inPort);
    if (inPort.contains(':')) {
        inPort =  inPort.section(':', 0, 0);
    }
    inputVariants.append(inPort);
    if (inPort.contains('(')) {
        inPort = inPort.section('(', 0, 0);
    }
    inputVariants.append(inPort);

    QStringList outputVariants;
    QString outPort = _outPort;
    outputVariants.append(outPort);
    if (outPort.contains(':')) {
        outPort =  outPort.section(':', 0, 0);
    }
    outputVariants.append(outPort);
    if (outPort.contains('(')) {
        outPort = outPort.section('(', 0, 0);
    }
    outputVariants.append(outPort);

    if (MidiInput::instance()->inputPort().isEmpty() && !_inPort.isEmpty()) {
        writeString("Trying to set Input Port to " % _inPort);
        const QStringList inPorts = MidiInput::instance()->inputPorts();
        for (const QString &portVariant : inputVariants) {
            for (const QString &port : inPorts) {
                if (port.startsWith(portVariant)) {
                    writeString("Found port " % port);
                    MidiInput::instance()->setInputPort(port);
                    _inPort.clear();
                    break;
                }
            }
            if (_inPort.isEmpty()) {
                break;
            }
        }
    }

    if (MidiOutput::instance()->outputPort().isEmpty() && !_outPort.isEmpty()) {
        writeString("Trying to set Output Port to " % _outPort);
        const QStringList outPorts = MidiOutput::instance()->outputPorts();
        for (const QString &portVariant : outputVariants) {
            for (const QString &port : outPorts) {
                if (port.startsWith(portVariant)) {
                    writeString("Found port " % port);
                    MidiOutput::instance()->setOutputPort(port);
                    _outPort.clear();
                    break;
                }
            }
            if (_outPort.isEmpty()) {
                break;
            }
        }
    }

    // if not both are set, try again in 1 second
    if ((MidiOutput::instance()->outputPort().isEmpty() && !_outPort.isEmpty()) ||
            (MidiInput::instance()->inputPort().isEmpty() && !_inPort.isEmpty())) {
        retries++;
        QTimer *timer = new QTimer();
        connect(timer, &QTimer::timeout, this, &Terminal::processStarted);
        timer->setSingleShot(true);
        timer->start(1000);
    }
}

void Terminal::printToTerminal() {
    writeString(QString::fromLocal8Bit(_process->readAllStandardOutput()));
}

void Terminal::printErrorToTerminal() {
    writeString(QString::fromLocal8Bit(_process->readAllStandardError()));
}

QTextEdit *Terminal::console() {
    return _textEdit;
}
