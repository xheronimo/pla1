#pragma once
#include <Preferences.h>

namespace NVS
{
    class Store
    {
    public:
        Store(const char* ns, bool readOnly = false)
            : _ns(ns), _readOnly(readOnly), _open(false) {}

        ~Store() { end(); }

        bool begin()
        {
            if (_open) return true;
            _open = _prefs.begin(_ns, _readOnly);
            return _open;
        }

        void end()
        {
            if (_open) {
                _prefs.end();
                _open = false;
            }
        }

        // ---------- GET ----------
        bool getBool(const char* key, bool def = false)
        {
            begin();
            return _prefs.getBool(key, def);
        }

        uint32_t getUInt(const char* key, uint32_t def = 0)
        {
            begin();
            return _prefs.getUInt(key, def);
        }

        float getFloat(const char* key, float def = 0.0f)
        {
            begin();
            return _prefs.getFloat(key, def);
        }

        size_t getBytes(const char* key, void* out, size_t len)
        {
            begin();
            return _prefs.getBytes(key, out, len);
        }

        // ---------- PUT ----------
        void putBool(const char* key, bool value)
        {
            begin();
            _prefs.putBool(key, value);
        }

        void putUInt(const char* key, uint32_t value)
        {
            begin();
            _prefs.putUInt(key, value);
        }

        void putFloat(const char* key, float value)
        {
            begin();
            _prefs.putFloat(key, value);
        }

        void putBytes(const char* key, const void* data, size_t len)
        {
            begin();
            _prefs.putBytes(key, data, len);
        }

        void remove(const char* key)
        {
            begin();
            _prefs.remove(key);
        }

        void clear()
        {
            begin();
            _prefs.clear();
        }

    private:
        Preferences _prefs;
        const char* _ns;
        bool _readOnly;
        bool _open;
    };

    // Stores globales
    inline Store& sys()
    {
        static Store inst("system");
        return inst;
    }

    inline Store& signals()
    {
        static Store inst("signals");
        return inst;
    }

    inline Store& alarms()
    {
        static Store inst("alarms");
        return inst;
    }
}
