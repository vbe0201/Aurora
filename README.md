# Aurora

Aurora is a cross-platform, customizable and modular Discord client with a focus on efficiency,
speed and correctness.

A lot of effort is being put into mimicking the behavior of the official clients as accurately
as possible to minimize the ban risk for users, while also improving the user experience at
those spots where the official clients fail, including UI customization options, better
theming and lower resource consumption.

## Disclaimer

As per [Discord Terms of Service](https://discord.com/terms):

```
RIGHTS TO USE THE SERVICE
[...]
you agree not to (and not to attempt to) (i) use the Service for any use or purpose other than
as expressly permitted by these Terms;(ii) copy, adapt, modify, prepare derivative works based
upon, distribute, license, sell, transfer, publicly display, publicly perform, transmit, stream,
broadcast, attempt to discover any source code, reverse engineer, decompile, dissemble, or
otherwise exploit the Service or any portion of the Service, except as expressly permitted in
these Terms; or (iii) use data mining, robots, spiders, or similar data gathering and extraction
tools on the Service.
[...]
```

Discord takes a clear stance against the automation of user accounts, including unofficial
client implementations such as Aurora, which may result in your Discord account being terminated,
if found.

Although Aurora's implementation is based on careful reverse-engineering of Discord's internal
structures and workings, the risk is still present. This software should be used at the individual
discretion of the user.

## Installation

Coming soon.

## Building Aurora

Aurora utilizes a CMake-based build setup for its code and all third-party dependencies which
includes that most dependencies will be resolved during the build process.

However, some dependencies are not shipped with Aurora and need to be installed in advance:

* CMake itself - https://cmake.org/

* OpenSSL - See external instructions for installation of **development** builds on your OS

Now the actual application can be compiled:

```shell script
# Clone Aurora with git submodules
git clone --recursive https://github.com/vbe0201/Aurora
cd Aurora

# Create a separate build directory to avoid in-source builds
mkdir build
cd build

# Invoke CMake to build Aurora, options may be overridden for customization
cmake ..
cmake --build . --target Aurora --config Release
```

## Contributions

Aurora gladly accepts contributions that help improving code, documentation or the general
user experience for everyone. That's why we need your help. If you enjoy the project,
consider giving something back by

* helping others in the issues section

* reviewing code in Pull Requests

* requesting or implementing new features

* simply using it and telling us what worked and what didn't

## Third-party

Aurora is built on top of the following open source tools and libraries:

* Boost C++ Libraries ([Boost Software License](https://www.boost.org/users/license.html))

* CMake ([New BSD License](https://github.com/Kitware/CMake/blob/master/Copyright.txt))

* OpenSSL ([OpenSSL License](https://www.openssl.org/source/license.html))

* zlib ([zlib License](https://www.zlib.net/zlib_license.html))

## Licensing

Aurora is licensed under the terms of the GPLv3, with exemptions for specific projects
noted below.

Exemptions:

* Discord is exempt from GPLv3 licensing and may at its option re-license any source code
authored for the Aurora project under the MIT license instead.
