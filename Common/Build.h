#pragma once

// X-Macros to string
#define XSTR(x) STR_IMPL(x)
#define STR_IMPL(x) #x

#define XCONCAT(x, y) CONCAT_IMPL(x, y)
#define CONCAT_IMPL(x, y) x##y

#define NCL_COUNTER __COUNTER__

#define NCL_NAME "NCL"

#define NCL_VERSION_MAJOR 1
#define NCL_VERSION_MINOR 0
#define NCL_VERSION_REVISION 0
#define NCL_MAKE_VERSION(major, minor, revision) "v" XSTR(major) "." XSTR(minor) "." XSTR(revision)

#define NCL_VERSION NCL_MAKE_VERSION(NCL_VERSION_MAJOR, NCL_VERSION_MINOR, NCL_VERSION_REVISION)