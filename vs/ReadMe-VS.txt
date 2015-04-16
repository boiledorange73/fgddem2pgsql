fgddem2pgsql for Windows

What to do before build.

1. Expat "binary" is required. Download from
  http://sourceforge.net/projects/expat/
  and install it.

2. Change macros.
  1. Click View -> Other Window -> Property Manager
  2. In Property Manager,
    Expand "Debug | Win32"
    Double click fgddem2pgsql
  3. On left side in "fgddem2pgsql" property page,
    Expand "Common Property"
    Click "User Macro"
  4. On right side in propertypage,
    Change "EXPAT_DIR", "EXPAT_INCLUDE", "EXPAT_LIB" and "EXPAT_DLL"
	to fit project properties to your environment.

3. Creates Redistribution Package
  1. WiX is required. Download from http://wixtoolset.org/ , and install it.
  2. Build Release version with VS 2013
    (In "Release" folder, fgddem2pgsql.exe, libexpat.dll, C++ libs
      and some texts are created).
  3. run following:
    candle fgddem2pgsql.wxs
    light fgddem2pgsql.wixobj

