#include "RoverCommunicator.h"
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <string>
#include <regex>

using namespace Houston;

/*static void print_buf(const unsigned char *buf, size_t buf_len)
{
    size_t i = 0;
    for(i = 0; i < buf_len; ++i)
    fprintf(stdout, "%02X%s", buf[i],
             ( i + 1 ) % 16 == 0 ? "\r\n" : " " );

}*/

Message::Message(std::vector<char> &raw) {
    std::string str(raw.begin(), raw.end()); 

    int l_idx = 0;
    int idx = 0;
    int bodybreak;
    try {
        str = std::regex_replace(str, std::regex("^[\r\n ]+"), "");
        std::string headers = str.substr(0, (bodybreak = str.find("\r\n\r\n")));
        while ((idx = 2 + headers.find("\r\n", idx, 2)) != std::string::npos) {
            std::string line = headers.substr(l_idx,idx-l_idx-2);
            int colonAt;
            std::string key = line.substr(0, (colonAt = line.find(": ")));
            std::string value = line.substr(colonAt+2);
            addHeader(key, value);
            if (l_idx > idx) break;
            l_idx = idx;
        }

        m_bodyString = str.substr(bodybreak + 4, str.size()-bodybreak-12);
        m_body = std::vector<char>(m_bodyString.begin(), m_bodyString.end());
    } catch (const std::out_of_range& oor) {
        printf("[e] Unparseable message.\n=====[Message below]=====\n%s\n=========================\n", str.c_str());
    }
}

std::vector<char> Message::raw() {
    std::string header_buff;
    for (std::pair<std::string, std::string> element : m_headers) {
        header_buff += element.first + ": " + element.second + "\r\n";
    }
    std::vector<char> out(header_buff.begin(), header_buff.end());
    out.push_back('\r'); out.push_back('\n');
    out.push_back('\r'); out.push_back('\n');
    std::copy(m_body.begin(), m_body.end(), std::back_inserter(out));
    out.push_back('\r'); out.push_back('\n');
    out.push_back('\r'); out.push_back('\n');
    out.push_back('\r'); out.push_back('\n');

    return out;
}

std::vector<char> Message::rawActionOnlyMessage() {
    std::string header_buff;
    for (std::pair<std::string, std::string> element : m_headers) {
        header_buff += element.first + ": " + element.second + "\r\n";
    }
    std::vector<char> out(header_buff.begin(), header_buff.end());
    out.push_back('\r'); out.push_back('\n');
    return out;
}

Communication::Communication(std::string port, int baudrate)
 : m_serial(std::unique_ptr<IO::SerialPort>(new IO::SerialPort(port, baudrate)))
 , m_thread(&Communication::threadfunc, this)
{
    Message statusQuery;
    statusQuery.addHeader("Action", "status");

    m_serial->write(statusQuery.raw());
    std::vector<char> buff;
    m_serial->readUntilEnd(buff);

    Message m(buff);
    if (m.hasheader("Status")) {
        printf("stat: %s\n", m.getHeader("Status").c_str());
    }
    printf("[i] Started communication on %s at %d baud\n", port.c_str(), baudrate);
}

Communication::~Communication() {
    runThread = false;
    m_thread.join();
    printf("[i] Comm worker thread concluded\n");
}

void Communication::threadfunc() {
    while (runThread) {
        usleep(1000000); //1hz
        
        if (m_commandedTransaction != Ui::NONE) {
            switch(m_commandedTransaction) {
                case Ui::PING: {
                    Message out;
                    out.addHeader("Action", "ping");
                    sendMessage(out);
                    Message in;
                    if (getMessage(in)) {
                        printf("Ping reply: %s\n", in.getBodyString().c_str());
                    }
                }; break;
                case Ui::SAMPLE: {
                    Message out;
                    out.addHeader("Action", "soil");
                    sendMessage(out);
                    Message in;
                    if (getMessage(in)) {
                        printf("Soil reply: %s\n", in.getBodyString().c_str());
                    }
                }; break;
                case Ui::STATUS: {
                    Message out;
                    out.addHeader("Action", "status");
                    sendMessage(out);
                    Message in;
                    if (getMessage(in)) {
                        printf("Status reply:\n %s\n", in.getBodyString().c_str());
                    }
                }; break;
                case Ui::TAKE_PHOTO: {
                    Message out;
                    out.addHeader("Action", "photo");
                    sendMessage(out);
                    Message in;
                    if (getMessage(in)) {
                        //printf("Ping photo: %s\n", in.getBodyString().c_str());
                        printf("Got photo\n");
                        //print_buf(const_cast<unsigned char *>(reinterpret_cast<unsigned char *>(in.getBodyBytes().data())), in.getBodyBytes().size());
                        m_transactionResult = in;
                        

                    }
                }; break;
                case Ui::MANUAL_LAND: {
                    Message out;
                    out.addHeader("Action", "land");
                    sendMessage(out);
                    Message in;
                    if (getMessage(in)) {
                        printf("Status reply:\n %s\n", in.getBodyString().c_str());
                    }
                }; break;
                case Ui::ARM_LANDING: {
                    Message out;
                    out.addHeader("Action", "arm");
                    sendMessage(out);
                    Message in;
                    if (getMessage(in)) {
                        printf("Status reply:\n %s\n", in.getBodyString().c_str());
                    }
                }; break;
                case Ui::LEGZERO: {
                    Message out;
                    out.addHeader("Action", "zero");
                    sendMessage(out);
                    Message in;
                    if (getMessage(in)) {
                        printf("Status reply:\n %s\n", in.getBodyString().c_str());
                    }
                }; break;
                default: {
                    printf("Unknown button\n");
                }; break;
            }
            m_commandedTransaction = Ui::NONE;
        }
        // Autoping
        if (secsSinceLastRead() > 5) {
            Message pingQuery;
            pingQuery.addHeader("Action", "status");
            sendMessage(pingQuery);

            Message resp;
            if (getMessage(resp)) {
                std::lock_guard<std::mutex> lock(m_statusMutex);
                m_lastStatusMessage = resp;
            }
        }
    }
}

bool Communication::hasStatusMessage() {
    std::lock_guard<std::mutex> lock(m_statusMutex);
    return m_hasStatus;
}

Message Communication::lastStatusMessage() {
    std::lock_guard<std::mutex> lock(m_statusMutex);
    return m_lastStatusMessage;
}

void Communication::sendMessage(Message m) {
    std::lock_guard<std::mutex> lock(m_messageTransactionMutex);
    m_serial->write(m.rawActionOnlyMessage());
}

bool Communication::getMessage(Message& m) {
    std::lock_guard<std::mutex> lock(m_messageTransactionMutex);
    std::vector<char> buff;
    if (m_serial->readUntilEnd(buff)) {
        m = Message(buff);
        return true;
    }
    return false;
}