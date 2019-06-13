#include "SerialPort.h"
#include <unistd.h>
#include <vector>

using namespace Houston::IO;

static inline void closeFdOnError(int fd) { if (fd != -1) close(fd); return; }

SerialPort::SerialPort(std::string file, int baudrate) {
    // Open the serial port read/write, with no controlling terminal, and don't wait for a connection.
    // The O_NONBLOCK flag also causes subsequent I/O on the device to be non-blocking.
    // See open(2) <x-man-page://2/open> for details.
    m_fd = open(file.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (m_fd == -1) {
        printf("Error opening serial port %s - %s(%d).\n",
               file.c_str(), strerror(errno), errno);
        closeFdOnError(m_fd);
    }
    fcntl(m_fd, F_SETFL, 0);
    
    // Note that open() follows POSIX semantics: multiple open() calls to the same file will succeed
    // unless the TIOCEXCL ioctl is issued. This will prevent additional opens except by root-owned
    // processes.
    // See tty(4) <x-man-page//4/tty> and ioctl(2) <x-man-page//2/ioctl> for details.
    
    if (ioctl(m_fd, TIOCEXCL) == -1) {
        printf("Error setting TIOCEXCL on %s - %s(%d).\n",
               file.c_str(), strerror(errno), errno);
        closeFdOnError(m_fd);
    }
    
    // Now that the device is open, clear the O_NONBLOCK flag so subsequent I/O will block.
    // See fcntl(2) <x-man-page//2/fcntl> for details.
    
    if (fcntl(m_fd, F_SETFL, 0) == -1) {
        printf("Error clearing O_NONBLOCK %s - %s(%d).\n",
               file.c_str(), strerror(errno), errno);
        closeFdOnError(m_fd);
    }
    
    // Get the current options and save them so we can restore the default settings later.
    if (tcgetattr(m_fd, &m_origTermOpts) == -1) {
        printf("Error getting tty attributes %s - %s(%d).\n",
               file.c_str(), strerror(errno), errno);
        closeFdOnError(m_fd);
    }
    
    // The serial port attributes such as timeouts and baud rate are set by modifying the termios
    // structure and then calling tcsetattr() to cause the changes to take effect. Note that the
    // changes will not become effective without the tcsetattr() call.
    // See tcsetattr(4) <x-man-page://4/tcsetattr> for details.
    
    m_termOpts = m_origTermOpts;
    
    // Set raw input (non-canonical) mode, with reads blocking until either a single character
    // has been received or a one second timeout expires.
    // See tcsetattr(4) <x-man-page://4/tcsetattr> and termios(4) <x-man-page://4/termios> for details.
    
    cfmakeraw(&m_termOpts);
    m_termOpts.c_cc[VMIN] = 0;
    m_termOpts.c_cc[VTIME] = 10;
    
    // The baud rate, word length, and handshake options can be set as follows:
    
    cfsetspeed(&m_termOpts, baudrate);		// Set baud
    m_termOpts.c_cflag &= ~PARENB;
    m_termOpts.c_cflag &= ~CSTOPB;
    m_termOpts.c_cflag |= CS8;
    m_termOpts.c_cflag &= ~CRTSCTS;
    m_termOpts.c_cflag |= CREAD | CLOCAL;
    m_termOpts.c_iflag &= ~(IXON | IXOFF | IXANY);
    m_termOpts.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
    m_termOpts.c_oflag &= ~OPOST;
    m_termOpts.c_oflag &= ~ONLCR;
    m_termOpts.c_lflag &= ~ICANON;
    m_termOpts.c_lflag &= ~ECHO;
    m_termOpts.c_lflag &= ~ECHOE;
    m_termOpts.c_lflag &= ~ECHONL;
    m_termOpts.c_lflag &= ~ISIG;
    m_termOpts.c_oflag &= ~ONOEOT;
    m_termOpts.c_oflag &= ~OXTABS;
    
	// The IOSSIOSPEED ioctl can be used to set arbitrary baud rates
	// other than those specified by POSIX. The driver for the underlying serial hardware
	// ultimately determines which baud rates can be used. This ioctl sets both the input
	// and output speed.
	
	speed_t speed = baudrate; // Set 14400 baud
    if (ioctl(m_fd, IOSSIOSPEED, &speed) == -1) {
        printf("Error calling ioctl(..., IOSSIOSPEED, ...) %s - %s(%d).\n",
               file.c_str(), strerror(errno), errno);
    }
    
    // Cause the new options to take effect immediately.
    if (tcsetattr(m_fd, TCSANOW, &m_termOpts) == -1) {
        printf("Error setting tty attributes %s - %s(%d).\n",
               file.c_str(), strerror(errno), errno);
        closeFdOnError(m_fd);
    }
    
    // To set the modem handshake lines, use the following ioctls.
    // See tty(4) <x-man-page//4/tty> and ioctl(2) <x-man-page//2/ioctl> for details.
    
    // Assert Data Terminal Ready (DTR)
    if (ioctl(m_fd, TIOCSDTR) == -1) {
        printf("Error asserting DTR %s - %s(%d).\n",
               file.c_str(), strerror(errno), errno);
    }
    
    // Clear Data Terminal Ready (DTR)
    if (ioctl(m_fd, TIOCCDTR) == -1) {
        printf("Error clearing DTR %s - %s(%d).\n",
               file.c_str(), strerror(errno), errno);
    }
    
    // Set the modem lines depending on the bits set in handshake
    m_handshake = TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR;
    if (ioctl(m_fd, TIOCMSET, &m_handshake) == -1) {
        printf("Error setting handshake lines %s - %s(%d).\n",
               file.c_str(), strerror(errno), errno);
    }
    
}

int SerialPort::write(std::string msg) {
    return ::write(m_fd, msg.c_str(), msg.length());
}

int SerialPort::write(void* buff, int len) {
    return ::write(m_fd, buff, len);
}

int SerialPort::write(std::vector<char> msg) {
    return write(&msg[0], msg.size());
}

bool SerialPort::readUntilEnd(std::vector<char>& buffer) {
    buffer.empty();
    bool seenEnd = false;
    unsigned long lastbyte = time(NULL);
    char read_buf [256];
    memset(&read_buf, '\0', sizeof(read_buf));
    uint content_length = 0;
    uint hdr_bytes = 0;
    m_recPercentage = 0.0;
    while (!seenEnd) {
        if (time(NULL) - lastbyte > 3) {
            printf("[e] Timeout. Assume all data arrived\n");
            break;
        }

        // Header Pre-parsing
        if (content_length == 0 && buffer.size() > 16) {
            std::string eager_header_parser(buffer.begin(), buffer.end());
            int index;
            if ((index = 16 + eager_header_parser.find("Content-Length: ")) != std::string::npos) {
                content_length = atoi(eager_header_parser.substr(index, eager_header_parser.find("\r\n")).c_str());
            }
        }

        if (hdr_bytes == 0 && buffer.size() > 16) {
            std::string eager_header_parser(buffer.begin(), buffer.end());
            int index;
            if ((index = 2 + eager_header_parser.find("\r\n")) != std::string::npos) {
                hdr_bytes = index;
            }
        }
        
        int n = read(m_fd, &read_buf, sizeof(read_buf));
        if (n > 0) {
            lastbyte = time(NULL);
            {
                std::lock_guard<std::mutex> guard(m_lastReadMutex);
                m_lastReadTime = time(NULL);
            }
            buffer.insert(buffer.end(), &read_buf[0], &read_buf[n]);
            if (buffer.size() > 6) {
                int sz = buffer.size();
                seenEnd = (buffer[sz-1] == '\n' && buffer[sz-2] == '\r' &&
                           buffer[sz-3] == '\n' && buffer[sz-4] == '\r' &&
                           buffer[sz-5] == '\n' && buffer[sz-6] == '\r');
            }
        }

        // Update rx percent
        if (content_length > 0) {
            m_recPercentage = (buffer.size() - hdr_bytes) / (content_length * 1.0);
        }
    }
    m_recPercentage = 1.0;
    return seenEnd;
}

int SerialPort::getTimeOfLastRead() {
    std::lock_guard<std::mutex> guard(m_lastReadMutex);
    return m_lastReadTime;
}

SerialPort::~SerialPort() {
    usleep(50000); //Super important for some reason the following code doesnt really wait
    // Block until all written output has been sent from the device.
    // Note that this call is simply passed on to the serial device driver.
	// See tcsendbreak(3) <x-man-page://3/tcsendbreak> for details.
    if (tcdrain(m_fd) == -1) {
        printf("Error waiting for drain - %s(%d).\n",
               strerror(errno), errno);
    }
    
    // Traditionally it is good practice to reset a serial port back to
    // the state in which you found it. This is why the original termios struct
    // was saved.
    if (tcsetattr(m_fd, TCSANOW, &m_origTermOpts) == -1) {
        printf("Error resetting tty attributes - %s(%d).\n",
               strerror(errno), errno);
    }
    printf("[i] Closing serialport\n");
    close(m_fd);
}