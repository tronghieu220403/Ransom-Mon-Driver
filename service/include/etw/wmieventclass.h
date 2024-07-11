#ifdef _WIN32

#pragma once

#ifndef PROCESSMONITORING_ETW_WMIEVENTCLASS_H_
#define PROCESSMONITORING_ETW_WMIEVENTCLASS_H_

#include "../ulti/everything.h"

namespace rm
{

    class WmiEventClass
    {
        private:

            inline static IWbemServices* p_services_ = NULL;

            USHORT pointer_size_ = 0;

            std::wstring class_guid_;
            ULONG version_;
            ULONG type_;

            typedef struct _property_list
            {
                BSTR name;     // Property name
                LONG data_type;  // Property data type
                IWbemQualifierSet* p_qualifiers;
            } PropertyList;

            IWbemClassObject* GetEventCategoryClass(BSTR bstr_class_guid, ULONG version);
            IWbemClassObject* GetEventClass(IWbemClassObject* p_event_category_class, ULONG event_type);
            BOOL GetPropertyList(IWbemClassObject* p_class, PropertyList** p_p_properties, DWORD* p_property_count, LONG** p_p_property_index);
            void FreePropertyList(PropertyList* p_properties, DWORD count, LONG* p_index);
            std::wstring GetPropertyName(PropertyList* p_property);
            
            //PropertyValue 
            void GetEventPropertyValue(PropertyList* p_property, 
                //PBYTE p_event_data, 
                int& data_size);

        public:

            WmiEventClass(std::wstring class_guid, ULONG version, ULONG type, USHORT pointer_size);

            USHORT GetPointerSize() const;
            void SetPointerSize(USHORT pointer_size);
            std::wstring GetClassGuid() const;
            void SetClassGuid(std::wstring class_guid);
            ULONG GetVersion() const;
            void SetVersion(ULONG version);
            ULONG GetType() const;
            void SetType(ULONG type);

            long long ConnectToETWNamespace();
            
            std::pair<int, int> GetPropertyInfo(std::wstring property_name);

            ~WmiEventClass();
    };
}
#endif

#endif
