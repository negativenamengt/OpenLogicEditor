// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"
#include "Templates/SubclassOf.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "OpenLogicV2.h"
#include "Misc/Base64.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Serialization/StructuredArchive.h"
#include "UObject/StructOnScope.h"
#include "OpenLogicTypes.generated.h"

class UWidget;
class UOpenLogicTask;
class UOpenLogicProperty;
class UExecutionPinBase;
class UNodeBase;
class UOpenLogicRuntimeEventContext;
class UOpenLogicRuntimeGraph;

// Represents the role of a pin in the task.
UENUM(BlueprintType)
enum class EPinRole : uint8
{
	FlowControl,
	DataProperty
};

UENUM(BlueprintType)
enum class EOpenLogicPinDirection : uint8
{
	Input,
	Output
};

UENUM(BlueprintType, meta = (Bitflags))
enum EOpenLogicPinAttribute : uint8
{
	None = 0 UMETA(Hidden),
	MetaClass = 1 << 0,
	ActorMetaClass = 1 << 1,
	ClampMin = 1 << 2,
	ClampMax = 1 << 3
};
ENUM_CLASS_FLAGS(EOpenLogicPinAttribute)

template <typename T, typename = void>
struct HasStructuredArchiveOperator : std::false_type {};

template <typename T>
struct HasStructuredArchiveOperator<T, std::void_t<
	decltype(std::declval<FStructuredArchive::FSlot>() << std::declval<T&>())
>> : std::true_type {};

template <typename T>
constexpr bool HasStructuredArchiveOperator_v = HasStructuredArchiveOperator<T>::value;

USTRUCT(BlueprintType)
struct OPENLOGICV2_API FOpenLogicDefaultValue
{
	GENERATED_USTRUCT_BODY()
	
	// Generic setter for the default value (compile-time check)
	template<typename T>
	bool SetValue(const T& Value)
	{
		SerializedData.Reset();

		FMemoryWriter Writer(SerializedData, false);

		// If T supports structured-archive, use it:
		if constexpr (HasStructuredArchiveOperator_v<T>)
		{
			FStructuredArchiveFromArchive Ar(Writer);
			FStructuredArchive::FSlot Slot = Ar.GetSlot();
			Slot << const_cast<T&>(Value);
		}
		else
		{
			// RAW-BYTES fallback
			Writer.Serialize(static_cast<void*>(&Value), sizeof(T));
		}

		return !Writer.IsError();
	}

	// Generic getter for the default value (compile-time check)
	template<typename T>
	bool GetValue(T& OutValue) const
	{
		if (SerializedData.Num() == 0)
		{
			OutValue = T();
			return false;
		}

		// Create one reader for both branches
		FMemoryReader Reader(SerializedData, false);

		if constexpr (HasStructuredArchiveOperator_v<T>)
		{
			// Structured-archive path
			FStructuredArchiveFromArchive Ar(Reader);
			FStructuredArchive::FSlot Slot = Ar.GetSlot();
			Slot << OutValue;
		}
		else
		{
			// Raw-bytes fallback
			Reader.Serialize(&OutValue, sizeof(T));
		}

		return !Reader.IsError();
	}
	
	TSharedPtr<void> DeserializeToStruct(UScriptStruct* StructType);
	
	void Clear()
	{
		SerializedData.Reset();
	}

	bool IsEmpty() const
	{
		return SerializedData.Num() == 0;
	}
	
	FString ToBase64() const
	{
		return FBase64::Encode(SerializedData);
	}

	bool FromBase64(const FString& Base64String)
	{
		return FBase64::Decode(Base64String, SerializedData);
	}

	bool operator==(const FOpenLogicDefaultValue& Other) const
	{
		return SerializedData == Other.SerializedData;
	}

	bool operator!=(const FOpenLogicDefaultValue& Other) const
	{
		return !(*this == Other);
	}
	
public:
	UPROPERTY()
		TArray<uint8> SerializedData;
};

USTRUCT(BlueprintType)
struct OPENLOGICV2_API FOpenLogicPinAttributes
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinAttributes", NoClear)
		TSubclassOf<UObject> MetaClass = UObject::StaticClass();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinAttributes", NoClear)
		TSubclassOf<AActor> ActorMetaClass = AActor::StaticClass();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinAttributes")
		float ClampMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinAttributes")
		float ClampMax = 0.0f;
};

// Represents a pin in the task.
USTRUCT(BlueprintType)
struct OPENLOGICV2_API FOpenLogicPinData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLogic)
	FName PinName = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLogic)
	FText Description = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLogic)
	EPinRole Role = EPinRole::FlowControl;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLogic)
	TSubclassOf<UOpenLogicProperty> PropertyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLogic)
	FOpenLogicDefaultValue DefaultValue;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLogic)
	FOpenLogicPinAttributes Attributes;
	
	// Default constructor
	FOpenLogicPinData()
	{
		PinName = "";
		Description = FText::GetEmpty();
		Role = EPinRole::FlowControl;
		PropertyClass = nullptr;
	}

	// Constructor with parameters
	FOpenLogicPinData(FName InName, const FText& InDescription, const EPinRole InRole, const TSubclassOf<UOpenLogicProperty> InPropertyClass)
	{
		PinName = InName;
		Description = InDescription;
		Role = InRole;
		PropertyClass = InPropertyClass;
	}

	// Constructor with name
	FOpenLogicPinData(FName InName)
	{
		PinName = InName;
		Description = FText::GetEmpty();
		Role = EPinRole::FlowControl;
		PropertyClass = nullptr;
	}

	FString GetPinNameAsString() const
	{
		return PinName.ToString();
	}

	// Returns all the pin attributes this pin benefits from.
	TArray<FName> GetRelevantPropertyAttributes() const;
};

USTRUCT()
struct FOpenLogicConnectablePin
{
	GENERATED_USTRUCT_BODY()
	
	// The index into the task's input or output pin array.
	UPROPERTY()
	int32 PinIndex = INDEX_NONE;

	UPROPERTY()
	FOpenLogicPinData PinData;
};

// Represents the type of the node.
UENUM(BlueprintType)
enum class ENodeType : uint8
{
	Function,
	Event
};

UENUM(BlueprintType)
enum class ENodeLifecycle : uint8
{
	// The lifecycle of the node is determined by the task node's implementation.
	Custom,

	// The node persists for the entire graph lifetime.
	Persistent,
};

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_OpenLogicUserLibrary)

USTRUCT(BlueprintType)
struct OPENLOGICV2_API FTaskData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task Settings", meta = (DisplayName = "Task Name"))
	FString Name = "Untitled Node";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task Settings", meta = (DisplayName = "Custom Description"))
	FText Description;

	// The category of the node in the node palette.
	// To add subcategories, use the"|" character to separate them.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node Settings", meta = (DisplayName = "Node Palette Category"))
	FString Category = "";

	// The library tags that the node belongs to.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node Settings")
	FGameplayTagContainer Library = FGameplayTagContainer(TAG_OpenLogicUserLibrary);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node Settings")
	ENodeType Type = ENodeType::Function;

	// A custom widget class for the node. If not set, the default widget class defined in the graph customization asset will be used.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node Settings")
	TSoftClassPtr<UNodeBase> CustomNodeWidgetClass;
	
	// A connector that can receive data or get the node executed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pins")
	TArray<FOpenLogicPinData> InputPins;

	// A connector that can send data or execute other nodes.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pins")
	TArray<FOpenLogicPinData> OutputPins;
};

USTRUCT(BlueprintType)
struct OPENLOGICV2_API FOpenLogicEventContainer
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EventContainer")
	TArray<FGuid> NodeId;
};

USTRUCT(BlueprintType)
struct OPENLOGICV2_API FOpenLogicPinConnection
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	FGuid NodeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	int32 PinID = INDEX_NONE;

	FOpenLogicPinConnection()
    {
        NodeID = FGuid();
        PinID = INDEX_NONE;
    }

	FOpenLogicPinConnection(const FGuid& InNodeID, int32 InPinID)
    {
        NodeID = InNodeID;
        PinID = InPinID;
    }
	
	bool operator==(const FOpenLogicPinConnection& Other) const
	{
		return NodeID == Other.NodeID && PinID == Other.PinID;
	}
};

USTRUCT(BlueprintType)
struct OPENLOGICV2_API FOpenLogicPinState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PinState)
		bool IsUserCreated = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PinState, meta = (EditCondition = "IsUserCreated", EditConditionHides))
		FOpenLogicPinData PinData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PinState)
		TArray<FOpenLogicPinConnection> Connections;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PinState)
		FOpenLogicDefaultValue DefaultValue;

	// Default constructor
	FOpenLogicPinState()
	{
		IsUserCreated = false;
		PinData = FOpenLogicPinData();
		Connections = TArray<FOpenLogicPinConnection>();
	}

	// Constructor with parameters
	FOpenLogicPinState(const FOpenLogicPinData& InPinData, bool bIsUserCreated, const TArray<FOpenLogicPinConnection>& InConnections = TArray<FOpenLogicPinConnection>())
    {
		PinData = InPinData;
        IsUserCreated = bIsUserCreated;
        Connections = InConnections;
    }
};

USTRUCT(BlueprintType)
struct OPENLOGICV2_API FOpenLogicNode
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLogic)
	TSoftClassPtr<UOpenLogicTask> TaskClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLogic)
	FVector2D Position = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLogic)
	TMap<int32, FOpenLogicPinState> InputPins;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLogic)
	TMap<int32, FOpenLogicPinState> OutputPins;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLogic)
	TMap<FGuid, FString> BlueprintContent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	TMap<FName, FString> CppContent;

public:
	FOpenLogicPinData GetInputPinData(int32 PinIndex) const { return GetPinData(PinIndex, true); }
	FOpenLogicPinData GetOutputPinData(int32 PinIndex) const { return GetPinData(PinIndex, false); }

	int32 GetInputPinIndexFromName(const FName& PinName) const { return GetPinIndexFromName(PinName, true); }
	int32 GetOutputPinIndexFromName(const FName& PinName) const { return GetPinIndexFromName(PinName, false); }

private:
	FOpenLogicPinData GetPinData(int32 PinIndex, bool bIsInput) const;
	int32 GetPinIndexFromName(const FName& PinName, bool bIsInput) const;
};

USTRUCT(BlueprintType)
struct OPENLOGICV2_API FOpenLogicGraphData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graph")
	TMap<FGuid, FOpenLogicNode> Nodes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graph")
	TMap<TSubclassOf<UOpenLogicTask>, FOpenLogicEventContainer> Events;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graph")
	FVector2D GraphPosition = FVector2D::ZeroVector;

public:
	// The graph schema version that this graph was last saved with.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLogic)
	int32 SchemaVersion = 0;

	bool MigrateToLatestSchemaVersion();
	
	static int32 GetLatestSchemaVersion()
	{
		return 1;
	}

private:
	// Migrates task property storage from task-generated GUIDs to Blueprint compiler-generated GUIDs.
	void Migration_TaskProperties(); // TO DO: Reverify if it works correctly

	// Migrates the blueprint content serialization to use FProperty's ExportText / ImportText instead of JSON.
	void Migration_BlueprintContentSerialization();
};

UENUM(BlueprintType)
enum class EWireDrawingStyle : uint8
{
	Spline, // Spline curve
	Subway, // 45-degree angles
};

USTRUCT()
struct OPENLOGICV2_API FCachedWireData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
		FVector2D SourcePosition = FVector2D::Zero();

	UPROPERTY()
		FVector2D TargetPosition = FVector2D::Zero();

	// Tangent vector used in spline drawing
	UPROPERTY()
		FVector2D DirectionalTangent = FVector2D::Zero();

	// Points used for electronic wires
	UPROPERTY()
		TArray<FVector2D> Points;
};

USTRUCT()
struct OPENLOGICV2_API FOpenLogicGridCell
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
		TSet<UNodeBase*> CellNodes;
};

UENUM(BlueprintType)
enum class EPinConnectionValidity : uint8
{
	None,
	Compatible, // Connection can be made
	Replaceable, // Existing connection can be replaced
	RoleMismatch, // Pin roles do not match
	PropertyMismatch, // Property types do not match
	ConnectionLimitExceeded, // Maximum number of connections exceeded
	DirectionConflict, // Pins have the same direction
	IdenticalPin, // Attempting to connect the same pin
	IdenticalNode, // Attempting to connect the same node
};

UENUM()
enum class EOpenLogicTaskState : uint8
{
	None, // Task is not initialized
	Initialized, // Task has been initialized
	Running, // The task is currently running
	Evaluation, // This state only happens if ForceEvaluation is true in the task settings. This is used to keep up-to-date return values for pure nodes (without execution pins).
	Completed, // The task has completed its execution
};

UENUM(BlueprintType)
enum class EOpenLogicRuntimeThreadType : uint8
{
	GameThread,
	BackgroundThread
};

USTRUCT(BlueprintType)
struct OPENLOGICV2_API FOpenLogicThreadSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Thread Settings")
		EOpenLogicRuntimeThreadType NodeExecutionThread = EOpenLogicRuntimeThreadType::GameThread;
};

USTRUCT()
struct OPENLOGICV2_API FOpenLogicRuntimeProperty
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
		TSubclassOf<UOpenLogicProperty> PropertyClass;
	
public:
	FProperty* Property;
	void* ValuePtr;

	// Retrieves the value of the property.
	template<typename T>
	T GetValue() const
	{
		return *(T*)ValuePtr;
	}
};

USTRUCT()
struct OPENLOGICV2_API FOpenLogicRuntimeNode
{
	GENERATED_USTRUCT_BODY()

public:
	// The unique identifier of the node.
	UPROPERTY()
		FGuid NodeID;

	// The task instance of the node.
	UPROPERTY()
		UOpenLogicTask* TaskInstance;

	// The soft class pointer of the task.
	UPROPERTY()
		TSoftClassPtr<UOpenLogicTask> TaskClass;

	// The state of the task.
	UPROPERTY()
		EOpenLogicTaskState TaskState = EOpenLogicTaskState::None;

	// The number of input pins this node has.
	UPROPERTY()
		int32 InputPinsCount = 0;

	// The number of output pins this node has.
	UPROPERTY()
		int32 OutputPinsCount = 0;

	UPROPERTY()
		bool bReevaluateOnDemand = false;

	// The input properties of the node.
	TMap<int32, TSharedPtr<void>> InputProperties;

	// The output properties of the node.
	TMap<int32, TSharedPtr<void>> OutputProperties;
	
	bool IsValid() const
	{
		return NodeID.IsValid() && !TaskClass.IsNull();
	}
};

USTRUCT(BlueprintType)
struct OPENLOGICV2_API FOpenLogicGraphExecutionHandle
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
		int32 HandleIndex = INDEX_NONE;
	
	UPROPERTY()
		bool IsProcessed = false;

	UPROPERTY()
		bool IsRunning = false;
	
	UPROPERTY()
		TSoftClassPtr<UOpenLogicTask> TaskClass;

	UPROPERTY()
		FGuid NodeID;

	UPROPERTY()
		UOpenLogicRuntimeGraph* RuntimeGraph = nullptr;

	TMap<FGuid, TSharedPtr<FOpenLogicRuntimeNode>> RuntimeNodes;

	bool IsValid() const
	{
		return RuntimeGraph && NodeID.IsValid();
	}
};

USTRUCT()
struct OPENLOGICV2_API FOpenLogicQueuedExecutionHandle
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
		int32 HandleIndex = INDEX_NONE;
};

USTRUCT()
struct OPENLOGICV2_API FOpenLogicTaskPool
{
	GENERATED_USTRUCT_BODY()

	// Set of reusable task instances
	TSet<UOpenLogicTask*> AvailableTasks;

	// Set of task instances that are currently in use
	TSet<UOpenLogicTask*> ActiveTasks;

	// Retrieves a task from the pool or creates a new one if none are available
	UOpenLogicTask* GetTaskInstance(TSubclassOf<UOpenLogicTask> TaskClass, UObject* Outer);

	// Returns a task instance to the pool
	void ReturnTaskInstance(UOpenLogicTask* TaskInstance);
};

USTRUCT(BlueprintType)
struct OPENLOGICV2_API FOpenLogicPinHandle
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLogic)
		FName PinName;

	FOpenLogicPinHandle() = default;

	FOpenLogicPinHandle(const char* InPinName)
	{
		PinName = FName(InPinName);
	}
	
	FOpenLogicPinHandle(const FName& InPinName)
	{
		PinName = InPinName;
	}

	FOpenLogicPinHandle(const TCHAR* InPinName)
	{
		PinName = FName(InPinName);
	}
};

enum class EOpenLogicDefaultValueHandleType : uint8
{
	None,
#if WITH_EDITOR
	PropertyHandle,
#endif
	ExecutionPin
};

USTRUCT(BlueprintType)
struct OPENLOGICV2_API FOpenLogicDefaultValueHandle
{
	GENERATED_USTRUCT_BODY()

public:
	FOpenLogicDefaultValueHandle() = default;
	
#if WITH_EDITOR
	void SetPropertyHandle(TSharedPtr<IPropertyHandle> InPropertyHandle)
	{
		PropertyHandle = InPropertyHandle;
		HandleType = EOpenLogicDefaultValueHandleType::PropertyHandle;
	}
#endif

	void SetExecutionPin(UExecutionPinBase* InExecutionPin)
	{
		ExecutionPin = InExecutionPin;
		HandleType = EOpenLogicDefaultValueHandleType::ExecutionPin;
	}

public:
	// Sets the default value of the underlying property.
	// Returns true if the value was successfully set.
	template <typename T>
	bool SetDefaultValue(const T& InValue, bool bCommitChange = true)
	{
		FOpenLogicDefaultValue NewValue;
		NewValue.SetValue(InValue);

		CachedDefaultValue = NewValue;
		
		if (bCommitChange)
		{
			return CommitChange();
		}

		return true;
	}

	// Retrieves the default value of the underlying property.
	template <typename T>
	T GetDefaultValue() const
	{
		T Value;
		GetNodeValue().GetValue<T>(Value);

		return Value;
	}

	bool CommitChange();

public:
	// Checks if the handle is valid.
	bool IsValid() const;

protected:
	FOpenLogicDefaultValue GetNodeValue() const;
	
private:
#if WITH_EDITOR
	TSharedPtr<IPropertyHandle> PropertyHandle;
#endif

	UPROPERTY()
	UExecutionPinBase* ExecutionPin;
	
	EOpenLogicDefaultValueHandleType HandleType = EOpenLogicDefaultValueHandleType::None;
	FOpenLogicDefaultValue CachedDefaultValue;
};

USTRUCT(BlueprintType)
struct OPENLOGICV2_API FOpenLogicPinValueDisplay
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, Category = OpenLogic)
		UWidget* Widget = nullptr;

	// Whether the widget should be displayed inline with the pin name or below it.
	UPROPERTY(BlueprintReadWrite, Category = OpenLogic)
		bool bDisplayWidgetInline = true;
};