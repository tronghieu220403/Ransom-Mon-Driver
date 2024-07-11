#ifdef _WIN32

#include "wmieventclass.h"

namespace rm
{
    // https://learn.microsoft.com/vi-vn/windows/win32/etw/retrieving-event-data-using-mof
    WmiEventClass::WmiEventClass(std::wstring class_guid, ULONG version, ULONG type, USHORT pointer_size):
        class_guid_(class_guid),
        version_(version),
        type_(type),
        pointer_size_(pointer_size)
    {
        ConnectToETWNamespace();
    }

    USHORT WmiEventClass::GetPointerSize() const
    {
        return pointer_size_;
    }

    void WmiEventClass::SetPointerSize(USHORT pointer_size)
    {
        pointer_size_ = pointer_size;
    }

    std::wstring WmiEventClass::GetClassGuid() const {
        return class_guid_;
    }

    void WmiEventClass::SetClassGuid(std::wstring class_guid) {
        class_guid_ = class_guid;
    }

    ULONG WmiEventClass::GetVersion() const {
        return version_;
    }

    void WmiEventClass::SetVersion(ULONG version) {
        version_ = version;
    }

    ULONG WmiEventClass::GetType() const {
        return type_;
    }

    void WmiEventClass::SetType(ULONG type) {
        type_ = type;
    }


    long long WmiEventClass::ConnectToETWNamespace()
    {
        HRESULT hr = S_OK;
        IWbemLocator* p_locator = NULL;

        BSTR bstr_name_space = _bstr_t(L"root\\wmi");
        hr = CoInitialize(0);

        hr = CoCreateInstance(__uuidof(WbemLocator),
            0,
            CLSCTX_INPROC_SERVER,
            __uuidof(IWbemLocator),
            (LPVOID*)&p_locator);
        
        if (FAILED(hr))
        {
            wprintf(L"CoCreateInstance failed with 0x%x\n", hr);
            goto cleanup;
        }

        hr = p_locator->ConnectServer(bstr_name_space,
            NULL, NULL, NULL,
            0L, NULL, NULL,
            &p_services_);

        if (FAILED(hr))
        {
            wprintf(L"p_locator->ConnectServer failed with 0x%x\n", hr);
            goto cleanup;
        }

        hr = CoSetProxyBlanket(p_services_,
            RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
            NULL,
            RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL, EOAC_NONE);

        if (FAILED(hr))
        {
            wprintf(L"CoSetProxyBlanket failed with 0x%x\n", hr);
            p_services_->Release();
            p_services_ = NULL;
        }

        cleanup:

        if (p_locator)
            p_locator->Release();

        return hr;
    }

    IWbemClassObject* WmiEventClass::GetEventCategoryClass(BSTR bstr_class_guid, ULONG version)
    {
        HRESULT hr = S_OK;
        HRESULT hr_qualifier = S_OK;
        IEnumWbemClassObject* p_classes = NULL;
        IWbemClassObject* p_class = NULL;
        IWbemQualifierSet* p_qualifiers = NULL;
        ULONG cnt = 0;
        VARIANT var_guid;
        VARIANT var_version;

        hr = p_services_->CreateClassEnum(_bstr_t(L"EventTrace"),
            WBEM_FLAG_DEEP | WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_USE_AMENDED_QUALIFIERS,
            NULL, &p_classes);

        if (FAILED(hr))
        {
            wprintf(L"g_pServices->CreateClassEnum failed with 0x%x\n", hr);
            goto cleanup;
        }

        while (S_OK == hr)
        {
            hr = p_classes->Next(WBEM_INFINITE, 1, &p_class, &cnt);

            if (FAILED(hr))
            {
                wprintf(L"p_classes->Next failed with 0x%x\n", hr);
                goto cleanup;
            }

            // Get all the qualifiers for the class and search for the Guid qualifier. 

            hr_qualifier = p_class->GetQualifierSet(&p_qualifiers);

            if (p_qualifiers)
            {
                hr_qualifier = p_qualifiers->Get(L"Guid", 0, &var_guid, NULL);

                if (SUCCEEDED(hr_qualifier))
                {
                    // Compare this class' GUID to the one from the event.

                    if (_wcsicmp(var_guid.bstrVal, bstr_class_guid) == 0)
                    {
                        // If the GUIDs are equal, check for the correct version.
                        // The version is correct if the class does not contain the EventVersion
                        // qualifier or the class version matches the version from the event.

                        hr_qualifier = p_qualifiers->Get(L"EventVersion", 0, &var_version, NULL);

                        if (SUCCEEDED(hr_qualifier))
                        {
                            if (version == var_version.intVal)
                            {
                                break; //found class
                            }

                            VariantClear(&var_version);
                        }
                        else if (WBEM_E_NOT_FOUND == hr_qualifier)
                        {
                            break; //found class
                        }
                    }

                    VariantClear(&var_guid);
                }

                p_qualifiers->Release();
                p_qualifiers = NULL;
            }

            p_class->Release();
            p_class = NULL;
        }

    cleanup:

        if (p_classes)
        {
            p_classes->Release();
            p_classes = NULL;
        }

        if (p_qualifiers)
        {
            p_qualifiers->Release();
            p_qualifiers = NULL;
        }

        VariantClear(&var_version);
        VariantClear(&var_guid);

        return p_class;

    }

    IWbemClassObject* WmiEventClass::GetEventClass(IWbemClassObject* p_event_category_class, ULONG event_type)
    {
        HRESULT hr = S_OK;
        HRESULT hr_qualifier = S_OK;
        IEnumWbemClassObject* p_classes = NULL;
        IWbemClassObject* p_class = NULL;
        IWbemQualifierSet* p_qualifiers = NULL;
        ULONG cnt = 0;
        VARIANT var_class_name;
        VARIANT var_event_type;
        BOOL found_event_class = FALSE;

        // Get the name of the event category class so you can enumerate its children classes.

        hr = p_event_category_class->Get(L"__RELPATH", 0, &var_class_name, NULL, NULL);

        if (FAILED(hr))
        {
            wprintf(L"p_event_category_class->Get failed with 0x%x\n", hr);
            goto cleanup;
        }

        hr = p_services_->CreateClassEnum(var_class_name.bstrVal,
            WBEM_FLAG_SHALLOW | WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_USE_AMENDED_QUALIFIERS,
            NULL, &p_classes);

        if (FAILED(hr))
        {
            wprintf(L"p_services_->CreateClassEnum failed with 0x%x\n", hr);
            goto cleanup;
        }

        // Loop through the enumerated classes and find the event class that 
        // matches the event. The class is a match if the event type from the 
        // event matches the value from the event_type class qualifier. 

        while (S_OK == hr)
        {
            hr = p_classes->Next(WBEM_INFINITE, 1, &p_class, &cnt);

            if (FAILED(hr))
            {
                wprintf(L"p_classes->Next failed with 0x%x\n", hr);
                goto cleanup;
            }
            if (hr == S_FALSE)
            {
                goto cleanup;
            }

            // Get all the qualifiers for the class and search for the event_type qualifier. 

            hr_qualifier = p_class->GetQualifierSet(&p_qualifiers);

            if (FAILED(hr_qualifier))
            {
                wprintf(L"p_class->GetQualifierSet failed with 0x%x\n", hr_qualifier);
                p_class->Release();
                p_class = NULL;
                goto cleanup;
            }

            hr_qualifier = p_qualifiers->Get(L"EventType", 0, &var_event_type, NULL);

            if (FAILED(hr_qualifier))
            {
                wprintf(L"p_qualifiers->Get(eventtype) failed with 0x%x\n", hr_qualifier);
                p_class->Release();
                p_class = NULL;
                goto cleanup;
            }

            // If multiple events provide the same data, the event_type qualifier
            // will contain an array of types. Loop through the array and find a match.

            if (var_event_type.vt & VT_ARRAY)
            {
                HRESULT hrSafe = S_OK;
                int ClassEventType;
                SAFEARRAY* pEventTypes = var_event_type.parray;

                for (LONG i = 0; (ULONG)i < pEventTypes->rgsabound->cElements; i++)
                {
                    hrSafe = SafeArrayGetElement(pEventTypes, &i, &ClassEventType);

                    if (ClassEventType == event_type)
                    {
                        found_event_class = TRUE;
                        break;  //for loop
                    }
                }
            }
            else
            {
                if (var_event_type.intVal == event_type)
                {
                    found_event_class = TRUE;
                }
            }

            VariantClear(&var_event_type);

            if (TRUE == found_event_class)
            {
                break;  //while loop
            }

            p_class->Release();
            p_class = NULL;
        }

    cleanup:

        if (p_classes)
        {
            p_classes->Release();
            p_classes = NULL;
        }

        if (p_qualifiers)
        {
            p_qualifiers->Release();
            p_qualifiers = NULL;
        }

        VariantClear(&var_class_name);
        VariantClear(&var_event_type);

        return p_class;
    }

    BOOL WmiEventClass::GetPropertyList(IWbemClassObject* p_class, PropertyList** p_p_properties, DWORD* p_property_count, LONG** p_p_property_index)
    {
        HRESULT hr = S_OK;
        SAFEARRAY* p_names = NULL;
        LONG j = 0;
        VARIANT var;

        // Retrieve the property names.

        hr = p_class->GetNames(NULL, WBEM_FLAG_NONSYSTEM_ONLY, NULL, &p_names);
        if (p_names)
        {
            
            // number of properties in ETW Event
            *p_property_count = p_names->rgsabound->cElements;

            // Allocate a block of memory to hold an array of PropertyList structures.

            *p_p_properties = (PropertyList*)malloc(sizeof(PropertyList) * (*p_property_count));
            if (NULL == *p_p_properties)
            {
                hr = E_OUTOFMEMORY;
                goto cleanup;
            }

            // WMI may not return the properties in the order as defined in the MOF. Allocate
            // an array of indexes that map into the property list array, so you can retrieve
            // the event data in the correct order.

            *p_p_property_index = (LONG*)malloc(sizeof(LONG) * (*p_property_count));
            if (NULL == *p_p_property_index)
            {
                hr = E_OUTOFMEMORY;
                goto cleanup;
            }

            for (LONG i = 0; (ULONG)i < *p_property_count; i++)
            {
                //Save the name of the property.

                hr = SafeArrayGetElement(p_names, &i, &((*p_p_properties + i)->name));
                if (FAILED(hr))
                {
                    goto cleanup;
                }

                //Save the qualifiers. Used latter to help determine how to read the event data.

                hr = p_class->GetPropertyQualifierSet((*p_p_properties + i)->name, &((*p_p_properties + i)->p_qualifiers));
                if (FAILED(hr))
                {
                    goto cleanup;
                }

                // Use the WmiDataId qualifier to determine the correct property order.
                // Index[0] points to the property list element that contains WmiDataId("1"),
                // Index[1] points to the property list element that contains WmiDataId("2"),
                // and so on. 

                hr = (*p_p_properties + i)->p_qualifiers->Get(L"WmiDataId", 0, &var, NULL);
                if (SUCCEEDED(hr))
                {
                    j = var.intVal - 1;
                    VariantClear(&var);
                    *(*p_p_property_index + j) = i;
                }
                else if (WBEM_E_NOT_FOUND == hr)
                {
                    continue; // Ignore property without WmiDataId
                }
                else
                {
                    goto cleanup;
                }

                // Get the data type of the property by the name of property.

                hr = p_class->Get((*p_p_properties + i)->name, 0, NULL, &((*p_p_properties + i)->data_type), NULL);
                if (FAILED(hr))
                {
                    goto cleanup;
                }
            }
        }

    cleanup:

        if (p_names)
        {
            SafeArrayDestroy(p_names);
        }

        if (FAILED(hr))
        {
            if (*p_p_properties)
            {
                FreePropertyList(*p_p_properties, *p_property_count, *p_p_property_index);
            }

            return FALSE;
        }

        return TRUE;
    }

    void WmiEventClass::FreePropertyList(PropertyList* p_properties, DWORD count, LONG* p_index)
    {
        if (p_properties)
        {
            for (DWORD i = 0; i < count; i++)
            {
                SysFreeString((p_properties + i)->name);

                if ((p_properties + i)->p_qualifiers)
                {
                    (p_properties + i)->p_qualifiers->Release();
                    (p_properties + i)->p_qualifiers = NULL;
                }
            }

            free(p_properties);
        }

        if (p_index)
            free(p_index);
    }

    std::wstring WmiEventClass::GetPropertyName(PropertyList* p_property)
    {
        HRESULT hr;
        VARIANT var_display_name;
        std::wstring name;
        BSTR name_ptr;
        long long size = 0;
        // Retrieve the Description qualifier for the property. The description qualifier
        // should contain a printable display name for the property. If the qualifier is
        // not found, print the property name.

        hr = p_property->p_qualifiers->Get(L"Description", 0, &var_display_name, NULL);
        //wprintf(L"%s:\n", (SUCCEEDED(hr)) ? var_display_name.bstrVal : p_property->name);
        
        name_ptr = (SUCCEEDED(hr)) ? var_display_name.bstrVal : p_property->name;
        size = wcslen(name_ptr);
        name.resize(size);
        memcpy(&name[0], name_ptr, size * sizeof(WCHAR));

        VariantClear(&var_display_name);
        return name;
    }

    // Get datasize of a property, can not get datasize of a string or an array
    void WmiEventClass::GetEventPropertyValue(PropertyList* p_property, 
    int &data_size)
    {
        HRESULT hr;
        VARIANT var_qualifier;
        ULONG array_size = 1;
        int element_size = 0;

        data_size = 0;
        // If the property contains the Pointer or PointerType qualifier,
        // you do not need to know the data type of the property. You just
        // retrieve either four bytes or eight bytes depending on the 
        // pointer's size.

        if (SUCCEEDED(hr = p_property->p_qualifiers->Get(L"Pointer", 0, NULL, NULL)) ||
            SUCCEEDED(hr = p_property->p_qualifiers->Get(L"PointerType", 0, NULL, NULL)))
        {
            data_size = pointer_size_;
            return ;
        }
        else
        {
            if (p_property->data_type & CIM_FLAG_ARRAY)
            {
                hr = p_property->p_qualifiers->Get(L"MAX", 0, &var_qualifier, NULL);
                if (SUCCEEDED(hr))
                {
                    array_size = var_qualifier.intVal;
                    VariantClear(&var_qualifier);
                    if (array_size != 1)
                    {
                        return;
                    }
                }
                else
                {
                    return;
                }
            }

            switch (p_property->data_type & (~CIM_FLAG_ARRAY))
            {
            case CIM_SINT32:
            {
                data_size = sizeof(LONG) * array_size;
                return;
            }

            case CIM_UINT32:
            {
                data_size = sizeof(ULONG) * array_size;
                return;
            }

            case CIM_SINT64:
            {
                data_size = sizeof(LONGLONG) * array_size;
                return;
            }

            case CIM_UINT64:
            {
                data_size = sizeof(ULONGLONG) * array_size;
                return;
            }

            case CIM_OBJECT:
            
            hr = p_property->p_qualifiers->Get(L"Extension", 0, &var_qualifier, NULL);
            if (SUCCEEDED(hr))
            {
                if (_wcsicmp(L"SizeT", var_qualifier.bstrVal) == 0)
                {
                    VariantClear(&var_qualifier);

                    // You do not need to know the data type of the property, you just 
                    // retrieve either 4 bytes or 8 bytes depending on the pointer's size.

                    for (ULONG i = 0; i < array_size; i++)
                    {
                        data_size += pointer_size_;
                    }
                    return;
                }
                if (_wcsicmp(L"Port", var_qualifier.bstrVal) == 0)
                {
                    element_size = sizeof(USHORT);
                }
                else if (_wcsicmp(L"IPAddr", var_qualifier.bstrVal) == 0 ||
                    _wcsicmp(L"IPAddrV4", var_qualifier.bstrVal) == 0)
                {
                    element_size = sizeof(ULONG);
                }
                else if (_wcsicmp(L"IPAddrV6", var_qualifier.bstrVal) == 0)
                {
                    if (GetProcAddress(
                        GetModuleHandle(L"ntdll"), "RtlIpv6AddressToStringW")
                        == NULL)
                        {
                            return;
                        }

                    element_size = sizeof(IN6_ADDR);
                }
                else if (_wcsicmp(L"Guid", var_qualifier.bstrVal) == 0)
                {
                    element_size = sizeof(GUID);
                }
            }
            else if (_wcsicmp(L"Sid", var_qualifier.bstrVal) == 0)
            {
                // Handle sid here, in fact that our events does not contain any "SID" field;
                return;
            }
            else
            {
                return;
            }

            VariantClear(&var_qualifier);
            for (ULONG i = 0; i < array_size; i++)
            {
                data_size += element_size;
            }

            return;


            case CIM_SINT8:
            case CIM_UINT8:
            {
                hr = p_property->p_qualifiers->Get(L"Extension", 0, &var_qualifier, NULL);
                if (SUCCEEDED(hr))
                {
                    VariantClear(&var_qualifier);
                    data_size += sizeof(GUID);
                }
                else
                {
                    for (ULONG i = 0; i < array_size; i++)
                    {
                        data_size += sizeof(UINT8);
                    }
                }
                return;
            }

            case CIM_CHAR16:
            {
                for (ULONG i = 0; i < array_size; i++)
                {
                    data_size += sizeof(WCHAR);
                }
                return;
            }

            case CIM_SINT16:
            {
                for (ULONG i = 0; i < array_size; i++)
                {
                    data_size += sizeof(SHORT);
                }
                return;
            }

            case CIM_UINT16:
            {
                for (ULONG i = 0; i < array_size; i++)
                {
                    data_size += sizeof(USHORT);
                }
                return;
            }

            case CIM_STRING:
            
            return;

            default:
            {
                return;
            }

            } // switch
        }
    }

    WmiEventClass::~WmiEventClass()
    {
        if (p_services_)
        {
            p_services_->Release();
            p_services_ = NULL;
        }
        CoUninitialize();
    }

    std::pair<int, int> WmiEventClass::GetPropertyInfo(std::wstring property_name)
    {
        int offset = -1;
        int temp_offset = 0;
        IWbemClassObject *p_event_category_class = NULL;
        PropertyList *p_properties = NULL;
        IWbemClassObject *p_event_class = NULL;
        DWORD property_count = NULL;
        LONG *p_property_index = NULL;
        int data_size = 0;

        p_event_category_class = GetEventCategoryClass(&class_guid_[0], version_);
        if (p_event_category_class)
        {
            p_event_class = GetEventClass(p_event_category_class, type_);

            p_event_category_class->Release();
            p_event_category_class = NULL;

            if (p_event_class)
            {
                // Enumerate the properties and retrieve the event data.

                if (TRUE == GetPropertyList(p_event_class, &p_properties, &property_count, &p_property_index))
                {
                    for (LONG i = 0; (DWORD)i < property_count; i++)
                    {
                        std::wstring name = GetPropertyName(p_properties + p_property_index[i]);

                        GetEventPropertyValue(p_properties + p_property_index[i], data_size);
                        if (data_size == 0)
                        {
                            exit(0);
                        }
                        if (name == property_name)
                        {
                            offset = temp_offset;
                            break;
                        }

                        temp_offset += data_size;
                    }

                    FreePropertyList(p_properties, property_count, p_property_index);
                }

                p_event_class->Release();
                p_event_class = NULL;
            }
        }
        return {offset, data_size};
    }
}
#endif
