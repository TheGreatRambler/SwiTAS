#pragma once

#define BIT(n) (1U << (n))

#include "include/zpp.hpp"
#include <cstdint>
#include <memory>
#include <unordered_map>

#include "buttonData.hpp"

// clang-format off
#define DEFINE_STRUCT(Flag, body, ...) \
	struct Struct_##Flag : public zpp::serializer::polymorphic { \
		DataFlag flag = DataFlag::Flag; \
		body \
		friend zpp::serializer::access; \
		template <typename Archive, typename Self> static void serialize(Archive& archive, Self& self) { \
			archive(__VA_ARGS__); \
		} \
	};
// clang-format on

enum DataFlag : uint8_t {
	SendFrameData,
	RecieveGameFramebuffer,
	RecieveGameInfo,
	SendFlag,
	SendAutoRun,
	SendLogging,
	SendTrackMemoryRegion,
	SendSetNumControllers,
	SendAddMemoryRegion,
	SendStartFinalTas,
	RecieveMemoryRegion,
	RecieveLogging,
	RecieveFlag,
	RecieveApplicationConnected,
	RecieveGameMemoryInfo,
	RecieveAutoRunControllerData,
	SendFinalTasChunk,
	NUM_OF_FLAGS,
};

enum RecieveInfo : uint8_t {
	RUN_FRAME_DONE,
	FRAMEBUFFER_DONE,
	APPLICATION_DISCONNECTED,
	CONTROLLERS_CONNECTED,
	UNEXPECTED_CONTROLLER_SIZE,
	IS_YUZU,
	IS_HOMEBREW,
};

enum SendInfo : uint8_t {
	GET_FRAMEBUFFER,
	GET_GAME_INFO,
	START_TAS_MODE,
	END_TAS_MODE,
	PAUSE_DEBUG,
	UNPAUSE_DEBUG,
	// These methods aren't debug because they are called by savestateHandler
	PAUSE,
	UNPAUSE,
	RUN_BLANK_FRAME,
	STOP_FULL_SPEED,
	PAUSE_FULL_SPEED,
	STOP_FINAL_TAS,
	GET_IS_YUZU,
};

// This is used by the switch to determine size, a vector is always send back enyway
enum MemoryRegionTypes : uint8_t {
	Bit8 = 0,
	Bit16,
	Bit32,
	Bit64,
	Float,
	Double,
	Bool,
	CharPointer,
	ByteArray,
	NUM_OF_TYPES,
};

namespace MemoryDataInfo {
	// Shamelessly copied from libnx
	/// Memory type enumeration (lower 8 bits of \ref MemoryState)
	typedef enum {
		MemType_Unmapped            = 0x00, ///< Unmapped memory.
		MemType_Io                  = 0x01, ///< Mapped by kernel capability parsing in \ref svcCreateProcess.
		MemType_Normal              = 0x02, ///< Mapped by kernel capability parsing in \ref svcCreateProcess.
		MemType_CodeStatic          = 0x03, ///< Mapped during \ref svcCreateProcess.
		MemType_CodeMutable         = 0x04, ///< Transition from MemType_CodeStatic performed by \ref svcSetProcessMemoryPermission.
		MemType_Heap                = 0x05, ///< Mapped using \ref svcSetHeapSize.
		MemType_SharedMem           = 0x06, ///< Mapped using \ref svcMapSharedMemory.
		MemType_WeirdMappedMem      = 0x07, ///< Mapped using \ref svcMapMemory.
		MemType_ModuleCodeStatic    = 0x08, ///< Mapped using \ref svcMapProcessCodeMemory.
		MemType_ModuleCodeMutable   = 0x09, ///< Transition from \ref MemType_ModuleCodeStatic performed by \ref svcSetProcessMemoryPermission.
		MemType_IpcBuffer0          = 0x0A, ///< IPC buffers with descriptor flags=0.
		MemType_MappedMemory        = 0x0B, ///< Mapped using \ref svcMapMemory.
		MemType_ThreadLocal         = 0x0C, ///< Mapped during \ref svcCreateThread.
		MemType_TransferMemIsolated = 0x0D, ///< Mapped using \ref svcMapTransferMemory when the owning process has perm=0.
		MemType_TransferMem         = 0x0E, ///< Mapped using \ref svcMapTransferMemory when the owning process has perm!=0.
		MemType_ProcessMem          = 0x0F, ///< Mapped using \ref svcMapProcessMemory.
		MemType_Reserved            = 0x10, ///< Reserved.
		MemType_IpcBuffer1          = 0x11, ///< IPC buffers with descriptor flags=1.
		MemType_IpcBuffer3          = 0x12, ///< IPC buffers with descriptor flags=3.
		MemType_KernelStack         = 0x13, ///< Mapped in kernel during \ref svcCreateThread.
		MemType_CodeReadOnly        = 0x14, ///< Mapped in kernel during \ref svcControlCodeMemory.
		MemType_CodeWritable        = 0x15, ///< Mapped in kernel during \ref svcControlCodeMemory.
	} MemoryType;

	static std::unordered_map<MemoryType, std::string> memoryTypeName {
		{ MemType_Unmapped, "Unmapped" },
		{ MemType_Io, "Io" },
		{ MemType_Normal, "Normal" },
		{ MemType_CodeStatic, "CodeStatic" },
		{ MemType_CodeMutable, "CodeMutable" },
		{ MemType_Heap, "Heap" },
		{ MemType_SharedMem, "SharedMem" },
		{ MemType_WeirdMappedMem, "WeirdMappedMem" },
		{ MemType_ModuleCodeStatic, "ModuleCodeStatic" },
		{ MemType_ModuleCodeMutable, "ModuleCodeMutable" },
		{ MemType_IpcBuffer0, "IpcBuffer0" },
		{ MemType_MappedMemory, "MappedMemory" },
		{ MemType_ThreadLocal, "ThreadLocal" },
		{ MemType_TransferMemIsolated, "TransferMemIsolated" },
		{ MemType_TransferMem, "TransferMem" },
		{ MemType_ProcessMem, "ProcessMem" },
		{ MemType_Reserved, "Reserved" },
		{ MemType_IpcBuffer1, "IpcBuffer1" },
		{ MemType_IpcBuffer3, "IpcBuffer3" },
		{ MemType_KernelStack, "KernelStack" },
		{ MemType_CodeReadOnly, "CodeReadOnly" },
		{ MemType_CodeWritable, "CodeWritable" },
	};

	/// Memory state bitmasks.
	typedef enum {
		MemState_Type                       = 0xFF,                     ///< Type field (see \ref MemoryType).
		MemState_PermChangeAllowed          = BIT(8),                   ///< Permission change allowed.
		MemState_ForceRwByDebugSyscalls     = BIT(9),                   ///< Force read/writable by debug syscalls.
		MemState_IpcSendAllowed_Type0       = BIT(10),                  ///< IPC type 0 send allowed.
		MemState_IpcSendAllowed_Type3       = BIT(11),                  ///< IPC type 3 send allowed.
		MemState_IpcSendAllowed_Type1       = BIT(12),                  ///< IPC type 1 send allowed.
		MemState_ProcessPermChangeAllowed   = BIT(14),                  ///< Process permission change allowed.
		MemState_MapAllowed                 = BIT(15),                  ///< Map allowed.
		MemState_UnmapProcessCodeMemAllowed = BIT(16),                  ///< Unmap process code memory allowed.
		MemState_TransferMemAllowed         = BIT(17),                  ///< Transfer memory allowed.
		MemState_QueryPAddrAllowed          = BIT(18),                  ///< Query physical address allowed.
		MemState_MapDeviceAllowed           = BIT(19),                  ///< Map device allowed (\ref svcMapDeviceAddressSpace and \ref svcMapDeviceAddressSpaceByForce).
		MemState_MapDeviceAlignedAllowed    = BIT(20),                  ///< Map device aligned allowed.
		MemState_IpcBufferAllowed           = BIT(21),                  ///< IPC buffer allowed.
		MemState_IsPoolAllocated            = BIT(22),                  ///< Is pool allocated.
		MemState_IsRefCounted               = MemState_IsPoolAllocated, ///< Alias for \ref MemState_IsPoolAllocated.
		MemState_MapProcessAllowed          = BIT(23),                  ///< Map process allowed.
		MemState_AttrChangeAllowed          = BIT(24),                  ///< Attribute change allowed.
		MemState_CodeMemAllowed             = BIT(25),                  ///< Code memory allowed.
	} MemoryState;

	static std::unordered_map<MemoryState, std::string> memoryStateName {
		{ MemState_Type, "Type" },
		{ MemState_PermChangeAllowed, "PermChangeAllowed" },
		{ MemState_ForceRwByDebugSyscalls, "ForceRwByDebugSyscalls" },
		{ MemState_IpcSendAllowed_Type0, "IpcSendAllowed_Type0" },
		{ MemState_IpcSendAllowed_Type3, "IpcSendAllowed_Type3" },
		{ MemState_IpcSendAllowed_Type1, "IpcSendAllowed_Type1" },
		{ MemState_ProcessPermChangeAllowed, "ProcessPermChangeAllowed" },
		{ MemState_MapAllowed, "MapAllowed" },
		{ MemState_UnmapProcessCodeMemAllowed, "UnmapProcessCodeMemAllowed" },
		{ MemState_TransferMemAllowed, "TransferMemAllowed" },
		{ MemState_QueryPAddrAllowed, "QueryPAddrAllowed" },
		{ MemState_MapDeviceAllowed, "MapDeviceAllowed" },
		{ MemState_MapDeviceAlignedAllowed, "MapDeviceAlignedAllowed" },
		{ MemState_IpcBufferAllowed, "IpcBufferAllowed" },
		{ MemState_IsPoolAllocated, "IsPoolAllocated" },
		{ MemState_IsRefCounted, "IsRefCounted" },
		{ MemState_MapProcessAllowed, "MapProcessAllowed" },
		{ MemState_AttrChangeAllowed, "AttrChangeAllowed" },
		{ MemState_CodeMemAllowed, "CodeMemAllowed" },
	};

	/// Memory attribute bitmasks.
	typedef enum {
		MemAttr_IsBorrowed     = BIT(0), ///< Is borrowed memory.
		MemAttr_IsIpcMapped    = BIT(1), ///< Is IPC mapped (when IpcRefCount > 0).
		MemAttr_IsDeviceMapped = BIT(2), ///< Is device mapped (when DeviceRefCount > 0).
		MemAttr_IsUncached     = BIT(3), ///< Is uncached.
	} MemoryAttribute;

	static std::unordered_map<MemoryAttribute, std::string> memoryAttributeName {
		{ MemAttr_IsBorrowed, "IsBorrowed" },
		{ MemAttr_IsIpcMapped, "IsIpcMapped" },
		{ MemAttr_IsDeviceMapped, "IsDeviceMapped" },
		{ MemAttr_IsUncached, "IsUncached" },
	};

	/// Memory permission bitmasks.
	typedef enum {
		Perm_None     = 0,               ///< No permissions.
		Perm_R        = BIT(0),          ///< Read permission.
		Perm_W        = BIT(1),          ///< Write permission.
		Perm_X        = BIT(2),          ///< Execute permission.
		Perm_Rw       = Perm_R | Perm_W, ///< Read/write permissions.
		Perm_Rx       = Perm_R | Perm_X, ///< Read/execute permissions.
		Perm_DontCare = BIT(28),         ///< Don't care
	} Permission;

	static std::unordered_map<Permission, std::string> memoryPermissionName {
		{ Perm_None, "None" },
		{ Perm_R, "Read" },
		{ Perm_W, "Write" },
		{ Perm_X, "Execute" },
		{ Perm_Rw, "ReadWrite" },
		{ Perm_Rx, "ReadExecute" },
		{ Perm_DontCare, "DontCare" },
	};

	/// Memory information structure.
	typedef struct {
		uint64_t addr;            ///< Base address.
		uint64_t size;            ///< Size.
		uint32_t type;            ///< Memory type (see lower 8 bits of \ref MemoryState).
		uint32_t attr;            ///< Memory attributes (see \ref MemoryAttribute).
		uint32_t perm;            ///< Memory permissions (see \ref Permission).
		uint32_t device_refcount; ///< Device reference count.
		uint32_t ipc_refcount;    ///< IPC reference count.

		friend zpp::serializer::access;
		template <typename Archive, typename Self> static void serialize(Archive& archive, Self& self) {
			archive(self.addr, self.size, self.type, self.attr, self.perm, self.device_refcount, self.ipc_refcount);
		}
	} MemoryInfo;
};

// clang-format off
namespace Protocol {
	// Run a single frame and return when done
	DEFINE_STRUCT(SendFrameData,
		ControllerData controllerData;
		TouchAndKeyboardData extraData;
		// This is the frame of this frame, the framebuffer
		// sent back will be one more than this frame
		uint32_t frame;
		uint16_t savestateHookNum;
		uint16_t branchIndex;
		uint8_t playerIndex;
		uint8_t incrementFrame;
		uint8_t includeFramebuffer;
		TasValueToRecord typeToRecord;
	, self.controllerData, self.extraData, self.frame, self.playerIndex, self.incrementFrame, self.branchIndex, self.savestateHookNum, self.includeFramebuffer, self.isAutoRun)

	// Recieve all of the game's framebuffer
	DEFINE_STRUCT(RecieveGameFramebuffer,
		std::vector<uint8_t> buf;
		uint8_t fromFrameAdvance;
		uint32_t frame;
		uint16_t savestateHookNum;
		uint16_t branchIndex;
		uint8_t playerIndex;
		// Set by auto advance
		TasValueToRecord valueIncluded;
		ControllerData controllerData;
		TouchAndKeyboardData extraData;
	, self.buf, self.fromFrameAdvance, self.frame, self.savestateHookNum, self.branchIndex, self.playerIndex, self.controllerDataIncluded, self.controllerData)

	// Recieve a ton of game and user info
	DEFINE_STRUCT(RecieveGameInfo,
		std::string applicationName;
		uint64_t applicationProgramId;
		uint64_t applicationProcessId;
		std::string userNickname;
		std::vector<MemoryDataInfo::MemoryInfo> memoryInfo;
	, self.applicationName, self.applicationProgramId, self.applicationProcessId, self.userNickname, self.memoryInfo)

	// Send start, with mostly everything as an enum value
	DEFINE_STRUCT(SendFlag,
		SendInfo actFlag;
	, self.actFlag)

	// Needs to have number of controllers set right, TODO
	DEFINE_STRUCT(SendStartFinalTas,
		std::vector<std::string> scriptPaths;
	, self.scriptPaths)

	DEFINE_STRUCT(SendFinalTasChunk,
		std::string path;
		std::vector<uint8_t> contents;
		uint8_t openFile;
		uint8_t closeFile;
	, self.path, self.contents, self.openFile, self.closeFile)

	DEFINE_STRUCT(SendLogging,
		std::string log;
	, self.log)

	DEFINE_STRUCT(SendTrackMemoryRegion,
		uint64_t startByte;
		uint64_t size;
	, self.startByte, self.size)

	DEFINE_STRUCT(SendAddMemoryRegion,
		std::string pointerDefinition;
		MemoryRegionTypes type;
		uint8_t clearAllRegions;
		uint8_t u;
		uint64_t dataSize;
	, self.pointerDefinition, self.type, self.clearAllRegions, self.u, self.dataSize)

	DEFINE_STRUCT(SendSetNumControllers,
		uint8_t size;
	, self.size)

	DEFINE_STRUCT(RecieveMemoryRegion,
		std::vector<uint8_t> memory;
		std::string stringRepresentation;
		uint16_t index;
	, self.memory, self.stringRepresentation, self.index)

	DEFINE_STRUCT(RecieveLogging,
		std::string log;
	, self.log)

	// Recieve done, with mostly everything as an enum value
	DEFINE_STRUCT(RecieveFlag,
		RecieveInfo actFlag;
	, self.actFlag)

	DEFINE_STRUCT(RecieveApplicationConnected,
		std::string applicationName;
		uint64_t applicationProgramId;
		uint64_t applicationProcessId;
	, self.applicationName, self.applicationProgramId, self.applicationProcessId)
};
// clang-format on