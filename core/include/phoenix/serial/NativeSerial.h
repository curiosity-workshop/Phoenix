#pragma once

#if defined(_WIN32)
#include <phoenix/serial/WindowsSerialEnumerator.h>
#include <phoenix/serial/WindowsSerialTransportFactory.h>
#elif defined(__APPLE__)
#include <phoenix/serial/MacSerialEnumerator.h>
#include <phoenix/serial/MacSerialTransportFactory.h>
#else
#include <phoenix/serial/LinuxSerialEnumerator.h>
#include <phoenix/serial/LinuxSerialTransportFactory.h>
#endif

namespace phoenix::serial
{
#if defined(_WIN32)
    using NativeSerialControlMode = WindowsSerialControlMode;
    using NativeSerialEnumerator = WindowsSerialEnumerator;
    using NativeSerialTransportFactory = WindowsSerialTransportFactory;
#elif defined(__APPLE__)
    using NativeSerialControlMode = MacSerialControlMode;
    using NativeSerialEnumerator = MacSerialEnumerator;
    using NativeSerialTransportFactory = MacSerialTransportFactory;
#else
    using NativeSerialControlMode = LinuxSerialControlMode;
    using NativeSerialEnumerator = LinuxSerialEnumerator;
    using NativeSerialTransportFactory = LinuxSerialTransportFactory;
#endif
}
