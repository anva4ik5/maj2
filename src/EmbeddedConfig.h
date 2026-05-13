#pragma once

// ===========================================================================
// Embedded compile-time configuration.
// Values are injected via CMake -D EMBEDDED_API_URL=... / -D EMBEDDED_SHARED_KEY=...
// (set as GitHub Actions secrets so they never end up in source/config.ini).
// Local config.ini overrides these only if the corresponding key is non-empty.
// ===========================================================================

#ifndef EMBEDDED_API_URL
#define EMBEDDED_API_URL "https://majcheat-production.up.railway.app"
#endif

#ifndef EMBEDDED_SHARED_KEY
#define EMBEDDED_SHARED_KEY ""
#endif

#ifndef EMBEDDED_BOT_USERNAME
#define EMBEDDED_BOT_USERNAME ""
#endif

namespace EmbeddedConfig {
    inline const char* apiUrl()       { return EMBEDDED_API_URL; }
    inline const char* sharedKey()    { return EMBEDDED_SHARED_KEY; }
    inline const char* botUsername()  { return EMBEDDED_BOT_USERNAME; }
}
