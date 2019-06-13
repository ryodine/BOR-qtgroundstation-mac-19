#ifndef ROVER_COMM_H
#define ROVER_COMM_H

#include <thread>
#include "SerialPort.h"
#include "mainwindow.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace Houston {
////////////////////////////////
class Message {
public:
    Message() {};
    Message(std::vector<char> &raw);

    void addHeader(std::string key, std::string value) { m_headers.insert(std::make_pair(key, value)); };
    bool hasheader(std::string key) { return m_headers.find(key) != m_headers.end(); }
    std::string getHeader(std::string key) { return m_headers.at(key); };

    void setBody(std::vector<char> body) { m_body = body; m_bodyString = std::string(m_body.begin(), m_body.end()); };
    void setBody(std::string body) { m_bodyString = body; m_body = std::vector<char>(m_bodyString.begin(), m_bodyString.end()); }

    std::vector<char> getBodyBytes() { return m_body; };
    std::string getBodyString() { return m_bodyString; };

    std::vector<char> raw();
    std::vector<char> rawActionOnlyMessage();

private:
    std::unordered_map<std::string, std::string> m_headers;
    std::vector<char> m_body;
    std::string m_bodyString;

};
////////////////////////////////
class Communication {
public:
    Communication(std::string port, int baudrate);
    ~Communication();

    // Status
    int secsSinceLastRead() { return time(NULL) - m_serial->getTimeOfLastRead(); }
    bool hasStatusMessage();
    Message lastStatusMessage();
    double getRxPercent() { return m_serial->getPercentRecieved(); };

    // API
    void sendMessage(Message m);
    bool getMessage(Message& m);
    void scheduleUICommand(Ui::GuiButtons btn) { std::lock_guard<std::mutex> lock(m_transactionMutex); m_commandedTransaction = btn; };
    bool isProcessingCommand() { std::lock_guard<std::mutex> lock(m_transactionMutex); return m_commandedTransaction != Ui::NONE; };
    Message getResultOfUICommand() { std::lock_guard<std::mutex> lock(m_transactionMutex); return m_transactionResult; }
    
private:
    void threadfunc();

    bool runThread = true;
    std::unique_ptr<IO::SerialPort> m_serial;
    std::thread m_thread;

    std::mutex m_messageTransactionMutex;

    std::mutex m_statusMutex;
    Message m_lastStatusMessage;
    bool m_hasStatus;

    std::mutex m_transactionMutex;
    Ui::GuiButtons m_commandedTransaction = Ui::NONE;
    Message m_transactionResult;
};
}

#endif