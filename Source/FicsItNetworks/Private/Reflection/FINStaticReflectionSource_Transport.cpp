#include "Reflection/FINStaticReflectionSourceHooks.h"
#include "Reflection/FINStaticReflectionSourceMacros.h"

#include "FGFluidIntegrantInterface.h"
#include "FGPipeConnectionComponent.h"
#include "FGPipeSubsystem.h"
#include "FGRailroadTimeTable.h"
#include "FGTrainStationIdentifier.h"
#include "Buildables/FGBuildableDockingStation.h"
#include "Buildables/FGBuildablePipeHyper.h"
#include "Buildables/FGBuildablePipelinePump.h"
#include "Buildables/FGBuildablePipeReservoir.h"
#include "Buildables/FGBuildableRailroadSwitchControl.h"
#include "Buildables/FGBuildableTrainPlatformCargo.h"
#include "Buildables/FGPipeHyperStart.h"
#include "Computer/FINComputerSubsystem.h"
#include "Reflection/ReflectionHelper.h"
#include "Utils/FINRailroadSignalBlock.h"
#include "Utils/FINTargetPoint.h"
#include "Utils/FINTimeTableStop.h"
#include "Utils/FINTrackGraph.h"
#include "WheeledVehicles/FGTargetPointLinkedList.h"
#include "WheeledVehicles/FGWheeledVehicle.h"

BeginClass(AFGPipeHyperStart, "PipeHyperStart", "Pipe Hyper Start", "A actor that is a hypertube entrance buildable")
	Hook(UFINPipeHyperStartHook)
BeginSignal(PlayerEntered, "Player Entered", "Triggers when a players enters into this hypertube entrance.")
	SignalParam(RBool, success, "Sucess", "True if the transfer was sucessfull")
EndSignal()
BeginSignal(PlayerExited, "Player Exited", "Triggers when a players leaves through this hypertube entrance.")
EndSignal()
EndClass()

BeginClass(AFGBuildablePipeHyper, "BuildablePipeHyper", "Buildable Pipe Hyper", "A hypertube pipe")
EndClass()

BeginClass(UFGPipeConnectionComponentBase, "PipeConnectionBase", "Pipe Connection Base", "A actor component base that is a connection point to which a pipe for fluid or hyper can get attached to.")
BeginProp(RBool, isConnected, "Is Connected", "True if something is connected to this connection.") {
	Return self->IsConnected();
} EndProp()
BeginFunc(getConnection, "Get Connection", "Returns the connected pipe connection component.") {
	OutVal(RTrace<UFGPipeConnectionComponentBase>, connected, "Connected", "The connected pipe connection component.")
	FuncBody()
	connected = Ctx.GetTrace() / self->GetConnection();
} EndFunc()
EndClass()

BeginClass(UFGPipeConnectionComponent, "PipeConnection", "Pipe Connection", "A actor component that is a connection point to which a fluid pipe can get attached to.")
//Hook(UFINFactoryConnectorHook)
BeginProp(RFloat, fluidBoxContent, "Fluid Box Content", "Returns the amount of fluid this fluid container contains") {
	Return self->GetFluidIntegrant()->GetFluidBox()->Content;
} EndProp()
BeginProp(RFloat, fluidBoxHeight, "Fluid Box Height", "Returns the height of this fluid container") {
	Return self->GetFluidIntegrant()->GetFluidBox()->Height;
} EndProp()
BeginProp(RFloat, fluidBoxLaminarHeight, "Fluid Box Laminar Height", "Returns the laminar height of this fluid container") {
	Return self->GetFluidIntegrant()->GetFluidBox()->LaminarHeight;
} EndProp()
BeginProp(RFloat, fluidBoxFlowThrough, "Fluid Box Flow Through", "Returns the amount of fluid flowing through this fluid container") {
	Return self->GetFluidIntegrant()->GetFluidBox()->FlowThrough;
} EndProp()
BeginProp(RFloat, fluidBoxFlowFill, "Fluid Box Flow Fill", "Returns the fill rate of this fluid container") {
	Return self->GetFluidIntegrant()->GetFluidBox()->FlowFill;
} EndProp()
BeginProp(RFloat, fluidBoxFlowDrain, "Fluid Box Flow Drain", "Returns the drain rate of this fluid container") {
	Return self->GetFluidIntegrant()->GetFluidBox()->FlowDrain;
} EndProp()
BeginProp(RFloat, fluidBoxFlowLimit, "Fluid Box Flow Limit", "Returns the the maximum flow limit of this fluid container") {
	Return self->GetFluidIntegrant()->GetFluidBox()->FlowLimit;
} EndProp()
BeginProp(RInt, networkID, "Get Network ID", "Returns the network ID of the pipe network this connection is associated with") {
	Return (int64)self->GetPipeNetworkID();
} EndProp();
BeginFunc(getFluidDescriptor, "Get Fluid Descriptor", "?") {  /* TODO: Write DOC when figured out exactly what it does */
	OutVal(RTrace<UFGItemDescriptor>, fluidDescriptor, "Fluid Descriptor", "?")   /* TODO: Write DOC */
	FuncBody()
	fluidDescriptor = Ctx.GetTrace() / self->GetFluidDescriptor();
} EndFunc()
/*BeginFunc(getFluidIntegrant, "Get Fluid Integrant", "?") {  
	OutVal(RObject<IFGFluidIntegrantInterface>, fluidIntegrant, "Fluid Descriptor", "?")
    FuncBody()
    fluidIntegrant = Ctx.GetTrace() / self->GetFluidIntegrant();
} EndFunc()*/
BeginFunc(flushPipeNetwork, "Flush Pipe Network", "Flush the associated pipe network") {  
    FuncBody()
	auto networkID = self->GetPipeNetworkID();
    auto subsystem = AFGPipeSubsystem::GetPipeSubsystem(self->GetWorld());
	subsystem->FlushPipeNetwork(networkID);
} EndFunc()
EndClass()

BeginClass(AFGVehicle, "Vehicle", "Vehicle", "A base class for all vehciles.")
BeginProp(RFloat, health, "Health", "The health of the vehicle.") {
	Return self->GetHealthComponent()->GetCurrentHealth();
} EndProp()
BeginProp(RFloat, maxHealth, "Max Health", "The maximum amount of health this vehicle can have.") {
	Return self->GetHealthComponent()->GetMaxHealth();
} EndProp()
BeginProp(RBool, isSelfDriving, "Is Self Driving", "True if the vehicle is currently self driving.") {
	Return self->IsSelfDriving();
} PropSet() {
	FReflectionHelper::SetPropertyValue<FBoolProperty>(self, TEXT("mIsSelfDriving"), Val);
} EndProp() 
EndClass()

BeginClass(AFGWheeledVehicle, "WheeledVehicle", "Wheeled Vehicle", "The base class for all vehicles that used wheels for movement.")
BeginFunc(getFuelInv, "Get Fuel Inventory", "Returns the inventory that contains the fuel of the vehicle.") {
	OutVal(RTrace<UFGInventoryComponent>, inventory, "Inventory", "The fuel inventory of the vehicle.")
	FuncBody()
	inventory = Ctx.GetTrace() / self->GetFuelInventory();
} EndFunc()
BeginFunc(getStorageInv, "Get Storage Inventory", "Returns the inventory that contains the storage of the vehicle.") {
	OutVal(RTrace<UFGInventoryComponent>, inventory, "Inventory", "The storage inventory of the vehicle.")
	FuncBody()
	inventory = Ctx.GetTrace() / self->GetStorageInventory();
} EndFunc()
BeginFunc(isValidFuel, "Is Valid Fuel", "Allows to check if the given item type is a valid fuel for this vehicle.") {
	InVal(RClass<UFGItemDescriptor>, item, "Item", "The item type you want to check.")
	OutVal(RBool, isValid, "Is Valid", "True if the given item type is a valid fuel for this vehicle.")
	FuncBody()
	isValid = self->IsValidFuel(item);
} EndFunc()
BeginFunc(getCurrentTarget, "Get Current Target", "Returns the index of the target that the vehicle tries to move to right now.") {
	OutVal(RInt, index, "Index", "The index of the current target.")
	FuncBody()
	AFGDrivingTargetList* List = self->GetTargetList();
	index = (int64)List->FindTargetIndex(List->mCurrentTarget);
} EndFunc()
BeginFunc(nextTarget, "Next Target", "Sets the current target to the next target in the list.") {
	FuncBody()
	self->PickNextTarget();
} EndFunc()
BeginFunc(setCurrentTarget, "Set Current Target", "Sets the target with the given index as the target this vehicle tries to move to right now.") {
	InVal(RInt, index, "Index", "The index of the target this vehicle should move to now.")
	FuncBody()
	AFGDrivingTargetList* List = self->GetTargetList();
	AFGTargetPoint* Target = List->FindTargetByIndex(index);
	if (!Target) throw FFINException("index out of range");
	List->mCurrentTarget = Target;
} EndFunc()
BeginFunc(getTargetList, "Get Target List", "Returns the list of targets/path waypoints.") {
	OutVal(RTrace<AFGDrivingTargetList>, targetList, "Target List", "The list of targets/path-waypoints.")
	FuncBody()
	targetList = Ctx.GetTrace() / self->GetTargetList();
} EndFunc()
BeginProp(RFloat, speed, "Speed", "The current forward speed of this vehicle.") {
	Return self->GetForwardSpeed();
} EndProp()
BeginProp(RFloat, burnRatio, "Burn Ratio", "The amount of fuel this vehicle burns.") {
	Return self->GetFuelBurnRatio();
} EndProp()
BeginProp(RBool, hasFuel, "Has Fuel", "True if the vehicle has currently fuel to drive.") {
	Return self->HasFuel();
} EndProp()
EndClass()

BeginClass(AFGDrivingTargetList, "TargetList", "Target List", "The list of targets/path-waypoints a autonomous vehicle can drive")
BeginFunc(getTarget, "Get Target", "Returns the target struct at with the given index in the target list.") {
	InVal(RInt, index, "Index", "The index of the target you want to get the struct from.")
	OutVal(RStruct<FFINTargetPoint>, target, "Target", "The TargetPoint-Struct with the given index in the target list.")
	FuncBody()
	AFGTargetPoint* Target = self->FindTargetByIndex(index);
	if (!Target) throw FFINException("index out of range");
	target = (FINAny)FFINTargetPoint(Target);
} EndFunc()
BeginFunc(removeTarget, "Remove Target", "Removes the target with the given index from the target list.") {
	InVal(RInt, index, "Index", "The index of the target point you want to remove from the target list.")
	FuncBody()
	AFGTargetPoint* Target = self->FindTargetByIndex(index);
	if (!Target) throw FFINException( "index out of range");
	self->RemoveItem(Target);
	Target->Destroy();
} EndFunc()
BeginFunc(addTarget, "Add Target", "Adds the given target point struct at the end of the target list.") {
	InVal(RStruct<FFINTargetPoint>, target, "Target", "The target point you want to add.")
	FuncBody()
	AFGTargetPoint* Target = target.ToWheeledTargetPoint(self);
	if (!Target) throw FFINException("failed to create target");
	self->InsertItem(Target, self->mLast);
} EndFunc()
BeginFunc(setTarget, "Set Target", "Allows to set the target at the given index to the given target point struct.") {
	InVal(RInt, index, "Index", "The index of the target point you want to update with the given target point struct.")
	InVal(RStruct<FFINTargetPoint>, target, "Target", "The new target point struct for the given index.")
	FuncBody()
	AFGTargetPoint* Target = self->FindTargetByIndex(index);
	if (!Target) throw FFINException("index out of range");
	Target->SetActorLocation(target.Pos);
	Target->SetActorRotation(target.Rot);
	Target->SetTargetSpeed(target.Speed);
	Target->SetWaitTime(target.Wait);
} EndFunc()
BeginFunc(getTargets, "Get Targets", "Returns a list of target point structs of all the targets in the target point list.") {
	OutVal(RArray<RStruct<FFINTargetPoint>>, targets, "Targets", "A list of target point structs containing all the targets of the target point list.")
	FuncBody()
	TArray<FINAny> Targets;
	AFGTargetPoint* CurrentTarget = nullptr;
	int i = 0;
	do {
		if (i++) CurrentTarget = CurrentTarget->GetNext();
		else CurrentTarget = self->GetFirstTarget();
		Targets.Add((FINAny)FFINTargetPoint(CurrentTarget));
	} while (CurrentTarget && CurrentTarget != self->GetLastTarget());
	targets = Targets;
} EndFunc()
BeginFunc(setTargets, "Set Targets", "Removes all targets from the target point list and adds the given array of target point structs to the empty target point list.", 0) {
	InVal(RArray<RStruct<FFINTargetPoint>>, targets, "Targets", "A list of target point structs you want to place into the empty target point list.")
	FuncBody()
	int Count = self->GetTargetCount();
	for (const FINAny& Target : targets) {
		self->InsertItem(Target.GetStruct().Get<FFINTargetPoint>().ToWheeledTargetPoint(self), self->mLast);
	}
	for (int i = 0; i < Count; ++i) {
		self->RemoveItem(self->mFirst);
	}
} EndFunc()
EndClass()

BeginClass(AFGBuildableTrainPlatform, "TrainPlatform", "Train Platform", "The base class for all train station parts.")
BeginFunc(getTrackGraph, "Get Track Graph", "Returns the track graph of which this platform is part of.") {
	OutVal(RStruct<FFINTrackGraph>, graph, "Graph", "The track graph of which this platform is part of.")
	FuncBody()
	graph = (FINAny)FFINTrackGraph{Ctx.GetTrace(), self->GetTrackGraphID()};
} EndFunc()
BeginFunc(getTrackPos, "Get Track Pos", "Returns the track pos at which this train platform is placed.") {
	OutVal(RTrace<AFGBuildableRailroadTrack>, track, "Track", "The track the track pos points to.")
	OutVal(RFloat, offset, "Offset", "The offset of the track pos.")
	OutVal(RFloat, forward, "Forward", "The forward direction of the track pos. 1 = with the track direction, -1 = against the track direction")
	FuncBody()
	FRailroadTrackPosition pos = self->GetTrackPosition();
	if (!pos.IsValid()) throw FFINException("Railroad track position of self is invalid");
	track = Ctx.GetTrace()(pos.Track.Get());
	offset = pos.Offset;
	forward = pos.Forward;
} EndFunc()
BeginFunc(getConnectedPlatform, "Get Connected Platform", "Returns the connected platform in the given direction.") {
	InVal(RInt, direction, "Direction", "The direction in which you want to get the connected platform.")
	OutVal(RTrace<AFGBuildableTrainPlatform>, platform, "Platform", "The platform connected to this platform in the given direction.")
	FuncBody()
	platform = Ctx.GetTrace() / self->GetConnectedPlatformInDirectionOf(direction);
} EndFunc()
BeginFunc(getDockedVehicle, "Get Docked Vehicle", "Returns the currently docked vehicle.") {
	OutVal(RTrace<AFGVehicle>, vehicle, "Vehicle", "The currently docked vehicle")
	FuncBody()
	vehicle = Ctx.GetTrace() / FReflectionHelper::GetPropertyValue<FObjectProperty>(self, TEXT("mDockedRailroadVehicle"));
} EndFunc()
BeginFunc(getMaster, "Get Master", "Returns the master platform of this train station.") {
	OutVal(RTrace<AFGRailroadVehicle>, master, "Master", "The master platform of this train station.")
	FuncBody()
	master = Ctx.GetTrace() / FReflectionHelper::GetPropertyValue<FObjectProperty>(self, TEXT("mStationDockingMaster"));
} EndFunc()
BeginFunc(getDockedLocomotive, "Get Docked Locomotive", "Returns the currently docked locomotive at the train station.") {
	OutVal(RTrace<AFGLocomotive>, locomotive, "Locomotive", "The currently docked locomotive at the train station.")
	FuncBody()
	locomotive = Ctx.GetTrace() / FReflectionHelper::GetPropertyValue<FObjectProperty>(self, TEXT("mDockingLocomotive"));
} EndFunc()
BeginProp(RInt, status, "Status", "The current docking status of the platform.") {
	Return (int64)self->GetDockingStatus();
} EndProp()
BeginProp(RBool, isReversed, "Is Reversed", "True if the orientation of the platform is reversed relative to the track/station.") {
	Return self->IsOrientationReversed();
} EndProp()
EndClass()

BeginClass(AFGBuildableRailroadStation, "RailroadStation", "Railroad Station", "The train station master platform. This platform holds the name and manages docking of trains.")
Hook(UFINRailroadStationHook)
BeginSignal(StartDocking, "Start Docking", "Triggers when a train tries to dock onto the station.")
	SignalParam(RBool, successful, "Successful", "True if the train successfully docked.")
	SignalParam(RTrace<AFGLocomotive>, locomotive, "Locomotive", "The locomotive that tries to dock onto the station.")
	SignalParam(RFloat, offset, "Offset", "The offset at witch the train tried to dock.")
EndSignal()
BeginSignal(FinishDocking, "Finish Docking", "Triggers when a train finished the docking procedure and is ready to depart the station.")
EndSignal()
BeginSignal(CancelDocking, "Cancel Docking", "Triggers when a train cancels the docking procedure.")
EndSignal()
BeginProp(RString, name, "Name", "The name of the railroad station.") {
	Return self->GetStationIdentifier()->GetStationName().ToString();
} PropSet() {
	self->GetStationIdentifier()->SetStationName(FText::FromString(Val));
} EndProp()
BeginProp(RInt, dockedOffset, "Docked Offset", "The Offset to the beginning of the station at which trains dock.") {
	Return self->GetDockedVehicleOffset();
} EndProp()
EndClass()

BeginClass(AFGBuildableTrainPlatformCargo, "TrainPlatformCargo", "Train Platform Cargo", "A train platform that allows for loading and unloading cargo cars.")
BeginProp(RBool, isLoading, "Is Loading", "True if the cargo platform is currently loading the docked cargo vehicle.") {
	Return self->GetIsInLoadMode();
} PropSet() {
	self->SetIsInLoadMode(Val);
} EndProp()
BeginProp(RBool, isUnloading, "Is Unloading", "True if the cargo platform is currently loading or unloading the docked cargo vehicle.") {
	Return self->IsLoadUnloading();
} EndProp()
BeginProp(RFloat, dockedOffset, "Docked Offset", "The offset to the track start of the platform at were the vehicle docked.") {
	Return self->GetDockedVehicleOffset();
} EndProp()
BeginProp(RFloat, outputFlow, "Output Flow", "The current output flow rate.") {
	Return self->GetOutflowRate();
} EndProp()
BeginProp(RFloat, inputFlow, "Input Flow", "The current input flow rate.") {
	Return self->GetInflowRate();
} EndProp()
BeginProp(RBool, fullLoad, "Full Load", "True if the docked cargo vehicle is fully loaded.") {
	Return (bool)self->IsFullLoad();
} EndProp()
BeginProp(RBool, fullUnload, "Full Unload", "Ture if the docked cargo vehicle is fully unloaded.") {
	Return (bool)self->IsFullUnload();
} EndProp()
EndClass()

BeginClass(AFGRailroadVehicle, "RailroadVehicle", "Railroad Vehicle", "The base class for any vehicle that drives on train tracks.")
BeginFunc(getTrain, "Get Train", "Returns the train of which this vehicle is part of.") {
	OutVal(RTrace<AFGTrain>, train, "Train", "The train of which this vehicle is part of")
	FuncBody()
	train = Ctx.GetTrace() / Cast<UObject>(self->GetTrain());
} EndFunc()
BeginFunc(isCoupled, "Is Coupled", "Allows to check if the given coupler is coupled to another car.") {
	InVal(RInt, coupler, "Coupler", "The Coupler you want to check. 0 = Front, 1 = Back")
	OutVal(RBool, coupled, "Coupled", "True of the give coupler is coupled to another car.")
	FuncBody()
	coupled = self->IsCoupledAt(static_cast<ERailroadVehicleCoupler>(coupler));
} EndFunc()
BeginFunc(getCoupled, "Get Coupled", "Allows to get the coupled vehicle at the given coupler.") {
	InVal(RInt, coupler, "Coupler", "The Coupler you want to get the car from. 0 = Front, 1 = Back")
	OutVal(RTrace<AFGRailroadVehicle>, coupled, "Coupled", "The coupled car of the given coupler is coupled to another car.")
	FuncBody()
	coupled = Ctx.GetTrace() / self->GetCoupledVehicleAt(static_cast<ERailroadVehicleCoupler>(coupler));
} EndFunc()
BeginFunc(getTrackGraph, "Get Track Graph", "Returns the track graph of which this vehicle is part of.") {
	OutVal(RStruct<FFINTrackGraph>, track, "Track", "The track graph of which this vehicle is part of.")
	FuncBody()
	track = (FINAny)FFINTrackGraph{Ctx.GetTrace(), self->GetTrackGraphID()};
} EndFunc()
BeginFunc(getTrackPos, "Get Track Pos", "Returns the track pos at which this vehicle is.") {
	OutVal(RTrace<AFGBuildableRailroadTrack>, track, "Track", "The track the track pos points to.")
    OutVal(RFloat, offset, "Offset", "The offset of the track pos.")
    OutVal(RFloat, forward, "Forward", "The forward direction of the track pos. 1 = with the track direction, -1 = against the track direction")
    FuncBody()
    FRailroadTrackPosition pos = self->GetTrackPosition();
	if (!pos.IsValid()) throw FFINException("Railroad Track Position of self is invalid");
	track = Ctx.GetTrace()(pos.Track.Get());
	offset = pos.Offset;
	forward = pos.Forward;
} EndFunc()
BeginFunc(getMovement, "Get Movement", "Returns the vehicle movement of this vehicle.") {
	OutVal(RTrace<UFGRailroadVehicleMovementComponent>, movement, "Movement", "The movement of this vehicle.")
	FuncBody()
	movement = Ctx.GetTrace() / self->GetRailroadVehicleMovementComponent();
} EndFunc()
BeginProp(RFloat, length, "Length", "The length of this vehicle on the track.") {
	Return self->GetLength();
} EndProp()
BeginProp(RBool, isDocked, "Is Docked", "True if this vehicle is currently docked to a platform.") {
	Return self->IsDocked();
} EndProp()
BeginProp(RBool, isReversed, "Is Reversed", "True if the vheicle is placed reversed on the track.") {
	Return self->IsOrientationReversed();
} EndProp()
EndClass()

BeginClass(UFGRailroadVehicleMovementComponent, "RailroadVehicleMovement", "Railroad Vehicle Movement", "This actor component contains all the infomation about the movement of a railroad vehicle.")
BeginFunc(getVehicle, "Get Vehicle", "Returns the vehicle this movement component holds the movement information of.") {
	OutVal(RTrace<AFGRailroadVehicle>, vehicle, "Vehicle", "The vehicle this movement component holds the movement information of.")
	FuncBody()
	vehicle = Ctx.GetTrace() / self->GetOwningRailroadVehicle();
} EndFunc()
BeginFunc(getWheelsetRotation, "Get Wheelset Rotation", "Returns the current rotation of the given wheelset.") {
	InVal(RInt, wheelset, "Wheelset", "The index of the wheelset you want to get the rotation of.")
	OutVal(RFloat, x, "X", "The wheelset's rotation X component.")
	OutVal(RFloat, y, "Y", "The wheelset's rotation Y component.")
	OutVal(RFloat, z, "Z", "The wheelset's rotation Z component.")
	FuncBody()
	FVector rot = self->GetWheelsetRotation(wheelset);
	x = rot.X;
	y = rot.Y;
	z = rot.Z;
} EndFunc()
BeginFunc(getWheelsetOffset, "Get Wheelset Offset", "Returns the offset of the wheelset with the given index from the start of the vehicle.") {
	InVal(RInt, wheelset, "Wheelset", "The index of the wheelset you want to get the offset of.")
	OutVal(RFloat, offset, "Offset", "The offset of the wheelset.")
	FuncBody()
	offset = self->GetWheelsetOffset(wheelset);
} EndFunc()
BeginFunc(getCouplerRotationAndExtention, "Get Coupler Rotation And Extention", "Returns the normal vector and the extention of the coupler with the given index.") {
	InVal(RInt, coupler, "Coupler", "The index of which you want to get the normal and extention of.")
	OutVal(RFloat, x, "X", "The X component of the coupler normal.")
	OutVal(RFloat, y, "Y", "The Y component of the coupler normal.")
	OutVal(RFloat, z, "Z", "The Z component of the coupler normal.")
	OutVal(RFloat, extention, "Extention", "The extention of the coupler.")
	FuncBody()
	float extension;
	FVector rotation = self->GetCouplerRotationAndExtention(coupler, extension);
	x =rotation.X;
	y = rotation.Y;
	z = rotation.Z;
	extention = extension;
} EndFunc()

BeginProp(RFloat, orientation, "Orientation", "The orientation of the vehicle") {
	Return self->GetOrientation();
} EndProp()
BeginProp(RFloat, mass, "Mass", "The current mass of the vehicle.") {
	Return self->GetMass();
} EndProp()
BeginProp(RFloat, tareMass, "Tare Mass", "The tare mass of the vehicle.") {
	Return self->GetTareMass();
} EndProp()
BeginProp(RFloat, payloadMass, "Payload Mass", "The mass of the payload of the vehicle.") {
	Return self->GetPayloadMass();
} EndProp()
BeginProp(RFloat, speed, "Speed", "The current forward speed of the vehicle.") {
	Return self->GetForwardSpeed();
} EndProp()
BeginProp(RFloat, relativeSpeed, "Relative Speed", "The current relative forward speed to the ground.") {
	Return self->GetRelativeForwardSpeed();
} EndProp()
BeginProp(RFloat, maxSpeed, "Max Speed", "The maximum forward speed the vehicle can reach.") {
	Return self->GetMaxForwardSpeed();
} EndProp()
BeginProp(RFloat, gravitationalForce, "Gravitationl Force", "The current gravitational force acting on the vehicle.") {
	Return self->GetGravitationalForce();
} EndProp()
BeginProp(RFloat, tractiveForce, "Tractive Force", "The current tractive force acting on the vehicle.") {
	Return self->GetTractiveForce();
} EndProp()
BeginProp(RFloat, resistiveForce, "Resistive Froce", "The resistive force currently acting on the vehicle.") {
	Return self->GetResistiveForce();
} EndProp()
BeginProp(RFloat, gradientForce, "Gradient Force", "The gradient force currently acting on the vehicle.") {
	Return self->GetGradientForce();
} EndProp()
BeginProp(RFloat, brakingForce, "Braking Force", "The braking force currently acting on the vehicle.") {
	Return self->GetBrakingForce();
} EndProp()
BeginProp(RFloat, airBrakingForce, "Air Braking Force", "The air braking force currently acting on the vehicle.") {
	Return self->GetAirBrakingForce();
} EndProp()
BeginProp(RFloat, dynamicBrakingForce, "Dynamic Braking Force", "The dynamic braking force currently acting on the vehicle.") {
	Return self->GetDynamicBrakingForce();
} EndProp()
BeginProp(RFloat, maxTractiveEffort, "Max Tractive Effort", "The maximum tractive effort of this vehicle.") {
	Return self->GetMaxTractiveEffort();
} EndProp()
BeginProp(RFloat, maxDynamicBrakingEffort, "Max Dynamic Braking Effort", "The maximum dynamic braking effort of this vehicle.") {
	Return self->GetMaxDynamicBrakingEffort();
} EndProp()
BeginProp(RFloat, maxAirBrakingEffort, "Max Air Braking Effort", "The maximum air braking effort of this vehcile.") {
	Return self->GetMaxAirBrakingEffort();
} EndProp()
BeginProp(RFloat, trackGrade, "Track Grade", "The current track grade of this vehicle.") {
	Return self->GetTrackGrade();
} EndProp()
BeginProp(RFloat, trackCurvature, "Track Curvature", "The current track curvature of this vehicle.") {
	Return self->GetTrackCurvature();
} EndProp()
BeginProp(RFloat, wheelsetAngle, "Wheelset Angle", "The wheelset angle of this vehicle.") {
	Return self->GetWheelsetAngle();
} EndProp()
BeginProp(RFloat, rollingResistance, "Rolling Resistance", "The current rolling resistance of this vehicle.") {
	Return self->GetRollingResistance();
} EndProp()
BeginProp(RFloat, curvatureResistance, "Curvature Resistance", "The current curvature resistance of this vehicle.") {
	Return self->GetCurvatureResistance();
} EndProp()
BeginProp(RFloat, airResistance, "Air Resistance", "The current air resistance of this vehicle.") {
	Return self->GetAirResistance();
} EndProp()
BeginProp(RFloat, gradientResistance, "Gradient Resistance", "The current gardient resistance of this vehicle.") {
	Return self->GetGradientResistance();
} EndProp()
BeginProp(RFloat, wheelRotation, "Wheel Rotation", "The current wheel rotation of this vehicle.") {
	Return self->GetWheelRotation();
} EndProp()
BeginProp(RInt, numWheelsets, "Num Wheelsets", "The number of wheelsets this vehicle has.") {
	Return (int64)self->GetNumWheelsets();
} EndProp()
BeginProp(RBool, isMoving, "Is Moving", "True if this vehicle is currently moving.") {
	Return self->IsMoving();
} EndProp()
EndClass()

BeginClass(AFGTrain, "Train", "Train", "This class holds information and references about a trains (a collection of multiple railroad vehicles) and its timetable f.e.")
Hook(UFINTrainHook)
BeginSignal(SelfDrvingUpdate, "Self Drving Update", "Triggers when the self driving mode of the train changes")
	SignalParam(RBool, enabled, "Enabled", "True if the train is now self driving.")
EndSignal()
BeginFunc(getName, "Get Name", "Returns the name of this train.") {
	OutVal(RString, name, "Name", "The name of this train.")
	FuncBody()
	name = self->GetTrainName().ToString();
} EndFunc()
BeginFunc(setName, "Set Name", "Allows to set the name of this train.") {
	InVal(RString, name, "Name", "The new name of this trian.")
	FuncBody()
	self->SetTrainName(FText::FromString(name));
} EndFunc()
BeginFunc(getTrackGraph, "Get Track Graph", "Returns the track graph of which this train is part of.") {
	OutVal(RStruct<FFINTrackGraph>, track, "Track", "The track graph of which this train is part of.")
	FuncBody()
	track = (FINAny) FFINTrackGraph{Ctx.GetTrace(), self->GetTrackGraphID()};
} EndFunc()
BeginFunc(setSelfDriving, "Set Self Driving", "Allows to set if the train should be self driving or not.", 0) {
	InVal(RBool, selfDriving, "Self Driving", "True if the train should be self driving.")
	FuncBody()
	self->SetSelfDrivingEnabled(selfDriving);
} EndFunc()
BeginFunc(getMaster, "Get Master", "Returns the master locomotive that is part of this train.") {
	OutVal(RTrace<AFGLocomotive>, master, "Master", "The master locomotive of this train.")
	FuncBody()
	master = Ctx.GetTrace() / self->GetMultipleUnitMaster();
} EndFunc()
BeginFunc(getTimeTable, "Get Time Table", "Returns the timetable of this train.") {
	OutVal(RTrace<AFGRailroadTimeTable>, timeTable, "Time Table", "The timetable of this train.")
	FuncBody()
	timeTable = Ctx.GetTrace() / self->GetTimeTable();
} EndFunc()
BeginFunc(newTimeTable, "New Time Table", "Creates and returns a new timetable for this train.", 0) {
	OutVal(RTrace<AFGRailroadTimeTable>, timeTable, "Time Table", "The new timetable for this train.")
	FuncBody()
	timeTable = Ctx.GetTrace() / self->NewTimeTable();
} EndFunc()
BeginFunc(getFirst, "Get First", "Returns the first railroad vehicle that is part of this train.") {
	OutVal(RTrace<AFGRailroadVehicle>, first, "First", "The first railroad vehicle that is part of this train.")
	FuncBody()
	first = Ctx.GetTrace() / self->GetFirstVehicle();
} EndFunc()
BeginFunc(getLast, "Get Last", "Returns the last railroad vehicle that is part of this train.") {
	OutVal(RTrace<AFGRailroadVehicle>, last, "Last", "The last railroad vehicle that is part of this train.")
	FuncBody()
	last = Ctx.GetTrace() / self->GetLastVehicle();
} EndFunc()
BeginFunc(dock, "Dock", "Trys to dock the train to the station it is currently at.") {
	FuncBody()
	self->Dock();
} EndFunc()
BeginFunc(getVehicles, "Get Vehicles", "Returns a list of all the vehicles this train has.") {
	OutVal(RArray<RTrace<AFGRailroadVehicle>>, vehicles, "Vehicles", "A list of all the vehicles this train has.")
	FuncBody()
	TArray<FINAny> Vehicles;
	for (AFGRailroadVehicle* vehicle : self->mSimulationData.SimulatedVehicles) {
		Vehicles.Add(Ctx.GetTrace() / vehicle);
	}
	vehicles = Vehicles;
} EndFunc()
BeginProp(RBool, isPlayerDriven, "Is Player Driven", "True if the train is currently player driven.") {
	Return self->IsPlayerDriven();
} EndProp()
BeginProp(RBool, isSelfDriving, "Is Self Driving", "True if the train is currently self driving.") {
	Return self->IsSelfDrivingEnabled();
} EndProp()
BeginProp(RInt, selfDrivingError, "Self Driving Error", "The last self driving error.\n0 = No Error\n1 = No Power\n2 = No Time Table\n3 = Invalid Next Stop\n4 = Invalid Locomotive Placement\n5 = No Path") {
	Return (int64)self->GetSelfDrivingError();
} EndProp()
BeginProp(RBool, hasTimeTable, "Has Time Table", "True if the train has currently a time table.") {
	Return self->HasTimeTable();
} EndProp()
BeginProp(RInt, dockState, "Dock State", "The current docking state of the train.") {
	Return (int64)self->GetDockingState();
} EndProp()
BeginProp(RBool, isDocked, "Is Docked", "True if the train is currently docked.") {
	Return self->IsDocked();
} EndProp()
EndClass()

BeginClass(AFGRailroadTimeTable, "TimeTable", "Time Table", "Contains the time table information of train.")
BeginFunc(addStop, "Add Stop", "Adds a stop to the time table.") {
	InVal(RInt, index, "Index", "The index at which the stop should get added.")
	InVal(RTrace<AFGBuildableRailroadStation>, station, "Station", "The railroad station at which the stop should happen.")
	InVal(RStruct<FTrainDockingRuleSet>, ruleSet, "Rule Set", "The docking rule set that descibes when the train will depart from the station.")
	OutVal(RBool, added, "Added", "True if the stop got sucessfully added to the time table.")
	FuncBody()
	FTimeTableStop stop;
	stop.Station = Cast<AFGBuildableRailroadStation>(station.Get())->GetStationIdentifier();
	stop.DockingRuleSet = ruleSet;
	added = self->AddStop(index, stop);
} EndFunc()
BeginFunc(removeStop, "Remove Stop", "Removes the stop with the given index from the time table.") {
	InVal(RInt, index, "Index", "The index at which the stop should get added.")
	FuncBody()
	self->RemoveStop(index);
} EndFunc()
BeginFunc(getStops, "Get Stops", "Returns a list of all the stops this time table has") {
	OutVal(RArray<RStruct<FFINTimeTableStop>>, stops, "Stops", "A list of time table stops this time table has.")
	FuncBody()
	TArray<FINAny> Output;
	TArray<FTimeTableStop> Stops;
	self->GetStops(Stops);
	for (const FTimeTableStop& Stop : Stops) {
		Output.Add((FINAny)FFINTimeTableStop{Ctx.GetTrace() / Stop.Station->GetStation(), Stop.DockingRuleSet});
	}
	stops = Output;
} EndFunc()
BeginFunc(setStops, "Set Stops", "Allows to empty and fill the stops of this time table with the given list of new stops.") {
	InVal(RArray<RStruct<FFINTimeTableStop>>, stops, "Stops", "The new time table stops.")
	OutVal(RBool, gotSet, "Got Set", "True if the stops got sucessfully set.")
	FuncBody()
	TArray<FTimeTableStop> Stops;
	for (const FINAny& Any : stops) {
		Stops.Add(Any.GetStruct().Get<FFINTimeTableStop>());
	}
	gotSet = self->SetStops(Stops);
} EndFunc()
BeginFunc(isValidStop, "Is Valid Stop", "Allows to check if the given stop index is valid.") {
	InVal(RInt, index, "Index", "The stop index you want to check its validity.")
	OutVal(RBool, valid, "Valid", "True if the stop index is valid.")
	FuncBody()
	valid = self->IsValidStop(index);
} EndFunc()
BeginFunc(getStop, "Get Stop", "Returns the stop at the given index.") {
	InVal(RInt, index, "Index", "The index of the stop you want to get.")
	OutVal(RStruct<FFINTimeTableStop>, stop, "Stop", "The time table stop at the given index.")
	FuncBody()
	FTimeTableStop Stop = self->GetStop(index);
	if (IsValid(Stop.Station)) {
		stop = (FINAny)FFINTimeTableStop{Ctx.GetTrace() / Stop.Station->GetStation(), Stop.DockingRuleSet};
	} else {
		stop = FINAny();
	}
} EndFunc()
BeginFunc(setStop, "Set Stop", "Allows to override a stop already in the time table.") {
	InVal(RInt, index, "Index", "The index of the stop you want to override.")
	InVal(RStruct<FFINTimeTableStop>, stop, "Stop", "The time table stop you want to override with.")
	OutVal(RBool, success, "Success", "True if setting was successful, false if not, f.e. invalid index.")
	FuncBody()
	TArray<FTimeTableStop> Stops;
	self->GetStops(Stops);
	if (index < Stops.Num()) {
		Stops[index] = stop;
		self->SetStops(Stops);
		success = true;
	} else {
		success = false;
	}
} EndFunc()
BeginFunc(setCurrentStop, "Set Current Stop", "Sets the stop, to which the train trys to drive to right now.") {
	InVal(RInt, index, "Index", "The index of the stop the train should drive to right now.")
	FuncBody()
	self->SetCurrentStop(index);
} EndFunc()
BeginFunc(incrementCurrentStop, "Increment Current Stop", "Sets the current stop to the next stop in the time table.") {
	FuncBody()
	self->IncrementCurrentStop();
} EndFunc()
BeginFunc(getCurrentStop, "Get Current Stop", "Returns the index of the stop the train drives to right now.") {
	OutVal(RInt, index, "Index", "The index of the stop the train tries to drive to right now.")
    FuncBody()
    index = (int64) self->GetCurrentStop();
} EndFunc()
BeginProp(RInt, numStops, "Num Stops", "The current number of stops in the time table.") {
	Return (int64)self->GetNumStops();
} EndProp()
EndClass()

BeginClass(AFGBuildableRailroadTrack, "RailroadTrack", "Railroad Track", "A peice of railroad track over which trains can drive.")
Hook(UFINRailroadTrackHook)
BeginSignal(VehicleEnter, "VehicleEnter", "Triggered when a vehicle enters the track.")
	SignalParam(RTrace<AFGRailroadVehicle>, Vehicle, "Vehicle", "The vehicle that entered the track.")
EndSignal()
BeginSignal(VehicleExit, "VehicleExit", "Triggered when a vehcile exists the track.")
	SignalParam(RTrace<AFGRailroadVehicle>, Vehicle, "Vehicle", "The vehicle that exited the track.")
EndSignal()
BeginFunc(getClosestTrackPosition, "Get Closeset Track Position", "Returns the closes track position from the given world position") {
	InVal(RStruct<FVector>, worldPos, "World Pos", "The world position form which you want to get the closest track position.")
	OutVal(RTrace<AFGBuildableRailroadTrack>, track, "Track", "The track the track pos points to.")
    OutVal(RFloat, offset, "Offset", "The offset of the track pos.")
    OutVal(RFloat, forward, "Forward", "The forward direction of the track pos. 1 = with the track direction, -1 = against the track direction")
    FuncBody()
	FRailroadTrackPosition pos = self->FindTrackPositionClosestToWorldLocation(worldPos);
	if (!pos.IsValid()) throw FFINException("Railroad Track Position of self is invalid");
	track = Ctx.GetTrace()(pos.Track.Get());
	offset = pos.Offset;
	forward = pos.Forward;
} EndFunc()
BeginFunc(getWorldLocAndRotAtPos, "Get World Location And Rotation At Position", "Returns the world location and world rotation of the track position from the given track position.") {
	InVal(RTrace<AFGBuildableRailroadTrack>, track, "Track", "The track the track pos points to.")
    InVal(RFloat, offset, "Offset", "The offset of the track pos.")
    InVal(RFloat, forward, "Forward", "The forward direction of the track pos. 1 = with the track direction, -1 = against the track direction")
    OutVal(RStruct<FVector>, location, "Location", "The location at the given track position")
	OutVal(RStruct<FVector>, rotation, "Rotation", "The rotation at the given track position (forward vector)")
	FuncBody()
	FRailroadTrackPosition pos(Cast<AFGBuildableRailroadTrack>(track.Get()), offset, forward);
	FVector loc;
	FVector rot;
	self->GetWorldLocationAndDirectionAtPosition(pos, loc, rot);
	location = (FINAny)loc;
	rotation = (FINAny)rot;
} EndFunc()
BeginFunc(getConnection, "Get Connection", "Returns the railroad track connection at the given direction.") {
	InVal(RInt, direction, "Direction", "The direction of which you want to get the connector from. 0 = front, 1 = back")
	OutVal(RTrace<UFGRailroadTrackConnectionComponent>, connection, "Connection", "The connection component in the given direction.")
	FuncBody()
	connection = Ctx.GetTrace() / self->GetConnection(FMath::Clamp<int>(direction, 0, 1));
} EndFunc()
BeginFunc(getTrackGraph, "Get Track Graph", "Returns the track graph of which this track is part of.") {
	OutVal(RStruct<FFINTrackGraph>, track, "Track", "The track graph of which this track is part of.")
    FuncBody()
    track = (FINAny)FFINTrackGraph{Ctx.GetTrace(), self->GetTrackGraphID()};
} EndFunc()
BeginFunc(getVehciles, "Get Vehicles", "Returns a list of Railroad Vehicles on the Track") {
	OutVal(RArray<RTrace<AFGRailroadVehicle>>, vehicles, "Vehicles", "THe list of vehciles on the track.")
	FuncBody()
	TArray<FINAny> Vehicles;
	for (AFGRailroadVehicle* vehicle : self->GetVehicles()) {
		Vehicles.Add(Ctx.GetTrace() / vehicle);
	}
	vehicles = Vehicles;
} EndFunc()
BeginProp(RFloat, length, "Length", "The length of the track.") {
	Return self->GetLength();
} EndProp()
BeginProp(RBool, isOwnedByPlatform, "Is Owned By Platform", "True if the track is part of/owned by a railroad platform.") {
	Return self->GetIsOwnedByPlatform();
} EndProp()
EndClass()

BeginClass(UFGRailroadTrackConnectionComponent, "RailroadTrackConnection", "Railroad Track Connection", "This is a actor component for railroad tracks that allows to connecto to other track connections and so to connection multiple tracks with each eather so you can build a train network.")
BeginProp(RStruct<FVector>, connectorLocation, "Connector Location", "The world location of the the connection.") {
	Return self->GetConnectorLocation();
} EndProp()
BeginProp(RStruct<FVector>, connectorNormal, "Connector Normal", "The normal vecotr of the connector.") {
	Return self->GetConnectorNormal();
} EndProp()
BeginFunc(getConnection, "Get Connection", "Returns the connected connection with the given index.") {
	InVal(RInt, index, "Index", "The index of the connected connection you want to get.")
	OutVal(RTrace<UFGRailroadTrackConnectionComponent>, connection, "Connection", "The connected connection at the given index.")
	FuncBody()
	connection = Ctx.GetTrace() / self->GetConnection(index);
} EndFunc()
BeginFunc(getConnections, "Get Connections", "Returns a list of all connected connections.") {
	OutVal(RArray<RTrace<UFGRailroadTrackConnectionComponent>>, connections, "Connections", "A list of all connected connections.")
	FuncBody()
	TArray<FINAny> Connections;
	for (UFGRailroadTrackConnectionComponent* conn : self->GetConnections()) {
		Connections.Add(Ctx.GetTrace() / conn);
	}
	connections = Connections;
} EndFunc()
BeginFunc(getTrackPos, "Get Track Pos", "Returns the track pos at which this connection is.") {
	OutVal(RTrace<AFGBuildableRailroadTrack>, track, "Track", "The track the track pos points to.")
    OutVal(RFloat, offset, "Offset", "The offset of the track pos.")
    OutVal(RFloat, forward, "Forward", "The forward direction of the track pos. 1 = with the track direction, -1 = against the track direction")
    FuncBody()
    FRailroadTrackPosition pos = self->GetTrackPosition();
	if (!pos.IsValid()) throw FFINException("Railroad Track Position of self is invalid");
	track = Ctx.GetTrace()(pos.Track.Get());
	offset = pos.Offset;
	forward = pos.Forward;
} EndFunc()
BeginFunc(getTrack, "Get Track", "Returns the track of which this connection is part of.") {
	OutVal(RTrace<AFGBuildableRailroadTrack>, track, "Track", "The track of which this connection is part of.")
	FuncBody()
	track = Ctx.GetTrace() / self->GetTrack();
} EndFunc()
BeginFunc(getSwitchControl, "Get Switch Control", "Returns the switch control of this connection.") {
	OutVal(RTrace<AFGBuildableRailroadSwitchControl>, switchControl, "Switch", "The switch control of this connection.")
	FuncBody()
	switchControl = Ctx.GetTrace() / self->GetSwitchControl();
} EndFunc()
BeginFunc(getStation, "Get Station", "Returns the station of which this connection is part of.") {
	OutVal(RTrace<AFGBuildableRailroadStation>, station, "Station", "The station of which this connection is part of.")
	FuncBody()
	station = Ctx.GetTrace() / self->GetStation();
} EndFunc()
BeginFunc(getFacingSignal, "Get Facing Signal", "Returns the signal this connection is facing to.") {
	OutVal(RTrace<AFGBuildableRailroadSignal>, signal, "Signal", "The signal this connection is facing.")
	FuncBody()
	signal = Ctx.GetTrace() / self->GetFacingSignal();
} EndFunc()
BeginFunc(getTrailingSignal, "Get Trailing Signal", "Returns the signal this connection is trailing from.") {
	OutVal(RTrace<AFGBuildableRailroadSignal>, signal, "Signal", "The signal this connection is trailing.")
	FuncBody()
	signal = Ctx.GetTrace() / self->GetTrailingSignal();
} EndFunc()
BeginFunc(getOpposite, "Get Opposite", "Returns the opposite connection of the track this connection is part of.") {
	OutVal(RTrace<UFGRailroadTrackConnectionComponent>, opposite, "Opposite", "The opposite connection of the track this connection is part of.")
	FuncBody()
	opposite = Ctx.GetTrace() / self->GetOpposite();
} EndFunc()
BeginFunc(getNext, "Get Next", "Returns the next connection in the direction of the track. (used the correct path switched point to)") {
	OutVal(RTrace<UFGRailroadTrackConnectionComponent>, next, "Next", "The next connection in the direction of the track.")
	FuncBody()
	next = Ctx.GetTrace() / self->GetNext();
} EndFunc()
BeginFunc(setSwitchPosition, "Set Switch Position", "Sets the position (connection index) to which the track switch points to.") {
	InVal(RInt, index, "Index", "The connection index to which the switch should point to.")
	FuncBody()
	self->SetSwitchPosition(index);
} EndFunc()
BeginFunc(getSwitchPosition, "Get Switch Position", "Returns the current switch position.") {
	OutVal(RInt, index, "Index", "The index of the connection connection the switch currently points to.")
    FuncBody()
    index = (int64)self->GetSwitchPosition();
} EndFunc()
BeginFunc(forceSwitchPosition, "Force Switch Position", "Forces the switch position to a given location. Even autopilot will be forced to use this track. A negative number can be used to remove the forced track.", 0) {
	InVal(RInt, index, "Index", "The connection index to whcih the switch should be force to point to. Negative number to remove the lock.")
	FuncBody()
	self->SetSwitchPosition(index);
	AFINComputerSubsystem::GetComputerSubsystem(self)->ForceRailroadSwitch(self, index);
} EndFunc()
BeginProp(RBool, isConnected, "Is Connected", "True if the connection has any connection to other connections.") {
	Return self->IsConnected();
} EndProp()
BeginProp(RBool, isFacingSwitch, "Is Facing Switch", "True if this connection is pointing to the merge/spread point of the switch.") {
	Return self->IsFacingSwitch();
} EndProp()
BeginProp(RBool, isTrailingSwitch, "Is Trailing Switch", "True if this connection is pointing away from the merge/spread point of a switch.") {
	Return self->IsTrailingSwitch();
} EndProp()
BeginProp(RInt, numSwitchPositions, "Num Switch Positions", "Returns the number of different switch poisitions this switch can have.") {
	Return (int64)self->GetNumSwitchPositions();
} EndProp()
EndClass()

BeginClass(AFGBuildableRailroadSwitchControl, "RailroadSwitchControl", "Railroad Switch Control", "The controler object for a railroad switch.")
BeginFunc(toggleSwitch, "Toggle Switch", "Toggles the railroad switch like if you interact with it.") {
	FuncBody()
	self->ToggleSwitchPosition();
} EndFunc()
BeginFunc(switchPosition, "Switch Position", "Returns the current switch position of this switch.") {
	OutVal(RInt, position, "Position", "The current switch position of this switch.")
    FuncBody()
    position = (int64)self->GetSwitchPosition();
} EndFunc()
BeginFunc(getControlledConnection, "Get Controlled Connection", "Returns the Railroad Connection this switch is controlling.") {
	OutVal(RTrace<UFGRailroadTrackConnectionComponent>, connection, "Connection", "The controlled connectino.")
	FuncBody()
	connection = Ctx.GetTrace() / self->GetmControlledConnection();
} EndFunc()
EndClass()

BeginClass(AFGBuildableRailroadSignal, "RailroadSignal", "Railroad Signal", "A train signal to control trains on a track.")
Hook(UFINRailroadSignalHook)
BeginSignal(AspectChanged, "Aspect Changed", "Triggers when the aspect of this signal changes.")
	SignalParam(RInt, aspect, "Aspect", "The new aspect of the signal (see 'Get Aspect' for more information)")
EndSignal()
BeginSignal(ValidationChanged, "Validation Changed", "Triggers when the validation of this signal changes.")
	SignalParam(RInt, validation, "Validation", "The new validation of the signal (see 'Block Validation' for more information)")
EndSignal()
BeginProp(RBool, isPathSignal, "Is Path Signal", "True if this signal is a path-signal.") {
	Return self->IsPathSignal();
} EndProp()
BeginProp(RBool, isBiDirectional, "Is Bi-Directional", "True if this signal is bi-directional. (trains can pass into both directions)") {
	Return self->IsBiDirectional();
} EndProp()
BeginProp(RBool, hasObservedBlock, "Has Observed Block", "True if this signal is currently observing at least one block.") {
	Return self->HasObservedBlock();
} EndProp()
BeginProp(RInt, blockValidation, "Block Validation", "Any error states of the block.\n0 = Unknown\n1 = No Error\n2 = No Exit Signal\n3 = Contains Loop\n4 = Contains Mixed Entry Signals") {
	Return (int64)self->GetBlockValidation();
} EndProp()
BeginProp(RInt, aspect, "Aspect", "The aspect of the signal. The aspect shows if a train is allowed to pass (clear) or not and if it should dock.\n0 = Unknown\n1 = The track is clear and the train is allowed to pass.\n2 = The next track is Occupied and the train should stop\n3 = The train should dock.") {
	Return (int64)self->GetAspect();
} EndProp()
BeginFunc(getObservedBlock, "Get Observed Block", "Returns the track block this signals observes.") {
	OutVal(RStruct<FFINRailroadSignalBlock>, block, "Block", "The railroad signal block this signal is observing.")
	FuncBody()
	block = FINStruct(FFINRailroadSignalBlock(self->GetObservedBlock()));
} EndFunc()
BeginFunc(getGuardedConnnections, "Get Guarded Connections", "Returns a list of the guarded connections. (incoming connections)") {
	OutVal(RArray<RTrace<UFGRailroadTrackConnectionComponent>>, guardedConnections, "GuardedConnections", "The guarded connections.")
	FuncBody()
	TArray<FINAny> GuardedConnections;
	for (UFGRailroadTrackConnectionComponent* Connection : self->GetGuardedConnections()) {
		GuardedConnections.Add(Ctx.GetTrace() / Connection);
	}
	guardedConnections = GuardedConnections;
} EndFunc()
BeginFunc(getObservedConnections, "Get Observed Connections", "Returns a list of the observed connections. (outgoing connections)") {
	OutVal(RArray<RTrace<UFGRailroadTrackConnectionComponent>>, observedConnections, "ObservedConnections", "The observed connections.")
	FuncBody()
	TArray<FINAny> ObservedConnections;
	for (UFGRailroadTrackConnectionComponent* Connection : self->GetObservedConnections()) {
		ObservedConnections.Add(Ctx.GetTrace() / Connection);
	}
	observedConnections = ObservedConnections;
} EndFunc()
EndClass()

BeginClass(AFGBuildableDockingStation, "DockingStation", "Docking Station", "A docking station for wheeled vehicles to transfer cargo.")
BeginFunc(getFuelInv, "Get Fueld Inventory", "Returns the fuel inventory of the docking station.") {
	OutVal(RTrace<UFGInventoryComponent>, inventory, "Inventory", "The fuel inventory of the docking station.")
	FuncBody()
	inventory = Ctx.GetTrace() / self->GetFuelInventory();
} EndFunc()
BeginFunc(getInv, "Get Inventory", "Returns the cargo inventory of the docking staiton.") {
	OutVal(RTrace<UFGInventoryComponent>, inventory, "Inventory", "The cargo inventory of this docking station.")
	FuncBody()
	inventory = Ctx.GetTrace() / self->GetInventory();
} EndFunc()
BeginFunc(getDocked, "Get Docked", "Returns the currently docked actor.") {
	OutVal(RTrace<AActor>, docked, "Docked", "The currently docked actor.")
	FuncBody()
	docked = Ctx.GetTrace() / self->GetDockedActor();
} EndFunc()
BeginFunc(undock, "Undock", "Undocked the currently docked vehicle from this docking station.") {
	FuncBody()
	self->Undock(true);
} EndFunc()
BeginProp(RBool, isLoadMode, "Is Load Mode", "True if the docking station loads docked vehicles, flase if it unloads them.") {
	Return self->GetIsInLoadMode();
} PropSet() {
	self->SetIsInLoadMode(Val);
} EndProp()
BeginProp(RBool, isLoadUnloading, "Is Load Unloading", "True if the docking station is currently loading or unloading a docked vehicle.") {
	Return self->IsLoadUnloading();
} EndProp()
EndClass()

BeginClass(AFGBuildablePipeReservoir, "PipeReservoir", "Pipe Reservoir", "The base class for all fluid tanks.")
BeginFunc(flush, "Flush", "Emptys the whole fluid container.") {
	FuncBody()
	AFGPipeSubsystem::Get(self->GetWorld())->FlushIntegrant(self);
} EndFunc()
BeginFunc(getFluidType, "Get Fluid Type", "Returns the type of the fluid.") {
	OutVal(RClass<UFGItemDescriptor>, type, "Type", "The type of the fluid the tank contains.")
	FuncBody()
	type = (UClass*)self->GetFluidDescriptor();
} EndFunc()
BeginProp(RFloat, fluidContent, "Fluid Content", "The amount of fluid in the tank.") {
	Return self->GetFluidBox()->Content;
} EndProp()
BeginProp(RFloat, maxFluidContent, "Max Fluid Content", "The maximum amount of fluid this tank can hold.") {
	Return self->GetFluidBox()->MaxContent;
} EndProp()
BeginProp(RFloat, flowFill, "Flow Fill", "The currentl inflow rate of fluid.") {
	Return self->GetFluidBox()->FlowFill;
} EndProp()
BeginProp(RFloat, flowDrain, "Float Drain", "The current outflow rate of fluid.") {
	Return self->GetFluidBox()->FlowDrain;
} EndProp()
BeginProp(RFloat, flowLimit, "Flow Limit", "The maximum flow rate of fluid this tank can handle.") {
	Return self->GetFluidBox()->FlowLimit;
} EndProp()
EndClass()

BeginClass(AFGBuildablePipelinePump, "PipelinePump", "PipelinePump", "A building that can pump fluids to a higher level within a pipeline.")
BeginProp(RFloat, maxHeadlift, "Max Headlift", "The maximum amount of headlift this pump can provide.") {
	Return self->GetMaxHeadLift();
} EndProp()
BeginProp(RFloat, designedHeadlift, "Designed Headlift", "The amomunt of headlift this pump is designed for.") {
	Return self->GetDesignHeadLift();
} EndProp()
BeginProp(RFloat, indicatorHeadlift, "Indicator Headlift", "The amount of headlift the indicator shows.") {
	Return self->GetIndicatorHeadLift();
} EndProp()
BeginProp(RFloat, indicatorHeadliftPct, "Indicator Headlift Percent", "The amount of headlift the indicator shows as percantage from max.") {
	Return self->GetIndicatorHeadLiftPct();
} EndProp()
BeginProp(RFloat, userFlowLimit, "User Flow Limit", "The flow limit of this pump the user can specifiy. Use -1 for now user set limit. (in m^3/s)") {
	Return self->GetUserFlowLimit();
} PropSet() {
	self->SetUserFlowLimit(Val);
} EndProp()
BeginProp(RFloat, flowLimit, "Flow Limit", "The overal flow limit of this pump. (in m^3/s)") {
	Return self->GetFlowLimit();
} EndProp()
BeginProp(RFloat, flowLimitPct, "Flow Limit Pct", "The overal flow limit of this pump. (in percent)") {
	Return self->GetFlowLimitPct();
} EndProp()
BeginProp(RFloat, flow, "Flow", "The current flow amount. (in m^3/s)") {
	Return self->GetIndicatorFlow();
} EndProp()
BeginProp(RFloat, flowPct, "Float Pct", "The current flow amount. (in percent)") {
	Return self->GetIndicatorFlowPct();
} EndProp()
EndClass()

BeginStructConstructable(FFINTimeTableStop, "TimeTableStop", "Time Table Stop", "Information about a train stop in a time table.")
BeginProp(RTrace<AFGBuildableRailroadStation>, station, "Station", "The station at which the train should stop") {
	Return self->Station;
} PropSet() {
	self->Station = Val;
} EndProp()
BeginFunc(getRuleSet, "Get Rule Set", "Returns The rule set wich describe when the train will depart from the train station.") {
	OutVal(RStruct<FTrainDockingRuleSet>, ruleset, "Rule Set", "The rule set of this time table stop.")
	FuncBody()
	ruleset = FINStruct(self->RuleSet);
} EndFunc()
BeginFunc(setRuleSet, "Set Rule Set", "Allows you to change the Rule Set of this time table stop.") {
	InVal(RStruct<FTrainDockingRuleSet>, ruleset, "Rule Set", "The rule set you want to use instead.")
	FuncBody()
	self->RuleSet = ruleset;
} EndFunc()
EndStruct()

BeginStructConstructable(FTrainDockingRuleSet, "TrainDockingRuleSet", "Train Docking Rule Set", "Contains infromation about the rules that descibe when a trian should depart from a station")
BeginProp(RInt, definition, "Defintion", "0 = Load/Unload Once, 1 = Fully Load/Unload") {
	Return (FINInt)self->DockingDefinition;
} PropSet() {
	self->DockingDefinition = (ETrainDockingDefinition)Val;
} EndProp()
BeginProp(RFloat, duration, "Duration", "The amount of time the train will dock at least.") {
	Return self->DockForDuration;
} PropSet() {
	self->DockForDuration = Val;
} EndProp()
BeginProp(RBool, isDurationAndRule, "Is Duration and Rule", "True if the duration of the train stop and the other rules have to be applied.") {
	Return self->IsDurationAndRule;
} PropSet() {
	self->IsDurationAndRule = Val;
} EndProp()
BeginFunc(getLoadFilters, "Get Load Filters", "Returns the types of items that will be loaded.") {
	OutVal(RArray<RClass<UFGItemDescriptor>>, filters, "Filters", "The item filter array")
	FuncBody()
	TArray<FINAny> Filters;
	for (TSubclassOf<UFGItemDescriptor> Filter : self->LoadFilterDescriptors) {
		Filters.Add((FINClass)Filter);
	}
	filters = Filters;
} EndFunc()
BeginFunc(setLoadFilters, "Set Load Filters", "Sets the types of items that will be loaded.") {
	InVal(RArray<RClass<UFGItemDescriptor>>, filters, "Filters", "The item filter array")
	FuncBody()
	TArray<TSubclassOf<UFGItemDescriptor>> Filters;
	for (const FINAny& Filter : filters) {
		Filters.Add(Filter.GetClass());
	}
	self->LoadFilterDescriptors = Filters;
} EndFunc()
BeginFunc(getUnloadFilters, "Get Unload Filters", "Returns the types of items that will be unloaded.") {
	OutVal(RArray<RClass<UFGItemDescriptor>>, filters, "Filters", "The item filter array")
	FuncBody()
	TArray<FINAny> Filters;
	for (TSubclassOf<UFGItemDescriptor> Filter : self->UnloadFilterDescriptors) {
		Filters.Add((FINClass)Filter);
	}
	filters = Filters;
} EndFunc()
BeginFunc(setUnloadFilters, "Set Unload Filters", "Sets the types of items that will be loaded.") {
	InVal(RArray<RClass<UFGItemDescriptor>>, filters, "Filters", "The item filter array")
	FuncBody()
	TArray<TSubclassOf<UFGItemDescriptor>> Filters;
	for (const FINAny& Filter : filters) {
		Filters.Add(Filter.GetClass());
	}
	self->UnloadFilterDescriptors = Filters;
} EndFunc()
EndStruct()

BeginStruct(FFINTrackGraph, "TrackGraph", "Track Graph", "Struct that holds a cache of a whole train/rail network.")
BeginFunc(getTrains, "Get Trains", "Returns a list of all trains in the network.") {
	OutVal(RArray<RTrace<AFGTrain>>, trains, "Trains", "The list of trains in the network.")
	FuncBody()
	TArray<FINAny> Trains;
	TArray<AFGTrain*> TrainList;
	AFGRailroadSubsystem::Get(*self->Trace)->GetTrains(self->TrackID, TrainList);
	for (AFGTrain* Train : TrainList) {
		Trains.Add(self->Trace / Train);
	}
	trains = Trains;
} EndFunc()
BeginFunc(getStations, "Get Stations", "Returns a list of all trainstations in the network.") {
	OutVal(RArray<RTrace<AFGBuildableRailroadStation>>, stations, "Stations", "The list of trainstations in the network.")
    FuncBody()
    TArray<FINAny> Stations;
	TArray<AFGTrainStationIdentifier*> StationList;
	AFGRailroadSubsystem::Get(*self->Trace)->GetTrainStations(self->TrackID, StationList);
	for (const auto& Station : StationList) {
		Stations.Add(self->Trace / Station->mStation);
	}
	stations = Stations;
} EndFunc()
EndStruct()

BeginStruct(FFINRailroadSignalBlock, "RailroadSignalBlock", "Railroad Signal Block", "A track section that combines the area between multiple signals.")
BeginProp(RBool, isValid, "Is Valid", "Is true if this signal block reference is valid.") {
	Return self->Block.IsValid();
} EndProp()
BeginProp(RBool, isBlockOccupied, "Is Block Occupied", "True if the block this signal is observing is currently occupied by a vehicle.") {
	if (!self->Block.IsValid()) throw FFINException(TEXT("Signalblock is invalid"));
	Return self->Block.Pin()->IsOccupied();
} EndProp()
BeginProp(RBool, hasBlockReservation, "Has Block Reservation", "True if the block this signal is observing has a reservation of a train e.g. will be passed by a train soon.") {
	if (!self->Block.IsValid()) throw FFINException(TEXT("Signalblock is invalid"));
	Return self->Block.Pin()->HaveReservations();
} EndProp()
BeginProp(RBool, isPathBlock, "Is Path Block", "True if the block this signal is observing is a path-block.") {
	if (!self->Block.IsValid()) throw FFINException(TEXT("Signalblock is invalid"));
	Return self->Block.Pin()->IsPathBlock();
} PropSet() {
	if (!self->Block.IsValid()) throw FFINException(TEXT("Signalblock is invalid"));
	self->Block.Pin()->SetIsPathBlock(Val);
} EndProp()
BeginProp(RInt, blockValidation, "Block Validation", "Returns the blocks validation status.") {
	if (!self->Block.IsValid()) throw FFINException(TEXT("Signalblock is invalid"));
	Return (int64)self->Block.Pin()->GetBlockValidation();
} EndProp()
BeginFunc(isOccupiedBy, "Is Occupied By", "Allows you to check if this block is occupied by a given train.") {
	InVal(RObject<AFGTrain>, train, "Train", "The train you want to check if it occupies this block")
	OutVal(RBool, isOccupied, "Is Occupied", "True if the given train occupies this block.")
	FuncBody()
	if (!self->Block.IsValid()) throw FFINException(TEXT("Signalblock is invalid"));
	isOccupied = self->Block.Pin()->IsOccupiedBy(train.Get());
} EndFunc()
BeginFunc(getOccupation, "Get Occupation", "Returns a list of trains that currently occupate the block.") {
	OutVal(RArray<RTrace<AFGTrain>>, occupation, "Occupation", "A list of trains occupying the block.")
	FuncBody()
	if (!self->Block.IsValid()) throw FFINException(TEXT("Signalblock is invalid"));
	TArray<FINAny> Occupation;
	for (TWeakObjectPtr<AFGRailroadVehicle> train : FStaticReflectionSourceHelper::FFGRailroadSignalBlock_GetOccupiedBy(*self->Block.Pin())) {
		if (train.IsValid()) Occupation.Add(Ctx.GetTrace() / train.Get());
	}
	occupation = Occupation;
} EndFunc()
BeginFunc(getQueuedReservations, "Get Queued Reservations", "Returns a list of trains that try to reserve this block and wait for approval.") {
	OutVal(RArray<RTrace<AFGTrain>>, reservations, "Reservations", "A list of trains that try to reserve this block and wait for approval.")
	FuncBody()
	if (!self->Block.IsValid()) throw FFINException(TEXT("Signalblock is invalid"));
	TArray<FINAny> Reservations;
	for (TSharedPtr<FFGRailroadBlockReservation> Reservation : FStaticReflectionSourceHelper::FFGRailroadSignalBlock_GetQueuedReservations(*self->Block.Pin())) {
		if (!Reservation.IsValid()) continue;
		AFGTrain* Train = Reservation->Train.Get();
		if (Train) Reservations.Add(Ctx.GetTrace() / Train);
	}
	reservations = Reservations;
} EndFunc()
BeginFunc(getApprovedReservations, "Get Approved Reservations", "Returns a list of trains that are approved by this block.") {
	OutVal(RArray<RTrace<AFGTrain>>, reservations, "Reservations", "A list of trains that are approved by this block.")
	FuncBody()
	if (!self->Block.IsValid()) throw FFINException(TEXT("Signalblock is invalid"));
	TArray<FINAny> Reservations;
	for (TSharedPtr<FFGRailroadBlockReservation> Reservation : FStaticReflectionSourceHelper::FFGRailroadSignalBlock_GetApprovedReservations(*self->Block.Pin())) {
		if (!Reservation.IsValid()) continue;
		AFGTrain* Train = Reservation->Train.Get();
		if (Train) Reservations.Add(Ctx.GetTrace() / Train);
	}
	reservations = Reservations;
} EndFunc()
EndStruct()

BeginStructConstructable(FFINTargetPoint, "TargetPoint", "Target Point", "Target Point in the waypoint list of a wheeled vehicle.")
BeginProp(RStruct<FVector>, pos, "Pos", "The position of the target point in the world.") {
	Return self->Pos;
} PropSet() {
	self->Pos = Val;
} EndProp()
BeginProp(RStruct<FRotator>, rot, "Rot", "The rotation of the target point in the world.") {
	Return self->Rot;
} PropSet() {
	self->Rot = Val;
} EndProp()
BeginProp(RFloat, speed, "Speed", "The speed at which the vehicle should pass the target point.") {
	Return self->Speed;
} PropSet() {
	self->Speed = Val;
} EndProp()
BeginProp(RFloat, wait, "Wait", "The amount of time which needs to pass till the vehicle will continue to the next target point.") {
	Return self->Wait;
} PropSet() {
	self->Wait = Val;
} EndProp()
EndStruct()
