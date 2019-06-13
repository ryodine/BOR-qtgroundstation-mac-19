#ifndef SERIALPORT_GUARD_H
#define SERIALPORT_GUARD_H

#include <string>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <paths.h>
#include <termios.h>
#include <sysexits.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/serial/ioss.h>
#include <IOKit/IOBSD.h>
#include <mutex>

namespace Houston {
namespace IO {
class SerialPort {
public:
    SerialPort(std::string file, int baudrate);
    ~SerialPort();

    int write(std::string msg);
    int write(void* buff, int len);
    int write(std::vector<char> msg);
    bool readUntilEnd(std::vector<char>& buffer);

    int getTimeOfLastRead();

    double getPercentRecieved() { return m_recPercentage; };
    
private:
    int m_fd;
    struct termios m_termOpts;
    struct termios m_origTermOpts;
    int m_handshake;

    std::mutex m_lastReadMutex;
    int m_lastReadTime;
    double m_recPercentage = 0.0;


};
}
}

#endif