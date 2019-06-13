#include "SerialScanner.h"

using namespace Houston::IO;

int USBSerialScanner::scan(int v, int p, std::string& path) {
    bool modemFound = false;
    CFMutableDictionaryRef	classesToMatch;
    classesToMatch = IOServiceMatching(kIOSerialBSDServiceValue);
    if (classesToMatch == NULL) {
        printf("IOServiceMatching returned a NULL dictionary.\n");
    }
    else {
        CFDictionarySetValue(classesToMatch,
                             CFSTR(kIOSerialBSDTypeKey),
                             CFSTR(kIOSerialBSDAllTypes));
    
        io_iterator_t io_iterator;
        kern_return_t kr = IOServiceGetMatchingServices(kIOMasterPortDefault, classesToMatch, &io_iterator);
        if (KERN_SUCCESS != kr) {
            printf("IOServiceGetMatchingServices returned %d\n", kr);
            return -1;
        }

        io_object_t modemService;
        CFIndex maxPathSize = 200;
        char bsdPath[maxPathSize];

        while ((modemService = IOIteratorNext(io_iterator)) && !modemFound) {
            CFTypeRef	bsdPathAsCFString;

            bsdPathAsCFString = IORegistryEntryCreateCFProperty(modemService,
                                                                CFSTR(kIODialinDeviceKey),
                                                                kCFAllocatorDefault,
                                                                0);
            if (bsdPathAsCFString) {
                Boolean result;

                result = CFStringGetCString((CFStringRef)bsdPathAsCFString,
                                            bsdPath,
                                            maxPathSize,
                                            kCFStringEncodingUTF8);
                CFRelease(bsdPathAsCFString);
                
                if (result) {
                    IOObjectRetain(modemService);
                    io_object_t parent = GetParentFTDIDevice(modemService);
                    if (GetDeviceClass(parent) == "AppleUSBFTDI") {
                        printf("[i] Found device v:%d, p:%d \n", GetPropertyInt(parent, "idVendor"), GetPropertyInt(parent, "idProduct"));
                        
                        if (GetPropertyInt(parent, "idVendor") == v && GetPropertyInt(parent, "idProduct") == p) {
                            modemFound = true;
                            printf("[i] ==> vendor and product MATCH\n");
                            path = bsdPath;
                        } else {
                            IOObjectRelease(modemService);
                        }
                    } else {
                        IOObjectRelease(modemService);
                    }
                }
            }
            
            // Release the io_service_t now that we are done with it.
            
            (void) IOObjectRelease(modemService);
        }
        
    }
    return modemFound;
}

std::string USBSerialScanner::GetPropertyString(io_object_t& device, const char* key)
{
    std::string result;
    
    CFStringRef propertyName = CFStringCreateWithCString(kCFAllocatorDefault, key, kCFStringEncodingASCII);
    
    if (propertyName == NULL)
    {
        return result;
    }
    
    CFTypeRef propertyValue = IORegistryEntryCreateCFProperty(device, propertyName, kCFAllocatorDefault, 0);
    CFRelease(propertyName);
    
    if (propertyValue == NULL)
    {
        printf("Property %s does not exist", key);
        return result;
    }
    
    if (CFGetTypeID(propertyValue) == CFStringGetTypeID())
    {
        return CFStringToString(static_cast<CFStringRef>(propertyValue));
    }
    else
    {
        printf("Property %s is not a string type", key);
    }
    
    CFRelease(propertyValue);
    return result;
}

uint USBSerialScanner::GetPropertyInt(io_object_t& device, const char* key)
{
    uint result = 0;
    
    CFStringRef propertyName = CFStringCreateWithCString(kCFAllocatorDefault, key, kCFStringEncodingASCII);
    
    if (propertyName == NULL)
    {
        return result;
    }
    
    CFTypeRef propertyValue = IORegistryEntryCreateCFProperty(device, propertyName, kCFAllocatorDefault, 0);
    CFRelease(propertyName);
    
    if (propertyValue == NULL)
    {
        printf("Property %s does not exist", key);
        return result;
    }
    
    if (CFGetTypeID(propertyValue) == CFNumberGetTypeID())
    {
        CFNumberGetValue(static_cast<CFNumberRef>(propertyValue), kCFNumberSInt16Type, &result);
    }
    else
    {
        printf("Property %s is not an integer type", key);
    }
    
    CFRelease(propertyValue);
    return result;
}

std::string USBSerialScanner::CFStringToString(CFStringRef input)
{
    if (input) {
        int len = CFStringGetLength(input)+1; // ASCII, to allow further open() operation

        char* cstr = new char[len];
        Boolean result = CFStringGetCString(input, cstr, len, kCFStringEncodingASCII);
        CFRelease(input);
        
        if (result)
        {
            std::string resultString(cstr, len);
            delete[] cstr;
            return resultString;
        }
        delete[] cstr;
    }
    return "";
}
std::string USBSerialScanner::GetDeviceClass(io_object_t& device)
{
    std::string result;
    io_name_t name;
    kern_return_t kern_result = IOObjectGetClass(device, name);
    if(kern_result == KERN_SUCCESS)
    {
        result = std::string(name);
    }
    return result;
}

io_object_t USBSerialScanner::GetParentFTDIDevice(io_object_t& device) {
    io_object_t parent = 0;
    
    io_object_t deviceObject = device;
    IOObjectRetain(deviceObject);
        
    while (IORegistryEntryGetParentEntry(deviceObject, kIOServicePlane, &parent) == KERN_SUCCESS)
    {
        IOObjectRelease(deviceObject);
        deviceObject = parent;
        if (GetDeviceClass(parent) == "AppleUSBFTDI") {
            return deviceObject;
        }
    }
    return device;
}