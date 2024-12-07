#include "FicsItNetworksDocumentation.h"
#include "FicsItReflection.h"
#include "Package.h"
#include "Paths.h"
#include "FINLua/FINLuaModule.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Reflection/FIRArrayProperty.h"
#include "Reflection/FIRClass.h"
#include "Reflection/FIRClassProperty.h"
#include "Reflection/FIRObjectProperty.h"
#include "Reflection/FIRStructProperty.h"
#include "Reflection/FIRTraceProperty.h"

UE_DISABLE_OPTIMIZATION_SHIP

namespace FINGenLuaDocSumneko {
	inline const auto MiscDocumentation = TEXT(
		R"(error("I don't know what your misson is. But is file is not meant to be executed in any way. It's a meta file.")
---@meta

---@class FIN.classes
classes = {}

---@class FIN.structs
structs = {}

-- some more FicsIt-Networks things to support more type specific things and also adds documentation for `computer`, `component`, `event` and `filesystem` libraries in FicsIt-Networks (keep in mind this is all written by hand and can maybe not represent all features available)

--- # Not in FicsIt-Networks available #
package = nil

--- # Not in FicsIt-Networks available #
os = nil

--- # Not in FicsIt-Networks available #
collectgarbage = nil

--- # Not in FicsIt-Networks available #
io = nil

--- # Not in FicsIt-Networks available #
arg = nil

--- # Not in FicsIt-Networks available #
require = nil

---@class FIN.UUID : string


-- adding alias to make more descriptive correct naming and more plausible when using `computer.getPCIDevice()`

---@alias FIN.PCIDevice FIN.FINComputerModule

---@class Engine.Object
local Object = {}

--- The network id of this component.
---
--- ## Only on objects that are network components.
--- ### Flags:
--- * ? Runtime Synchronous - Can be called/changed in Game Tick ?
--- * ? Runtime Parallel - Can be called/changed in Satisfactory Factory Tick ?
--- * Read Only - The value of this property can not be changed by code
---@type FIN.UUID
Object.id = nil

--- The nick you gave the component in the network its in.
--- If it has no nick name it returns `nil`.
---
--- ## Only on objects that are network components.
--- ### Flags:
--- * ? Runtime Synchronous - Can be called/changed in Game Tick ?
--- * ? Runtime Parallel - Can be called/changed in Satisfactory Factory Tick ?
--- * Read Only - The value of this property can not be changed by code
---@type string
Object.nick = nil

-- global functions from FicsIt-Networks

--- Tries to find the item with the provided name.
---@param name string
---@return Engine.Object
function findItem(name) end

--- Tries to find the items or item provided via name.
---@param ... string
---@return Engine.Object[]
function getItems(...) end

---@param seconds number
function sleep(seconds) end

---@param func function Wraps the given thread/coroutine in a Lua-Future
---@return FIN.Future
function async(func) end

--- Allows to log the given strings to the Game Log.
---@param ... any A list of log messages that should get printed to the game console.
function debug.log(...) end

)");

	inline const auto FutureApiDocumentation = TEXT(R"(
---@class FIN.Future
local Future = {}

--- Gets the data.
---@return any ...
function Future:get() end

---@return boolean success, number timeout
function Future:poll() end

--- Waits for the future to finish processing and returns the result.
---@async
---@return any ...
function Future:await() end

--- Checks if the Future is done processing.
---@return boolean isDone
function Future:canGet() end

--- **FicsIt-Networks Lua Lib:** `future`
---
--- This Module provides the Future type and all its necessary functionallity.
---@class FIN.Future.Api
future = {}

--- Wraps the given thread/coroutine in a Lua-Future
---@param thread thread
---@return FIN.Future
function future.async(thread) end

--- Creates a new Future that will only finish once all futures passed as parameters have finished.
---@param ... FIN.Future
---@return FIN.Future The Future that will finish once all other futures finished.
function future.join(...) end

--- Creates a future that returns after the given amount of seconds.
---@return FIN.Future
function future.sleep(seconds) end

--- A list of futures that are considered "Tasks".
--- Tasks could be seen as background threads. Effectively getting "joined" together.
--- Examples for tasks are callback invocations of timers and event listeners.
---@type FIN.Future[]
future.tasks = nil

--- Adds the given futures to the tasks list.
---@param ... FIN.Future
function future.addTask(...) end

--- Runs the default task scheduler once.
---@return integer left
---@return number timeout
function future.run() end

--- Runs the default task scheduler indefinitely until no tasks are left.
function future.loop() end

)");

	inline const auto EventApiDocumentation = TEXT(R"(
--- **FicsIt-Networks Lua Lib:** `event`
---
--- The Event API provides classes, functions and variables for interacting with the component network.
---@class FIN.Event.Api
event = {}

--- Adds the running lua context to the listen queue of the given component.
---@param component Engine.Object - The network component lua representation the computer should now listen to.
function event.listen(component) end

--- Returns all signal senders this computer is listening to.
---@return Engine.Object[] components - An array containing instances to all sginal senders this computer is listening too.
function event.listening() end

--- Waits for a signal in the queue. Blocks the execution until a signal got pushed to the signal queue, or the timeout is reached.
--- Returns directly if there is already a signal in the queue (the tick doesn’t get yielded).
---@param timeoutSeconds number? - The amount of time needs to pass until pull unblocks when no signal got pushed.
---@return string signalName - The name of the returned signal.
---@return Engine.Object component - The component representation of the signal sender.
---@return any ... - The parameters passed to the signal.
function event.pull(timeoutSeconds) end

--- Removes the running lua context from the listen queue of the given components. Basically the opposite of listen.
---@param component Engine.Object - The network component lua representations the computer should stop listening to.
function event.ignore(component) end

--- Stops listening to any signal sender. If afterwards there are still coming signals in, it might be the system itself or caching bug.
function event.ignoreAll() end

--- Clears every signal from the signal queue.
function event.clear() end

---@alias FIN.EventFilter.Supported_Types
---|nil
---|boolean
---|number
---|string
---|Engine.Object
---|FIN.Struct
---|FIN.Class
---|FIN.EventFilter.Supported_Types[]

---@class FIN.EventFilter.Config
---@field event string | string[] | nil
---@field sender Engine.Object | Engine.Object[] | nil
---@field values table<string, FIN.EventFilter.Supported_Types>

--- Creates an Event filter expression.
---@param filter FIN.EventFilter.Config
---@return FIN.EventFilter
function event.filter(filter) end

--- Registers the given function as a listener.
--- When `event.pull()` pulls a signal from the queue, that matches the given Event-Filter,
--- a Task will be created using the function and the signals parameters will be passed into the function.
---@param filter FIN.EventFilter
---@param func fun(event: string, sender: Engine.Object, ...: any)
function event.registerListener(filter, func) end

---@class FIN.EventQueue
local event_queue = {}

---@param timeout number
---@return string event
---@return Engine.Object sender
---@return any ...
function event_queue:pull(timeout) end

--- Returns a Future that resolves when a signal got added to the queue that matches the given Event Filter.
---@param filter FIN.EventFilter
---@return FIN.Future
function event_queue:wait_for(filter) end

--- Creates a new event queue.
--- When this variable closes or gets garbage collected, it will stop receiving signals.
---@param filter FIN.EventFilter
function event.queue(filter) end

--- Returns a Future that resolves when a signal got polled that matches the given Event Filter.
---@param filter FIN.EventFilter
---@return FIN.Future
function event:wait_for(filter) end

--- Runs an infinite loop or `future.run()`, `event.pull(0)` and `coroutine.yield()`.
function event.loop() end
)");

	inline const auto ComponentApiDocumentation = TEXT(R"(
--- **FicsIt-Networks Lua Lib:** `component`
---
--- The Component API provides structures, functions and signals for interacting with the network itself like returning network components.
---@class FIN.Component.Api
component = {}


--- Generates and returns instances of the network component with the given UUID.
--- If a network component cannot be found for a given UUID, nil will be used for the return. Otherwise, an instance of the network component will be returned.
---@generic T : Engine.Object
---@param id FIN.UUID - UUID of a network component.
---@return T? component
function component.proxy(id) end

--- Generates and returns instances of the network components with the given UUIDs.
--- You can pass any amount of parameters and each parameter will then have a corresponding return value.
--- If a network component cannot be found for a given UUID, nil will be used for the return. Otherwise, an instance of the network component will be returned.
---@generic T : Engine.Object
---@param ... FIN.UUID - UUIDs
---@return T? ... - components
function component.proxy(...) end

--- Generates and returns instances of the network components with the given UUIDs.
--- You can pass any amount of parameters and each parameter will then have a corresponding return value.
--- If a network component cannot be found for a given UUID, nil will be used for the return. Otherwise, an instance of the network component will be returned.
---@generic T : Engine.Object
---@param ids FIN.UUID[]
---@return T[] components
function component.proxy(ids) end

--- Generates and returns instances of the network components with the given UUIDs.
--- You can pass any amount of parameters and each parameter will then have a corresponding return value.
--- If a network component cannot be found for a given UUID, nil will be used for the return. Otherwise, an instance of the network component will be returned.
---@generic T : Engine.Object
---@param ... FIN.UUID[]
---@return T[] ... - components
function component.proxy(...) end

--- Searches the component network for components with the given query.
---@param query string
---@return FIN.UUID[] UUIDs
function component.findComponent(query) end

--- Searches the component network for components with the given query.
--- You can pass multiple parameters and each parameter will be handled separately and returns a corresponding return value.
---@param ... string - querys
---@return FIN.UUID[] ... - UUIDs
function component.findComponent(...) end

--- Searches the component network for components with the given type.
---@param type Engine.Object
---@return FIN.UUID[] UUIDs
function component.findComponent(type) end

--- Searches the component network for components with the given type.
--- You can pass multiple parameters and each parameter will be handled separately and returns a corresponding return value.
---@param ... Engine.Object - classes to search for
---@return FIN.UUID[] ... - UUIDs
function component.findComponent(...) end
)");

	inline const auto ComputerApiDocumentation = TEXT(R"(
--- **FicsIt-Networks Lua Lib:** `computer`
---
--- The Computer API provides a interface to the computer owns functionalities.
---@class FIN.Computer.Api
computer = {}

--- This function is mainly used to allow switching to a higher tick runtime state. Usually you use this when you want to make your code run faster when using functions that can run in asynchronous environment.
function computer.skip() end

--- Returns some kind of strange/mysterious time data from a unknown place (the real life).
---@return integer Timestamp - Unix Timestamp
---@return string DateTimeStamp - Serverside Formatted Date-Time-Stamp
---@return string DateTimeStamp - Date-Time-Stamp after ISO 8601
function computer.magicTime() end

--- Returns the current memory usage
---@return integer usage
---@return integer capacity
function computer.getMemory() end

--- Returns the current computer case instance
---@return FIN.ComputerCase
function computer.getInstance() end

--- This function allows you to get all installed PCI-Devices in a computer of a given type.
---@generic TPCIDevice : FIN.PCIDevice
---@param type TPCIDevice
---@return TPCIDevice[]
function computer.getPCIDevices(type) end

--- Returns the amount of milliseconds passed since the system started.
---@return integer milliseconds - Amount of milliseconds since system start
function computer.millis() end

--- Stops the current code execution immediately and queues the system to restart in the next tick.
function computer.reset() end

--- Stops the current code execution.
--- Basically kills the PC runtime immediately.
function computer.stop() end

--- Sets the code of the current eeprom. Doesn’t cause a system reset.
---@param code string - The code you want to place into the eeprom.
function computer.setEEPROM(code) end

--- Returns the code the current eeprom contents.
---@return string code - The code in the EEPROM
function computer.getEEPROM() end

--- Lets the computer emit a simple beep sound with the given pitch.
---@param pitch number - The pitch of the beep sound you want to play.
function computer.beep(pitch) end

--- Crashes the computer with the given error message.
---@param errorMsg string - The crash error message you want to use
function computer.panic(errorMsg) end

--- Shows a text notification to the player. If player is `nil` to all players.
---@param text string
---@param playerName string?
function computer.textNotification(text, playerName) end

--- Creates an attentionPing at the given position to the player. If player is `nil` to all players.
---@param position Engine.Vector
---@param playerName string?
function computer.attentionPing(position, playerName) end

--- Returns the number of game seconds passed since the save got created. A game day consists of 24 game hours, a game hour consists of 60 game minutes, a game minute consists of 60 game seconds.
---@return number time - The number of game seconds passed since the save got created.
function computer.time() end

--- Does the same as computer.skip
function computer.promote() end

--- Reverts effects of skip
function computer.demote() end

--- Returns `true` if the tick state is to higher
---@return boolean isPromoted
function computer.isPromoted() end

---@alias FIN.LogEntry.Verbosity
---|0 Debug
---|1 Info
---|2 Warning
---|3 Error
---|4 Fatal

---@param verbosity FIN.LogEntry.Verbosity
---@param format string
---@param ... any
function computer.log(verbosity, format, ...) end

--- Field containing a reference to the Media Subsystem.
---@type FIN.FINMediaSubsystem
computer.media = nil
)");

	inline const auto FileSystemApiDocumentationPart1 = TEXT(R"(
--- **FicsIt-Networks Lua Lib:** `filesystem`
---
--- The filesystem api provides structures, functions and variables for interacting with the virtual file systems.
---
--- You can’t access files outside the virtual filesystem. If you try to do so, the Lua runtime crashes.
---@class FIN.Filesystem.Api
filesystem = {}

---@alias FIN.Filesystem.File.Openmode
---|"r" read only -> file stream can just read from file. If file doesn’t exist, will return nil
---|"w" write -> file stream can read and write creates the file if it doesn’t exist
---|"a" end of file -> file stream can read and write cursor is set to the end of file
---|"+r" truncate -> file stream can read and write all previous data in file gets dropped
---|"+a" append -> file stream can read the full file but can only write to the end of the existing file

--- Opens a file-stream and returns it as File-table.
---@param path string
---@param mode FIN.Filesystem.File.Openmode
---@return FIN.Filesystem.File File
function filesystem.open(path, mode) end

--- Creates the folder path.
---@param path string - folder path the function should create
---@param all boolean? - if true creates all sub folders from the path
---@return boolean success - returns `true` if it was able to create the directory
function filesystem.createDir(path, all) end

--- Removes the filesystem object at the given path.
---@param path string - path to the filesystem object
---@param all boolean? - if true deletes everything
---@return boolean success - returns `true` if it was able to remove the node
function filesystem.remove(path, all) end

--- Moves the filesystem object from the given path to the other given path.
--- Function fails if it is not able to move the object.
---@param from string - path to the filesystem object you want to move
---@param to string - path to the filesystem object the target should get moved to
---@return boolean success - returns `true` if it was able to move the node
function filesystem.move(from, to) end

--- Renames the filesystem object at the given path to the given name.
---@param path string - path to the filesystem object you want to rename
---@param name string - the new name for your filesystem object
---@return boolean success - returns true if it was able to rename the node
function filesystem.rename(path, name) end

--- Checks if the given path exists.
---@param path string - path you want to check if it exists
---@return boolean exists - true if given path exists
function filesystem.exists(path) end

--- Lists all children of this node. (f.e. items in a folder)
---@param path string - path to the filesystem object you want to get the childs from
---@return string[] childs - array of string which are the names of the childs
function filesystem.childs(path) end

---@deprecated
--- Lists all children of this node. (f.e. items in a folder)
---@param path string - path to the filesystem object you want to get the childs from
---@return string[] childs - array of string which are the names of the childs
function filesystem.children(path) end

--- Checks if path refers to a file.
---@param path string - path you want to check if it refers to a file
---@return boolean isFile - true if path refers to a file
function filesystem.isFile(path) end

--- Checks if given path refers to a directory.
---@param path string - path you want to check if it refers to a directory
---@return boolean isDir - returns true if path refers to a directory
function filesystem.isDir(path) end

--- This function mounts the device referenced by the the path to a device node to the given mount point.
---@param device string - the path to the device you want to mount
---@param mountPoint string - the path to the point were the device should get mounted to
function filesystem.mount(device, mountPoint) end

--- This function unmounts the device referenced to the the given mount point.
---@param mountPoint string - the path to the point were the device is referenced to
function filesystem.unmount(mountPoint) end

--- Executes Lua code in the file referd by the given path.
--- Function fails if path doesn’t exist or path doesn’t refer to a file.
---@param path string - path to file you want to execute as Lua code
---@return any ... - Returned values from executed file.
function filesystem.doFile(path) end

--- Loads the file refered by the given path as a Lua function and returns it.
--- Functions fails if path doesn’t exist or path doesn’t reger to a file.
---@param path string - path to the file you want to load as Lua function
---@return function loadedFunction - the file compiled as Lua function
function filesystem.loadFile(path) end
)");

	inline const auto FileSystemApiDocumentationPart2 = TEXT(R"(
---@alias FIN.Filesystem.PathParameters
---|0 Normalize the path. -> /my/../weird/./path → /weird/path
---|1 Normalizes and converts the path to an absolute path. -> my/abs/path → /my/abs/path
---|2 Normalizes and converts the path to an relative path. -> /my/relative/path → my/relative/path
---|3 Returns the whole file/folder name. -> /path/to/file.txt → file.txt
---|4 Returns the stem of the filename. -> /path/to/file.txt → file || /path/to/.file → .file
---|5 Returns the file-extension of the filename. -> /path/to/file.txt → .txt || /path/to/.file → empty-str || /path/to/file. → .

--- Combines a variable amount of strings as paths together.
---@param ... string - paths to be combined
---@return string path - the final combined path
function filesystem.path(...) end

--- Combines a variable amount of strings as paths together to one big path.
--- Additionally, applies given conversion.
---@param parameter FIN.Filesystem.PathParameters - defines a conversion that should get applied to the output path.
---@param ... string - paths to be combined
---@return string path - the final combined and converted output path
function filesystem.path(parameter, ...) end

---@alias FIN.Filesystem.PathRegister
---|1 Is filesystem root
---|2 Is Empty (includes if it is root-path)
---|4 Is absolute path
---|8 Is only a file/folder name
---|16 Filename has extension
---|32 Ends with a / → refers a directory

--- Will be checked for lexical features.
--- Return value which is a bit-flag-register describing those lexical features.
---@param path string - filesystem-path you want to get lexical features from. 
---@return FIN.Filesystem.PathRegister BitRegister - bit-register describing the features of each path
function filesystem.analyzePath(path) end

--- Each string will be viewed as one filesystem-path and will be checked for lexical features.
--- Each of those string will then have a integer return value which is a bit-flag-register describing those lexical features.
---@param ... string - filesystem-paths you want to get lexical features from.
---@return FIN.Filesystem.PathRegister ... - bit-registers describing the features of each path
function filesystem.analyzePath(...) end

--- For given string, returns a bool to tell if string is a valid node (file/folder) name.
---@param node string - node-name you want to check.
---@return boolean isNode - True if node is a valid node-name.
function filesystem.isNode(node) end

--- For each given string, returns a bool to tell if string is a valid node (file/folder) name.
---@param ... string - node-names you want to check.
---@return boolean ... - True if the corresponding string is a valid node-name.
function filesystem.isNode(...) end

---@alias FIN.Filesystem.Meta
---|"File" A normal File
---|"Directory" A directory or folder that can hold multiple nodes.
---|"Device" A special type of Node that represents a filesystem and can be mounted.
---|"Unknown" The node type is not known to this utility function.

--- Returns for each given string path, a table that defines contains some meta information about node the string references.
---@param ... string
---@return { type: FIN.Filesystem.Meta } ...
function filesystem.meta(...) end
)");

	inline const auto FileApiDocumentation = TEXT(R"(
---@class FIN.Filesystem.File
local File = {}

---@param data string
function File:write(data) end

---@param length integer
function File:read(length) end

---@alias FIN.Filesystem.File.SeekMode
---|"set" # Base is beginning of the file.
---|"cur" # Base is current position.
---|"end" # Base is end of file.

---@param mode FIN.Filesystem.File.SeekMode
---@param offset integer
---@return integer offset
function File:seek(mode, offset) end

function File:close() end
)");

	const FString Manual_FINGenLuaSumnekoDocumentation = FString::Printf(TEXT("%s\n%s\n%s\n%s\n%s\n%s%s\n%s"),
	                                                                     MiscDocumentation, FutureApiDocumentation,
	                                                                     EventApiDocumentation,
	                                                                     ComponentApiDocumentation,
	                                                                     ComputerApiDocumentation,
	                                                                     FileSystemApiDocumentationPart1,
	                                                                     FileSystemApiDocumentationPart2,
	                                                                     FileApiDocumentation);

	FString FINGenLuaSumnekoGetTypeName(const UFIRBase *Base) {
		TArray<FString> SplitBasePackageName;
		Base->GetPackage()->GetName().ParseIntoArray(SplitBasePackageName, TEXT("/"), true);

		FString BasePackageName;
		for (auto &NamePart : SplitBasePackageName) {
			if (NamePart.Equals(TEXT("Script"))) {
				continue; // check next name part
			}
			
			// we only want the the first not filtered out name should be the mod name
			if (NamePart.Equals(TEXT("CoreUObject"))) {
				BasePackageName += TEXT("Engine.");
				break;
			}

			// replace FIN and Satisfactory name to make type names smaller
			if (NamePart.Equals(TEXT("FactoryGame"))
				|| NamePart.Equals(TEXT("Game"))) {
				BasePackageName += TEXT("Satis.");
				break;
			} else if (NamePart.Equals(TEXT("FicsItNetworks"))
				|| NamePart.Equals(TEXT("FicsItNetworksComputer"))
				|| NamePart.Equals(TEXT("FicsItNetworksLua"))
				|| NamePart.Equals(TEXT("FicsItReflection"))
				|| NamePart.Equals(TEXT("FicsItNetworksMisc"))) {
				BasePackageName += TEXT("FIN.");
				break;
			}

			BasePackageName += NamePart + TEXT(".");
			break;
		}

		return BasePackageName + Base->GetInternalName();
	}

	FString FINGenLuaSumnekoGetType(FFicsItReflectionModule &Ref, const UFIRProperty *Prop) {
		if (!Prop) {
			return "any";
		}

		switch (Prop->GetType()) {
			case FIR_NIL:
				return "nil";
			case FIR_BOOL:
				return "boolean";
			case FIR_INT:
			case FIR_FLOAT:
				return "number";
			case FIR_STR:
				return "string";
			case FIR_OBJ: {
				const UFIRObjectProperty *ObjProp = Cast<UFIRObjectProperty>(Prop);
				const UFIRClass *Class = Ref.FindClass(ObjProp->GetSubclass());
				if (!Class)
					return "Engine.Object";
				return FINGenLuaSumnekoGetTypeName(Class);
			}
			case FIR_TRACE: {
				const UFIRTraceProperty *TraceProp = Cast<UFIRTraceProperty>(Prop);
				const UFIRClass *Class = Ref.FindClass(TraceProp->GetSubclass());
				if (!Class)
					return "Engine.Object";
				return FINGenLuaSumnekoGetTypeName(Class);
			}
			case FIR_CLASS: {
				const UFIRClassProperty *ClassProp = Cast<UFIRClassProperty>(Prop);
				const UFIRClass *Class = Ref.FindClass(ClassProp->GetSubclass());
				if (!Class)
					return "Engine.Object";
				return FINGenLuaSumnekoGetTypeName(Class);
			}
			case FIR_STRUCT: {
				const UFIRStructProperty *StructProp = Cast<UFIRStructProperty>(Prop);
				const UFIRStruct *Struct = Ref.FindStruct(StructProp->GetInner());
				if (!Struct)
					return "any";
				return FINGenLuaSumnekoGetTypeName(Struct);
			}
			case FIR_ARRAY: {
				const UFIRArrayProperty *ArrayProp = Cast<UFIRArrayProperty>(Prop);
				return FINGenLuaSumnekoGetType(Ref, ArrayProp->GetInnerType()) + "[]";
			}
			default:
				return "any";
		}
	}

	FString FormatDescription(FString Description) {
		// assuming its only LF line endings
		Description.ReplaceCharInline('\n', ' ');
		return Description;
	}

	void FINGenLuaSumnekoDescription(FString &Str, FString Description) {
		Description.ReplaceInline(TEXT("\r\n"), TEXT("\n"));

		FString Line;
		while (Description.Split(TEXT("\n"), &Line, &Description)) {
			Str.Append(TEXT("--- ") + Line + TEXT("<br>\n"));
		}
		Str.Append(TEXT("--- ") + Description + TEXT("\n"));
	}

	FString FINGenLuaSumnekoProperty(FFicsItReflectionModule &Ref, const FString &Parent,
	                                 const UFIRProperty *Prop) {
		FString PropertyDocumentation = "\n";

		const EFIRPropertyFlags PropFlags = Prop->GetPropertyFlags();
		FINGenLuaSumnekoDescription(PropertyDocumentation, TEXT("### Flags:"));

		if (PropFlags & FIR_Prop_RT_Sync) {
			FINGenLuaSumnekoDescription(PropertyDocumentation,
			                            TEXT("* Runtime Synchronous - Can be called/changed in Game Tick."));
		}

		if (PropFlags & FIR_Prop_RT_Parallel) {
			FINGenLuaSumnekoDescription(PropertyDocumentation,
			                            TEXT(
				                            "* Runtime Parallel - Can be called/changed in Satisfactory Factory Tick."));
		}

		if (PropFlags & FIR_Prop_RT_Async) {
			FINGenLuaSumnekoDescription(PropertyDocumentation,
			                            TEXT("* Runtime Asynchronous - Can be changed anytime."));
		}

		if (PropFlags & FIR_Prop_ReadOnly) {
			FINGenLuaSumnekoDescription(PropertyDocumentation,
			                            TEXT("* Read Only - The value of this property can not be changed by code."));
		}

		PropertyDocumentation.Appendf(
			TEXT("---@type %s\n%s.%s = nil\n"), *FINGenLuaSumnekoGetType(Ref, Prop), *Parent, *Prop->GetInternalName());

		return PropertyDocumentation;
	}

	FString FINGenLuaSumnekoOperator(FFicsItReflectionModule &Ref, const UFIRFunction *Op) {
		FString OpName = Op->GetInternalName();
		FString OpTypeSumneko;

		if (OpName.Contains(TEXT("Add"))) {
			OpTypeSumneko = "add";
		} else if (OpName.Contains(TEXT("Sub"))) {
			OpTypeSumneko = "sub";
		} else if (OpName.Contains(TEXT("Mul"))) {
			OpTypeSumneko = "mul";
		} else if (OpName.Contains(TEXT("Div"))) {
			OpTypeSumneko = "div";
		} else if (OpName.Contains(TEXT("Mod"))) {
			OpTypeSumneko = "mod";
		} else if (OpName.Contains(TEXT("Pow"))) {
			OpTypeSumneko = "pow";
		} else if (OpName.Contains(TEXT("Neg"))) {
			OpTypeSumneko = "unm";
		} else if (OpName.Contains(TEXT("FDiv"))) {
			OpTypeSumneko = "idiv";
		} else if (OpName.Contains(TEXT("BitAND"))) {
			OpTypeSumneko = "band";
		} else if (OpName.Contains(TEXT("BitOR"))) {
			OpTypeSumneko = "bor";
		} else if (OpName.Contains(TEXT("BitXOR"))) {
			OpTypeSumneko = "bxor";
		} else if (OpName.Contains(TEXT("BitNot"))) {
			OpTypeSumneko = "bnot";
		} else if (OpName.Contains(TEXT("ShiftL"))) {
			OpTypeSumneko = "shl";
		} else if (OpName.Contains(TEXT("ShiftR"))) {
			OpTypeSumneko = "shr";
		} else if (OpName.Contains(TEXT("Concat"))) {
			OpTypeSumneko = "concat";
		} else if (OpName.Contains(TEXT("Len"))) {
			OpTypeSumneko = "len";
		} else if (OpName.Contains(TEXT("Call"))) {
			OpTypeSumneko = "call";
		}

		// are defined but not used and don't have a sunmeko equivalent
		// else if (OpName.Contains(TEXT("Equals"))) {
		// 	OpTypeSumneko = "";
		// } else if (OpName.Contains(TEXT("LessThan"))) {
		// 	OpTypeSumneko = "";
		// } else if (OpName.Contains(TEXT("LessOrEqualThen"))) {
		// 	OpTypeSumneko = "";
		// } else if (OpName.Contains(TEXT("Index"))) {
		// 	OpTypeSumneko = "";
		// } else if (OpName.Contains(TEXT("NewIndex"))) {
		// 	OpTypeSumneko = "";
		// }

		else {
			// return empty string for unsupported operator
			return TEXT("");
		}

		FString OpParameter;
		FString OpReturn;
		for (const UFIRProperty *Prop : Op->GetParameters()) {
			const EFIRPropertyFlags Flags = Prop->GetPropertyFlags();

			if (!(Flags & FIR_Prop_Param)) {
				continue; // skip prop
			}

			if (Flags & FIR_Prop_OutParam && OpReturn.Len() == 0) {
				OpReturn.Append(FINGenLuaSumnekoGetType(Ref, Prop));
				continue; // found return
			} else if (OpParameter.Len() == 0) {
				OpParameter.Append(FINGenLuaSumnekoGetType(Ref, Prop));
				continue; // found parameter
			}

			break; // already found parameter and return
		}

		return FString::Printf(TEXT("\n---@operator %s%s : %s"),
		                       *OpTypeSumneko,
		                       *(OpParameter.Len() > 0
			                         ? TEXT("(") + OpParameter + TEXT(")")
			                         : TEXT("")),
		                       *OpReturn);
	}

	FString FINGenLuaSumnekoFunction(FFicsItReflectionModule &Ref, const FString &Parent,
	                                 const UFIRFunction *Func) {
		FString FunctionDocumentation = "\n";

		FINGenLuaSumnekoDescription(FunctionDocumentation, Func->GetDescription().ToString());

		const EFIRFunctionFlags funcFlags = Func->GetFunctionFlags();
		FINGenLuaSumnekoDescription(FunctionDocumentation, TEXT("### Flags:"));

		if (funcFlags & FIR_Func_RT_Sync) {
			FINGenLuaSumnekoDescription(FunctionDocumentation,
			                            TEXT("* Runtime Synchronous - Can be called/changed in Game Tick."));
		}

		if (funcFlags & FIR_Func_RT_Parallel) {
			FINGenLuaSumnekoDescription(FunctionDocumentation,
			                            TEXT(
				                            "* Runtime Parallel - Can be called/changed in Satisfactory Factory Tick."));
		}

		if (funcFlags & FIR_Func_RT_Async) {
			FINGenLuaSumnekoDescription(FunctionDocumentation,
			                            TEXT("* Runtime Asynchronous - Can be changed anytime."));
		}

		FString ParamDocumentation;
		FString ReturnDocumentation;
		FString ParamList;
		for (const UFIRProperty *Prop : Func->GetParameters()) {
			const EFIRPropertyFlags Flags = Prop->GetPropertyFlags();

			if (!(Flags & FIR_Prop_Param)) {
				continue; // skip prop
			}

			if (Flags & FIR_Prop_OutParam) {
				ReturnDocumentation.Appendf(TEXT("---@return %s %s %s\n"),
				                            *FINGenLuaSumnekoGetType(Ref, Prop),
				                            *Prop->GetInternalName(),
				                            *FormatDescription(Prop->GetDescription().ToString()));
			} else {
				ParamDocumentation.Appendf(
					TEXT("---@param %s %s %s\n"),
					*Prop->GetInternalName(),
					*FINGenLuaSumnekoGetType(Ref, Prop),
					*FormatDescription(Prop->GetDescription().ToString())
				);

				if (ParamList.Len() > 0) {
					ParamList.Append(", ");
				}

				ParamList.Append(Prop->GetInternalName());
			}
		}

		if (funcFlags & FIR_Func_VarArgs) {
			ParamDocumentation.Append(TEXT("---@param ... any @additional arguments as described\n"));
			
			if (ParamList.Len() > 0) {
				ParamList.Append(", ");
			}

			ParamList.Append("...");
		}
		if (funcFlags & FIR_Func_RT_Async) {
			ReturnDocumentation.Append(TEXT("---@return any ... @additional return values as described\n"));
		}

		FunctionDocumentation.Append(
			FString::Printf(TEXT("%s%sfunction %s:%s(%s) end\n"),
			                *ParamDocumentation,
			                *ReturnDocumentation,
			                *Parent,
			                *Func->GetInternalName(),
			                *ParamList));

		return FunctionDocumentation;
	}

	FString FINGenLuaSumnekoSignal(FFicsItReflectionModule &Ref, const FString &Parent,
	                               const UFIRSignal *Signal) {
		FString SignalDocumentation = "\n";

		FINGenLuaSumnekoDescription(SignalDocumentation, Signal->GetDescription().ToString() + TEXT("\n"));

		FINGenLuaSumnekoDescription(SignalDocumentation, TEXT("### returns from event.pull:\n```"));

		SignalDocumentation.Append(TEXT("--- local signalName, component"));
		for (const UFIRProperty *Prop : Signal->GetParameters()) {
			SignalDocumentation.Append(TEXT(", ") + Prop->GetDisplayName().ToString().Replace(TEXT(" "), TEXT("")));
		}
		if (Signal->IsVarArgs()) {
			SignalDocumentation.Append(TEXT(", ..."));
		}
		SignalDocumentation.Append(TEXT(" = event.pull()\n--- ```\n"));

		FINGenLuaSumnekoDescription(SignalDocumentation,
		                            FString::Printf(TEXT("- `signalName: \"%s\"`"), *Signal->GetInternalName()));
		FINGenLuaSumnekoDescription(SignalDocumentation,
		                            FString::Printf(TEXT("- `component: %s`"), *Parent));

		for (const UFIRProperty *Prop : Signal->GetParameters()) {
			FINGenLuaSumnekoDescription(SignalDocumentation,
			                            FString::Printf(
				                            TEXT("- `%s: %s` \n%s"),
				                            *Prop->GetDisplayName().ToString().Replace(TEXT(" "), TEXT("")),
				                            *FINGenLuaSumnekoGetType(Ref, Prop),
				                            *Prop->GetDescription().ToString()
			                            ));
		}
		if (Signal->IsVarArgs()) {
			FINGenLuaSumnekoDescription(SignalDocumentation, TEXT("- `...: any`"));
		}

		// hard coding the type is maybe not the best choice
		SignalDocumentation.Append(TEXT("---@deprecated\n---@type FIN.Signal\n"));
		SignalDocumentation.Appendf(
			TEXT("%s.SIGNAL_%s = nil\n"),
			*Parent,
			*Signal->GetInternalName()
		);

		return SignalDocumentation;
	}

	FString FINGenLuaSumnekoClass(FFicsItReflectionModule &Ref, const UFIRClass *Class) {
		FString ClassTypeName = FINGenLuaSumnekoGetTypeName(Class);

		FString OperatorDocumentation;
		FString MembersDocumentation;

		for (const UFIRProperty *Prop : Class->GetProperties(false)) {
			if (Prop->GetPropertyFlags() & FIR_Prop_Attrib) {
				MembersDocumentation.Append(FINGenLuaSumnekoProperty(Ref, Class->GetInternalName(), Prop));
			}
		}

		for (const UFIRFunction *Func : Class->GetFunctions(false)) {
			if (Func->GetFunctionFlags() & FIR_Func_MemberFunc) {
				if (Func->GetInternalName().Contains("FIR_Operator")) {
					OperatorDocumentation.Append(FINGenLuaSumnekoOperator(Ref, Func));
					continue; // to next function
				}

				MembersDocumentation.Append(FINGenLuaSumnekoFunction(Ref, Class->GetInternalName(), Func));
			}
		}

		for (const UFIRSignal *Signal : Class->GetSignals(false)) {
			MembersDocumentation.Append(FINGenLuaSumnekoSignal(Ref, Class->GetInternalName(), Signal));
		}

		FString ClassDocumentation = "\n";

		FINGenLuaSumnekoDescription(ClassDocumentation, Class->GetDescription().ToString());
		ClassDocumentation.Appendf(
			TEXT("---@class %s%s%s\n"),
			*ClassTypeName,
			*(Class->GetParent()
				  ? TEXT(" : ") + FINGenLuaSumnekoGetTypeName(Class->GetParent())
				  : TEXT("")),
			*OperatorDocumentation
		);

		FString ClassGlobalClassesDocumentation = FString::Printf(
			TEXT("---@class FIN.classes.%s : %s\nclasses.%s = nil\n"),
			*ClassTypeName,
			*ClassTypeName,
			*Class->GetInternalName()
		);

		return FString::Printf(TEXT("%slocal %s\n\n%s%s"),
		                       *ClassDocumentation, *Class->GetInternalName(), *ClassGlobalClassesDocumentation,
		                       *MembersDocumentation);
	}

	FString FINGenLuaSumnekoStruct(FFicsItReflectionModule &Ref, const UFIRStruct *Struct) {
		FString StructTypeName = FINGenLuaSumnekoGetTypeName(Struct);

		FString OperatorDocumentation;
		FString MembersDocumentation;
		TArray<FString> PropertyTypes;

		for (const UFIRProperty *Prop : Struct->GetProperties(false)) {
			if (Prop->GetPropertyFlags() & FIR_Prop_Attrib) {
				MembersDocumentation.Append(FINGenLuaSumnekoProperty(Ref, Struct->GetInternalName(), Prop));
				PropertyTypes.Add(FINGenLuaSumnekoGetType(Ref, Prop));
			}
		}

		for (const UFIRFunction *Func : Struct->GetFunctions(false)) {
			if (Func->GetFunctionFlags() & FIR_Func_MemberFunc) {
				if (Func->GetInternalName().Contains("FIR_Operator")) {
					OperatorDocumentation.Append(FINGenLuaSumnekoOperator(Ref, Func));
					continue; // to next function
				}

				MembersDocumentation.Append(FINGenLuaSumnekoFunction(Ref, Struct->GetInternalName(), Func));
			}
		}

		FString StructDocumentation = "\n";
		FINGenLuaSumnekoDescription(StructDocumentation, Struct->GetDescription().ToString());
		StructDocumentation.Appendf(
			TEXT("---@class %s%s\n"),
			*StructTypeName,
			*OperatorDocumentation
		);

		FString StructGlobalStructsDocumentation = FString::Printf(
			TEXT("---@class FIN.structs.%s : %s\n"), *StructTypeName, *StructTypeName);
		if (Struct->GetStructFlags() & FIR_Struct_Constructable) {
			FString ConstructorCallSignature = TEXT("{");
			for (int i = 0; i < PropertyTypes.Num(); ++i) {
				ConstructorCallSignature.Appendf(TEXT("%s [%d]: %s"),
				                                 i > 0
					                                 ? TEXT(",")
					                                 : TEXT(""),
				                                 i + 1, // offset because lua starts arrays at 1
				                                 *PropertyTypes[i]
				);
			}
			ConstructorCallSignature.Append(TEXT(" }"));

			StructGlobalStructsDocumentation.Appendf(
				TEXT("---@overload fun(data: %s ) : %s\n"),
				*ConstructorCallSignature,
				*StructTypeName
			);
		}
		StructGlobalStructsDocumentation.Appendf(TEXT("structs.%s = nil\n"), *Struct->GetInternalName());

		return FString::Printf(TEXT("%slocal %s\n\n%s%s"),
		                       *StructDocumentation, *Struct->GetInternalName(), *StructGlobalStructsDocumentation,
		                       *MembersDocumentation);
	}

	bool FINGenLuaDocSumneko(UWorld *World, const TCHAR *Command, FOutputDevice &Ar) {
		if (FParse::Command(&Command, TEXT("FINGenLuaDocSumneko"))) {
			FFicsItReflectionModule &Ref = FFicsItReflectionModule::Get();
			FString Documentation = Manual_FINGenLuaSumnekoDocumentation;

			{
				// adding "do" and "end" to get rid of local maximum variables reached
				int32_t count = 0;
				for (TPair<UClass*, UFIRClass*> const Class : Ref.GetClasses()) {
					if (count == 0) {
						Documentation.Append("do\n");
					}

					Documentation.Append(FINGenLuaSumnekoClass(Ref, Class.Value));
					count++;

					if (count == 180) {
						Documentation.Append("\nend\n");
						count = 0;
					}
				}
				for (TPair<UScriptStruct*, UFIRStruct*> const Struct : Ref.GetStructs()) {
					if (count == 0) {
						Documentation.Append("do\n");
					}

					Documentation.Append(FINGenLuaSumnekoStruct(Ref, Struct.Value));
					count++;

					if (count == 180) {
						Documentation.Append("\nend\n");
						count = 0;
					}
				}

				if (count != 0) {
					Documentation.Append("\nend\n");
				}
			}

			FString Path = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/"));
			Path = FPaths::Combine(Path, TEXT("FINLuaDocumentationSumneko.lua"));
			FFileHelper::SaveStringToFile(Documentation, *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

			return true;
		}
		return false;
	}

	[[maybe_unused]] static FStaticSelfRegisteringExec FINGenLuaDocSumnekoStaticExec(&FINGenLuaDocSumneko);
}

UE_ENABLE_OPTIMIZATION_SHIP
