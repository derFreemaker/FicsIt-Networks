#include "FINLua/Reflection/LuaObject.h"
#include "FINLuaProcessor.h"
#include "FicsItKernel/Network/NetworkController.h"
#include "FINLua/FINLuaModule.h"
#include "FINLua/LuaPersistence.h"
#include "Network/FINNetworkUtils.h"
#include "Network/Signals/FINSignalSubsystem.h"

namespace FINLua {
	LuaModule(R"(/**
	 * @LuaModule		Event
	 * @DisplayName		Event Module
	 */)", Event) {
		LuaModuleLibrary(R"(/**
		 * @LuaLibrary		event
		 * @DisplayName		Event Library
		 *
		 * The Event API provides classes, functions and variables for interacting with the component network.
		 */)", event) {
			void luaListen(lua_State* L, FFINNetworkTrace o) {
				const UFINKernelNetworkController* net = UFINLuaProcessor::luaGetProcessor(L)->GetKernel()->GetNetwork();
				UObject* obj = *o;
				if (!IsValid(obj)) luaL_error(L, "object is not valid");
				AFINSignalSubsystem* SigSubSys = AFINSignalSubsystem::GetSignalSubsystem(obj);
				SigSubSys->Listen(obj, o.Reverse() / net->GetComponent().GetObject());
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		listen(Object<Object>...)
			 * @DisplayName		Listen
			 *
			 * Adds the running lua context to the listen queue of the given components.
			 */)", listen) {
				// ReSharper disable once CppDeclaratorNeverUsed
				FLuaSyncCall SyncCall(L);
				const int args = lua_gettop(L);

				for (int i = 1; i <= args; ++i) {
					FFINNetworkTrace trace = luaFIN_checkObject(L, i, nullptr);
					luaListen(L, trace);
				}
				return UFINLuaProcessor::luaAPIReturn(L, 0);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		Object<Object>	listening()
			 * @DisplayName		Listening
			 *
			 * Returns all signal senders this computer is listening to.
			 */)", listening) {
				// ReSharper disable once CppDeclaratorNeverUsed
				FLuaSyncCall SyncCall(L);

				UObject* netComp = UFINLuaProcessor::luaGetProcessor(L)->GetKernel()->GetNetwork()->GetComponent().GetObject();

				TArray<UObject*> Listening = AFINSignalSubsystem::GetSignalSubsystem(netComp)->GetListening(netComp);
				int i = 0;
				lua_newtable(L);
				for (UObject* Obj : Listening) {
					luaFIN_pushObject(L, FFINNetworkTrace(netComp) / Obj);
					lua_seti(L, -2, ++i);
				}
				return 1;
			}

			// ReSharper disable once CppParameterNeverUsed
			int luaPullContinue(lua_State* L, int status, lua_KContext ctx) {
				const int args = lua_gettop(L);

				return args - ctx;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		string|nil, Object<Object>, ...		pull([timeout: number])
			 * @DisplayName		Pull
			 *
			 * Waits for a signal in the queue. Blocks the execution until a signal got pushed to the signal queue, or the timeout is reached. +
			 * Returns directly if there is already a signal in the queue (the tick doesn’t get yielded).
			 *
			 * @parameter	timeout		number			Timeout		The amount of time needs to pass until pull unblocks when no signal got pushed. If not set, the function will block indefinitely until a signal gets pushed. If set to `0` (int), will not yield the tick and directly return with the signal data or nil if no signal was in the queue.
			 * @return		event		string|nil		Event		The name of the returned signal. Nil when timeout got reached.
			 * @return		sender		Object<Object>	Sender		The component representation of the signal sender. Not set when timeout got reached.
			 * @return		parameters	any...				Parameters	The parameters passed to the signal. Not set when timeout got reached.
			 */)", pull) {
				const int args = lua_gettop(L);
				double t = 0.0;
				if (args > 0) t = lua_tonumber(L, 1);

				UFINLuaProcessor* luaProc = UFINLuaProcessor::luaGetProcessor(L);
				check(luaProc);
				const int a = luaProc->DoSignal(L);
				if (!a && !(args > 0 && lua_isinteger(L, 1) && lua_tointeger(L, 1) == 0)) {
					luaProc->Timeout = t;
					luaProc->PullStart =  (FDateTime::Now() - FFicsItNetworksModule::GameStart).GetTotalMilliseconds();
					luaProc->PullState = (args > 0) ? 1 : 2;

					luaProc->GetTickHelper().shouldWaitForSignal();

					return lua_yieldk(L, 0, args, luaPullContinue);
				}
				luaProc->PullState = 0;
				return UFINLuaProcessor::luaAPIReturn(L, a);
			}

			void luaIgnore(lua_State* L, FFINNetworkTrace o) {
				UObject* obj = *o;
				if (!IsValid(obj)) luaL_error(L, "object is not valid");
				AFINSignalSubsystem* SigSubSys = AFINSignalSubsystem::GetSignalSubsystem(obj);
				SigSubSys->Ignore(obj, *o.Reverse());
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		ignore(Object<Object>...)
			 * @DisplayName		Ignore
			 *
			 * Removes the running lua context from the listen queue of the given components. Basically the opposite of listen.
			 */)", ignore) {
				// ReSharper disable once CppDeclaratorNeverUsed
				FLuaSyncCall SyncCall(L);
				const int args = lua_gettop(L);

				for (int i = 1; i <= args; ++i) {
					FFINNetworkTrace Trace = luaFIN_checkObject(L, i, nullptr);
					luaIgnore(L, Trace);
				}
				return UFINLuaProcessor::luaAPIReturn(L, 0);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		ignoreAll()
			 * @DisplayName		Ignore All
			 *
			 * Stops listening to any signal sender. If afterwards there are still coming signals in, it might be the system itself or caching bug.
			 */)", ignoreAll) {
				// ReSharper disable once CppDeclaratorNeverUsed
				FLuaSyncCall SyncCall(L);
				UFINKernelNetworkController* net = UFINLuaProcessor::luaGetProcessor(L)->GetKernel()->GetNetwork();
				AFINSignalSubsystem* SigSubSys = AFINSignalSubsystem::GetSignalSubsystem(net->GetComponent().GetObject());
				SigSubSys->IgnoreAll(net->GetComponent().GetObject());
				return UFINLuaProcessor::luaAPIReturn(L, 1);
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		clear()
			 * @DisplayName		Clear
			 *
			 * Clears every signal from the signal queue.
			 */)", clear) {
				UFINLuaProcessor::luaGetProcessor(L)->GetKernel()->GetNetwork()->ClearSignals();
				return 0;
			}
		}

		LuaModulePostSetup() {
			lua_pushcfunction(L, (int(*)(lua_State*))event::luaPullContinue);
			luaFIN_persistValue(L, -1, "Event-luaPullContinue");
			lua_pop(L, 1);
		}
	}
}
