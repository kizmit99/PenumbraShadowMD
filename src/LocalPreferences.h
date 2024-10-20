#pragma once

#include <Preferences.h>

#define USE_PREFERENCES

class LocalPreferences
{
public :

    LocalPreferences(const char* nspace) : fNamespace(nspace) {
    }

    LocalPreferences() : fNamespace("LocalPreferences") {
    }

    void setNamespace(const char* nspace) {
        fNamespace = nspace;
    }

    void putInt(const char* key, int value) {
    #ifdef USE_PREFERENCES
        if (preferences.begin(fNamespace, false)) {
            preferences.putInt(key, value);
            preferences.end();
        }
    #endif
    }

    int getInt(const char* key, int defaultValue) {
        int value = defaultValue;
    #ifdef USE_PREFERENCES
        if (preferences.begin(fNamespace, true)) {
            value = preferences.getInt(key, defaultValue);
            preferences.end();
        }
    #endif
        return value;
    }

    void putBool(const char* key, bool value) {
    #ifdef USE_PREFERENCES
        if (preferences.begin(fNamespace, false)) {
            preferences.putBool(key, value);
            preferences.end();
        }
    #endif
    }

    bool getBool(const char* key, bool defaultValue) {
        bool value = defaultValue;
    #ifdef USE_PREFERENCES
        if (preferences.begin(fNamespace, true)) {
            value = preferences.getBool(key, defaultValue);
            preferences.end();
        }
    #endif
        return value;
    }

    void putString(const char* key, const char* value) {
    #ifdef USE_PREFERENCES
        if (preferences.begin(fNamespace, false)) {
            preferences.putString(key, value);
            preferences.end();
        }
    #endif
    }

    String getString(const char* key, const char* defaultValue) {
        String value = defaultValue;
    #ifdef USE_PREFERENCES
        if (preferences.begin(fNamespace, true)) {
            value = preferences.getString(key, defaultValue);
            preferences.end();
        }
    #endif
        return value;
    }

    size_t getString(const char* key, char* value, size_t maxLen) {
        size_t length = 0;
    #ifdef USE_PREFERENCES
        if (preferences.begin(fNamespace, true)) {
            length = preferences.getString(key, value, maxLen);
            preferences.end();
        }
    #endif
        return length;
    }

    void clear() {
    #ifdef USE_PREFERENCES
        if (preferences.begin(fNamespace, false)) {
            preferences.clear();
            preferences.end();
        }
    #endif
    }

    void remove(const char* key) {
    #ifdef USE_PREFERENCES
        if (preferences.begin(fNamespace, false)) {
            preferences.remove(key);
            preferences.end();
        }
    #endif
    }

    bool isKey(const char* key) {
        bool isKey = false;
    #ifdef USE_PREFERENCES
        if (preferences.begin(fNamespace, false)) {
            isKey = preferences.isKey(key);
            preferences.end();
        }
    #endif
        return isKey;
    }

private:
    const char* fNamespace;
    Preferences preferences;
};