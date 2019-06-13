#ifndef SERIAL_COMM_SCAN_H
#define SERIAL_COMM_SCAN_H

#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/serial/ioss.h>
#include <IOKit/IOBSD.h>
#include <string>


namespace Houston {
    namespace IO {
        class USBSerialScanner {
            public:
            USBSerialScanner() {};
            int scan(int vendor, int product, std::string& path);
            private:
            std::string CFStringToString(CFStringRef input);
            std::string GetPropertyString(io_object_t& device, const char* key);
            uint GetPropertyInt(io_object_t& device, const char* key);
            std::string GetDeviceClass(io_object_t& device);
            io_object_t GetParentFTDIDevice(io_object_t& device);
            
        };
    }
}

#endif