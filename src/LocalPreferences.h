#pragma once

#include <Preferences.h>
#include <core/StringUtils.h>

//#define USE_PREFERENCES

class LocalPreferences
{
public :

    LocalPreferences(const char* nspace) : fNamespace(nspace) {
    }

    void putInt(const char* key, int value) {
    #ifdef USE_PREFERENCES
        if (preferences.begin(PREFERENCE_NAMESPACE, false)) {
            preferences.putInt(key, value);
            preferences.end();
        }
    #endif
    }

    int getInt(const char* key, int defaultValue) {
        int value = defaultValue;
    #ifdef USE_PREFERENCES
        if (preferences.begin(PREFERENCE_NAMESPACE, true)) {
            value = preferences.getInt(key, defaultValue);
            preferences.end();
        }
    #endif
        return value;
    }

    void putBool(const char* key, bool value) {
    #ifdef USE_PREFERENCES
        if (preferences.begin(PREFERENCE_NAMESPACE, false)) {
            preferences.putBool(key, value);
            preferences.end();
        }
    #endif
    }

    bool getBool(const char* key, bool defaultValue) {
        bool value = defaultValue;
    #ifdef USE_PREFERENCES
        if (preferences.begin(PREFERENCE_NAMESPACE, true)) {
            value = preferences.getBool(key, defaultValue);
            preferences.end();
        }
    #endif
        return value;
    }

    void putString(const char* key, const char* value) {
    #ifdef USE_PREFERENCES
        if (preferences.begin(PREFERENCE_NAMESPACE, false)) {
            preferences.putString(key, value);
            preferences.end();
        }
    #endif
    }

    String getString(const char* key, const char* defaultValue) {
        String value = defaultValue;
    #ifdef USE_PREFERENCES
        if (preferences.begin(PREFERENCE_NAMESPACE, true)) {
            value = preferences.getString(key, defaultValue);
            preferences.end();
        }
    #endif
        return value;
    }

    void clear() {
    #ifdef USE_PREFERENCES
        if (preferences.begin(PREFERENCE_NAMESPACE, false)) {
            preferences.clear();
            preferences.end();
        }
    #endif
    }

    void remove(const char* key) {
    #ifdef USE_PREFERENCES
        if (preferences.begin(PREFERENCE_NAMESPACE, false)) {
            preferences.remove(key);
            preferences.end();
        }
    #endif
    }

private:
    const char* fNamespace;
    Preferences preferences;
};