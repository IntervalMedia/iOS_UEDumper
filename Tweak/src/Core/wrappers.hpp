#pragma once

#include <cstdio>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <string>
#include <utility>
#include <functional>
#include <algorithm>

#include "Offsets.hpp"

class UE_UObjectArray;

namespace UEVars
{
    extern uintptr_t BaseAddress;
    extern unsigned long pagezero_size;

    extern bool isUsingFNamePool;
    extern bool isUsingOutlineNumberName;

    extern uintptr_t NamePoolDataPtr;
    extern uintptr_t GNamesPtr;
    extern uintptr_t ObjObjectsPtr;

    extern UE_UObjectArray ObjObjects;

    extern UE_Offsets offsets;
    
    // custom method in case game has different implementation to get names
    extern NameByIndex_t customNameByIndex;
    
    // custom method in case game has different implementation to get objects
    extern ObjectByIndex_t customObjectByIndex;
};

template<typename T>
constexpr uint64_t GetMaxOfType()
{
    return (1ull << (sizeof(T) * 0x8ull)) - 1;
}

std::string UE_GetNameByID(int32_t id);

struct TArray
{
	uint8_t *Data;
	int32_t Count;
	int32_t Max;
};

class UE_FName
{
protected:
    uint8_t *object;

public:
    UE_FName(uint8_t *object) : object(object) {}
    UE_FName() : object(nullptr) {}
    int GetNumber() const;
    std::string GetName() const;
};

enum class PropertyType
{
    Unknown,
    StructProperty,
    ObjectProperty,
    SoftObjectProperty,
    FloatProperty,
    ByteProperty,
    BoolProperty,
    IntProperty,
    Int8Property,
    Int16Property,
    Int32Property,
    Int64Property,
    UInt16Property,
    UInt32Property,
    UInt64Property,
    NameProperty,
    DelegateProperty,
    SetProperty,
    ArrayProperty,
    WeakObjectProperty,
    LazyObjectProperty,
    StrProperty,
    TextProperty,
    MulticastSparseDelegateProperty,
    EnumProperty,
    DoubleProperty,
    MulticastDelegateProperty,
    ClassProperty,
    MulticastInlineDelegateProperty,
    MapProperty,
    InterfaceProperty,
    FieldPathProperty,
    SoftClassProperty
};

enum class EInternalObjectFlags : int32_t
{
    None = 0,
    ReachableInCluster = 1 << 23,
    ClusterRoot = 1 << 24,
    Native = 1 << 25,
    Async = 1 << 26,
    AsyncLoading = 1 << 27,
    Unreachable = 1 << 28,
    PendingKill = 1 << 29,
    RootSet = 1 << 30,
    GarbageCollectionKeepFlags = Native | Async | AsyncLoading,
    AllFlags = ReachableInCluster | ClusterRoot | Native | Async | AsyncLoading | Unreachable | PendingKill | RootSet,
};

class UE_UClass;
class UE_FField;

class UE_UObject
{
protected:
    uint8_t *object;

public:
    UE_UObject(void *object) : object((uint8_t *)object) {}
    UE_UObject() : object(nullptr) {}
    bool operator==(const UE_UObject obj) const { return obj.object == object; };
    bool operator!=(const UE_UObject obj) const { return obj.object != object; };
    int32_t GetIndex() const;
    UE_UClass GetClass() const;
    UE_UObject GetOuter() const;
    UE_UObject GetPackageObject() const;
    std::string GetName() const;
    std::string GetFullName() const;
    std::string GetCppName() const;
    std::string GetCppTypeName() const;
    void *GetAddress() const { return object; }
    operator uint8_t *() const { return object; };
    operator bool() const { return object != nullptr; }

    template <typename Base>
    Base Cast() const { return Base(object); }

    template <typename T>
    bool IsA() const;

    bool IsA(UE_UClass cmp) const;

    static UE_UClass StaticClass();
};

class UE_UObjectArray
{
public:
    UE_UObjectArray() : Objects(nullptr) {}
    
	uint8_t **Objects;

	int32_t GetNumElements() const;

	uint8_t *GetObjectPtr(int32_t id) const;

	void ForEachObject(const std::function<bool(uint8_t *)> &callback) const;
	void ForEachObjectOfClass(const class UE_UClass &cmp, const std::function<bool(uint8_t *)> &callback) const;
    
    bool IsObject(const UE_UObject &address) const;
    
    template<typename T = UE_UObject>
    T FindObject(const std::string &fullName) const
    {
        for (int32_t i = 0; i < GetNumElements(); i++)
        {
            UE_UObject object = GetObjectPtr(i);
            if (object && object.GetFullName() == fullName)
            {
                return object.Cast<T>();
            }
        }
        return T();
    }
    
    template<typename T = UE_UObject>
    T FindObjectFast(const std::string &name) const
    {
        for (int32_t i = 0; i < GetNumElements(); i++)
        {
            UE_UObject object = GetObjectPtr(i);
            if (object && object.GetName() == name)
            {
                return object.Cast<T>();
            }
        }
        return T();
    }
    
    template<typename T = UE_UObject>
    T FindObjectFastInOuter(const std::string &name, const std::string &outer)
    {
        for (int32_t i = 0; i < GetNumElements(); i++)
        {
            UE_UObject object = GetObjectPtr(i);
            if (object.GetName() == name && object.GetOuter().GetName() == outer)
            {
                return object.Cast<T>();
            }
        }

        return T();
    }
};

class UE_UInterface : public UE_UObject
{
public:
    static UE_UClass StaticClass();
};

class UE_AActor : public UE_UObject
{
public:
	static UE_UClass StaticClass();
};

class UE_UField : public UE_UObject
{
public:
	using UE_UObject::UE_UObject;
	UE_UField GetNext() const;
	static UE_UClass StaticClass();
};

enum EPropertyFlags : uint64_t
{
    CPF_None = 0,
    
    CPF_Edit = 0x0000000000000001,    ///< Property is user-settable in the editor.
    CPF_ConstParm = 0x0000000000000002,    ///< This is a constant function parameter
    CPF_BlueprintVisible = 0x0000000000000004,    ///< This property can be read by blueprint code
    CPF_ExportObject = 0x0000000000000008,    ///< Object can be exported with actor.
    CPF_BlueprintReadOnly = 0x0000000000000010,    ///< This property cannot be modified by blueprint code
    CPF_Net = 0x0000000000000020,    ///< Property is relevant to network replication.
    CPF_EditFixedSize = 0x0000000000000040,    ///< Indicates that elements of an array can be modified, but its size cannot be changed.
    CPF_Parm = 0x0000000000000080,    ///< Function/When call parameter.
    CPF_OutParm = 0x0000000000000100,    ///< Value is copied out after function call.
    CPF_ZeroConstructor = 0x0000000000000200,    ///< memset is fine for construction
    CPF_ReturnParm = 0x0000000000000400,    ///< Return value.
    CPF_DisableEditOnTemplate = 0x0000000000000800,    ///< Disable editing of this property on an archetype/sub-blueprint
    CPF_NonNullable = 0x0000000000001000,    ///< Object property can never be null
    CPF_Transient = 0x0000000000002000,    ///< Property is transient: shouldn't be saved or loaded, except for Blueprint CDOs.
    CPF_Config = 0x0000000000004000,    ///< Property should be loaded/saved as permanent profile.
    CPF_RequiredParm = 0x0000000000008000,    ///< Parameter must be linked explicitly in blueprint. Leaving the parameter out results in a compile error.
    CPF_DisableEditOnInstance = 0x0000000000010000,    ///< Disable editing on an instance of this class
    CPF_EditConst = 0x0000000000020000,    ///< Property is uneditable in the editor.
    CPF_GlobalConfig = 0x0000000000040000,    ///< Load config from base class, not subclass.
    CPF_InstancedReference = 0x0000000000080000,    ///< Property is a component references.
                                                    //CPF_                                = 0x0000000000100000,    ///<
    CPF_DuplicateTransient = 0x0000000000200000,    ///< Property should always be reset to the default value during any type of duplication (copy/paste, binary duplication, etc.)
                                                    //CPF_                                = 0x0000000000400000,    ///<
                                                    //CPF_                                = 0x0000000000800000,    ///<
    CPF_SaveGame = 0x0000000001000000,    ///< Property should be serialized for save games, this is only checked for game-specific archives with ArIsSaveGame
    CPF_NoClear = 0x0000000002000000,    ///< Hide clear (and browse) button.
                                         //CPF_                              = 0x0000000004000000,    ///<
    CPF_ReferenceParm = 0x0000000008000000,    ///< Value is passed by reference; CPF_OutParam and CPF_Param should also be set.
    CPF_BlueprintAssignable = 0x0000000010000000,    ///< MC Delegates only.  Property should be exposed for assigning in blueprint code
    CPF_Deprecated = 0x0000000020000000,    ///< Property is deprecated.  Read it from an archive, but don't save it.
    CPF_IsPlainOldData = 0x0000000040000000,    ///< If this is set, then the property can be memcopied instead of CopyCompleteValue / CopySingleValue
    CPF_RepSkip = 0x0000000080000000,    ///< Not replicated. For non replicated properties in replicated structs
    CPF_RepNotify = 0x0000000100000000,    ///< Notify actors when a property is replicated
    CPF_Interp = 0x0000000200000000,    ///< interpolatable property for use with cinematics
    CPF_NonTransactional = 0x0000000400000000,    ///< Property isn't transacted
    CPF_EditorOnly = 0x0000000800000000,    ///< Property should only be loaded in the editor
    CPF_NoDestructor = 0x0000001000000000,    ///< No destructor
                                              //CPF_                                = 0x0000002000000000,    ///<
    CPF_AutoWeak = 0x0000004000000000,    ///< Only used for weak pointers, means the export type is autoweak
    CPF_ContainsInstancedReference = 0x0000008000000000,    ///< Property contains component references.
    CPF_AssetRegistrySearchable = 0x0000010000000000,    ///< asset instances will add properties with this flag to the asset registry automatically
    CPF_SimpleDisplay = 0x0000020000000000,    ///< The property is visible by default in the editor details view
    CPF_AdvancedDisplay = 0x0000040000000000,    ///< The property is advanced and not visible by default in the editor details view
    CPF_Protected = 0x0000080000000000,    ///< property is protected from the perspective of script
    CPF_BlueprintCallable = 0x0000100000000000,    ///< MC Delegates only.  Property should be exposed for calling in blueprint code
    CPF_BlueprintAuthorityOnly = 0x0000200000000000,    ///< MC Delegates only.  This delegate accepts (only in blueprint) only events with BlueprintAuthorityOnly.
    CPF_TextExportTransient = 0x0000400000000000,    ///< Property shouldn't be exported to text format (e.g. copy/paste)
    CPF_NonPIEDuplicateTransient = 0x0000800000000000,    ///< Property should only be copied in PIE
    CPF_ExposeOnSpawn = 0x0001000000000000,    ///< Property is exposed on spawn
    CPF_PersistentInstance = 0x0002000000000000,    ///< A object referenced by the property is duplicated like a component. (Each actor should have an own instance.)
    CPF_UObjectWrapper = 0x0004000000000000,    ///< Property was parsed as a wrapper class like TSubclassOf<T>, FScriptInterface etc., rather than a USomething*
    CPF_HasGetValueTypeHash = 0x0008000000000000,    ///< This property can generate a meaningful hash value.
    CPF_NativeAccessSpecifierPublic = 0x0010000000000000,    ///< Public native access specifier
    CPF_NativeAccessSpecifierProtected = 0x0020000000000000,    ///< Protected native access specifier
    CPF_NativeAccessSpecifierPrivate = 0x0040000000000000,    ///< Private native access specifier
    CPF_SkipSerialization = 0x0080000000000000,    ///< Property shouldn't be serialized, can still be exported to text
};

typedef std::pair<PropertyType, std::string> type;

class IProperty
{
protected:
	const void *prop;

public:
	IProperty(const void *object) : prop(object) {}
	virtual std::string GetName() const = 0;
	virtual int32_t GetArrayDim() const = 0;
	virtual int32_t GetSize() const = 0;
	virtual int32_t GetOffset() const = 0;
	virtual uint64_t GetPropertyFlags() const = 0;
	virtual type GetType() const = 0;
	virtual uint8_t GetFieldMask() const = 0;
};

class IUProperty : public IProperty
{
public:
	using IProperty::IProperty;
	IUProperty(const class UE_UProperty *object) : IProperty(object) {}
	virtual std::string GetName() const;
	virtual int32_t GetArrayDim() const;
	virtual int32_t GetSize() const;
	virtual int32_t GetOffset() const;
	virtual uint64_t GetPropertyFlags() const;
	virtual type GetType() const;
	virtual uint8_t GetFieldMask() const;
};

class UE_UProperty : public UE_UField
{
public:
	using UE_UField::UE_UField;
	int32_t GetArrayDim() const;
	int32_t GetSize() const;
	int32_t GetOffset() const;
	uint64_t GetPropertyFlags() const;
	type GetType() const;

	IUProperty GetInterface() const;
	static UE_UClass StaticClass();
};

class UE_UStruct : public UE_UField
{
public:
	using UE_UField::UE_UField;
	UE_UStruct GetSuper() const;
	UE_FField GetChildProperties() const;
	UE_UField GetChildren() const;
	int32_t GetSize() const;
	static UE_UClass StaticClass();
    
    UE_FField FindChildProp(const std::string &name) const;
    UE_UField FindChild(const std::string &name) const;
};

enum EFunctionFlags : uint32_t
{
	// Function flags.
	FUNC_None = 0x00000000,
	FUNC_Final = 0x00000001,				  // Function is final (prebindable, non-overridable function).
	FUNC_RequiredAPI = 0x00000002,			  // Indicates this function is DLL exported/imported.
	FUNC_BlueprintAuthorityOnly = 0x00000004, // Function will only run if the object has network authority
	FUNC_BlueprintCosmetic = 0x00000008,	  // Function is cosmetic in nature and should not be invoked on dedicated servers
											  // FUNC_				= 0x00000010,   // unused.
											  // FUNC_				= 0x00000020,   // unused.
	FUNC_Net = 0x00000040,					  // Function is network-replicated.
	FUNC_NetReliable = 0x00000080,			  // Function should be sent reliably on the network.
	FUNC_NetRequest = 0x00000100,			  // Function is sent to a net service
	FUNC_Exec = 0x00000200,					  // Executable from command line.
	FUNC_Native = 0x00000400,				  // Native function.
	FUNC_Event = 0x00000800,				  // Event function.
	FUNC_NetResponse = 0x00001000,			  // Function response from a net service
	FUNC_Static = 0x00002000,				  // Static function.
	FUNC_NetMulticast = 0x00004000,			  // Function is networked multicast Server -> All Clients
	FUNC_UbergraphFunction = 0x00008000,	  // Function is used as the merge 'ubergraph' for a blueprint, only assigned when using the persistent 'ubergraph' frame
	FUNC_MulticastDelegate = 0x00010000,	  // Function is a multi-cast delegate signature (also requires FUNC_Delegate to be set!)
	FUNC_Public = 0x00020000,				  // Function is accessible in all classes (if overridden, parameters must remain unchanged).
	FUNC_Private = 0x00040000,				  // Function is accessible only in the class it is defined in (cannot be overridden, but function name may be reused in subclasses.  IOW: if overridden, parameters don't need to match, and Super.Func() cannot be accessed since it's private.)
	FUNC_Protected = 0x00080000,			  // Function is accessible only in the class it is defined in and subclasses (if overridden, parameters much remain unchanged).
	FUNC_Delegate = 0x00100000,				  // Function is delegate signature (either single-cast or multi-cast, depending on whether FUNC_MulticastDelegate is set.)
	FUNC_NetServer = 0x00200000,			  // Function is executed on servers (set by replication code if passes check)
	FUNC_HasOutParms = 0x00400000,			  // function has out (pass by reference) parameters
	FUNC_HasDefaults = 0x00800000,			  // function has structs that contain defaults
	FUNC_NetClient = 0x01000000,			  // function is executed on clients
	FUNC_DLLImport = 0x02000000,			  // function is imported from a DLL
	FUNC_BlueprintCallable = 0x04000000,	  // function can be called from blueprint code
	FUNC_BlueprintEvent = 0x08000000,		  // function can be overridden/implemented from a blueprint
	FUNC_BlueprintPure = 0x10000000,		  // function can be called from blueprint code, and is also pure (produces no side effects). If you set this, you should set FUNC_BlueprintCallable as well.
	FUNC_EditorOnly = 0x20000000,			  // function can only be called from an editor scrippt.
	FUNC_Const = 0x40000000,				  // function can be called from blueprint code, and only reads state (never writes state)
	FUNC_NetValidate = 0x80000000,			  // function must supply a _Validate implementation
	FUNC_AllFlags = 0xFFFFFFFF,
};

class UE_UFunction : public UE_UStruct
{
public:
	using UE_UStruct::UE_UStruct;
	uintptr_t GetFunc() const;

	int8_t GetNumParams() const;
	int16_t GetParamSize() const;

	uint32_t GetFunctionEFlags() const;
	std::string GetFunctionFlags() const;
	static UE_UClass StaticClass();
};

class UE_UScriptStruct : public UE_UStruct
{
public:
	using UE_UStruct::UE_UStruct;
	static UE_UClass StaticClass();
};

class UE_UClass : public UE_UStruct
{
public:
	using UE_UStruct::UE_UStruct;
	static UE_UClass StaticClass();
};

class UE_UEnum : public UE_UField
{
public:
	using UE_UField::UE_UField;
	TArray GetNames() const;
    std::string GetName() const;
	static UE_UClass StaticClass();
};

class UE_UDoubleProperty : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UFloatProperty : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UIntProperty : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UInt16Property : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UInt32Property : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UInt64Property : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UInt8Property : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UUInt16Property : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UUInt32Property : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UUInt64Property : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UTextProperty : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UStrProperty : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UStructProperty : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	UE_UStruct GetStruct() const;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UNameProperty : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UObjectPropertyBase : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	UE_UClass GetPropertyClass() const;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UObjectProperty : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	UE_UClass GetPropertyClass() const;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UArrayProperty : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	UE_UProperty GetInner() const;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UByteProperty : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	UE_UEnum GetEnum() const;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UBoolProperty : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	uint8_t GetFieldMask() const;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UEnumProperty : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
    UE_UProperty GetUnderlayingProperty() const;
    UE_UEnum GetEnum() const;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UClassProperty : public UE_UObjectPropertyBase
{
public:
	using UE_UObjectPropertyBase::UE_UObjectPropertyBase;
	UE_UClass GetMetaClass() const;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_USoftClassProperty : public UE_UClassProperty
{
public:
    using UE_UClassProperty::UE_UClassProperty;
    std::string GetTypeStr() const;
};

class UE_USetProperty : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	UE_UProperty GetElementProp() const;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UMapProperty : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	UE_UProperty GetKeyProp() const;
	UE_UProperty GetValueProp() const;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UInterfaceProperty : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	UE_UProperty GetInterfaceClass() const;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UMulticastDelegateProperty : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_UWeakObjectProperty : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_ULazyObjectProperty : public UE_UProperty
{
public:
	using UE_UProperty::UE_UProperty;
	std::string GetTypeStr() const;
	static UE_UClass StaticClass();
};

class UE_FFieldClass
{
protected:
	uint8_t *object;

public:
	UE_FFieldClass(uint8_t *object) : object(object){};
	UE_FFieldClass() : object(nullptr){};
    bool operator==(const UE_FFieldClass obj) const { return obj.object == object; };
    bool operator!=(const UE_FFieldClass obj) const { return obj.object != object; };
    void *GetAddress() const { return object; }
    operator uint8_t *() const { return object; };
    operator bool() const { return object != nullptr; }

    template <typename Base>
    Base Cast() const { return Base(object); }
	std::string GetName() const;
};

class UE_FField
{
protected:
	uint8_t *object;

public:
	UE_FField(uint8_t *object) : object(object) {}
	UE_FField() : object(nullptr) {}
    bool operator==(const UE_FField obj) const { return obj.object == object; };
    bool operator!=(const UE_FField obj) const { return obj.object != object; };
    void *GetAddress() const { return object; }
    operator uint8_t *() const { return object; };
    operator bool() const { return object != nullptr; }
    
    UE_FField GetNext() const;
	std::string GetName() const;
    UE_FFieldClass GetClass() const;

	template <typename Base>
	Base Cast() const { return Base(object); }
};

class IFProperty : public IProperty
{
public:
	IFProperty(const class UE_FProperty *object) : IProperty(object) {}
	virtual std::string GetName() const;
	virtual int32_t GetArrayDim() const;
	virtual int32_t GetSize() const;
	virtual int32_t GetOffset() const;
	virtual uint64_t GetPropertyFlags() const;
	virtual type GetType() const;
	virtual uint8_t GetFieldMask() const;
};

class UE_FProperty : public UE_FField
{
public:
	using UE_FField::UE_FField;
	int32_t GetArrayDim() const;
	int32_t GetSize() const;
	int32_t GetOffset() const;
	uint64_t GetPropertyFlags() const;
	type GetType() const;
	IFProperty GetInterface() const;
};

class UE_FStructProperty : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;
	UE_UStruct GetStruct() const;
	std::string GetTypeStr() const;
};

class UE_FObjectPropertyBase : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;
	UE_UClass GetPropertyClass() const;
	std::string GetTypeStr() const;
};

class UE_FArrayProperty : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;
	UE_FProperty GetInner() const;
	std::string GetTypeStr() const;
};

class UE_FByteProperty : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;
	UE_UEnum GetEnum() const;
	std::string GetTypeStr() const;
};

class UE_FBoolProperty : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;
	uint8_t GetFieldSize() const;
	uint8_t GetByteOffset() const;
	uint8_t GetByteMask() const;
	uint8_t GetFieldMask() const;
	std::string GetTypeStr() const;
};

class UE_FEnumProperty : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;
    UE_FProperty GetUnderlayingProperty() const;
    UE_UEnum GetEnum() const;
	std::string GetTypeStr() const;
};

class UE_FClassProperty : public UE_FObjectPropertyBase
{
public:
	using UE_FObjectPropertyBase::UE_FObjectPropertyBase;
	UE_UClass GetMetaClass() const;
	std::string GetTypeStr() const;
};

class UE_FSoftClassProperty : public UE_FClassProperty
{
public:
    using UE_FClassProperty::UE_FClassProperty;
    std::string GetTypeStr() const;
};

class UE_FSetProperty : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;
	UE_FProperty GetElementProp() const;
	std::string GetTypeStr() const;
};

class UE_FMapProperty : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;
	UE_FProperty GetKeyProp() const;
	UE_FProperty GetValueProp() const;
	std::string GetTypeStr() const;
};

class UE_FInterfaceProperty : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;
	UE_UClass GetInterfaceClass() const;
	std::string GetTypeStr() const;
};

class UE_FFieldPathProperty : public UE_FProperty
{
public:
	using UE_FProperty::UE_FProperty;
	UE_FName GetPropertyName() const;
	std::string GetTypeStr() const;
};

template <typename T>
bool UE_UObject::IsA() const
{
	auto cmp = T::StaticClass();
	if (!cmp)
	{
		return false;
	}

	return IsA(cmp);
}

class UE_UPackage
{
private:
	struct Member
	{
		std::string Type;
		std::string Name;
		uint32_t Offset = 0;
		uint32_t Size = 0;
	};
	struct Function
	{
		std::string Name;
		std::string FullName;
		std::string CppName;
		std::string Params;
		uint32_t EFlags = 0;
		std::string Flags;
		int8_t NumParams = 0;
		int16_t ParamSize = 0;
		uintptr_t Func = 0;
	};
	struct Struct
	{
		std::string Name;
		std::string FullName;
		std::string CppName;
		uint32_t Inherited = 0;
		uint32_t Size = 0;
		std::vector<Member> Members;
		std::vector<Function> Functions;
	};
	struct Enum
	{
		std::string FullName;
		std::string CppName;
		std::vector<std::string> Members;
	};

private:
	std::pair<uint8_t *const, std::vector<UE_UObject>> *Package;

public:
	std::vector<Struct> Classes;
	std::vector<Struct> Structures;
	std::vector<Enum> Enums;

private:
	static void GenerateFunction(const UE_UFunction &fn, Function *out);
	static void GenerateStruct(const UE_UStruct &object, std::vector<Struct> &arr);
	static void GenerateEnum(const UE_UEnum &object, std::vector<Enum> &arr);

	static void GenerateBitPadding(std::vector<Member> &members, uint32_t offset, uint8_t bitOffset, uint8_t size);
	static void GeneratePadding(std::vector<Member> &members, uint32_t offset, uint32_t size);
	static void FillPadding(const UE_UStruct &object, std::vector<Member> &members, uint32_t &offset, uint8_t &bitOffset, uint32_t end);

	static void AppendStructsToBuffer(std::vector<Struct> &arr, class BufferFmt *bufFmt);
	static void AppendEnumsToBuffer(std::vector<Enum> &arr, class BufferFmt *bufFmt);

public:
	UE_UPackage(std::pair<uint8_t *const, std::vector<UE_UObject>> &package) : Package(&package){};
    inline UE_UObject GetObject() const { return UE_UObject(Package->first); }
    void Process();
	bool AppendToBuffer(class BufferFmt *bufFmt);
};
