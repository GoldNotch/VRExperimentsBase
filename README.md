# VRExperimentsBase  
Plugin for Unreal Engine 4 to quick start VR project without C++ programming (in SciVI environment)  

It provides VR character(BaseInformant) which has setuped VR headset(Vive HTC Pro Eye), controllers and can interact to scene objects by default.  Also it collects information about EyeTrack and interactions and sends it in SciVi (via websocket).  

# Editor setup  
Plugin requires some setup to work correctrly.  
1) Install plugin [JsonBlueprint](https://www.unrealengine.com/marketplace/en-US/product/json-blueprint) to engine - it provides ability to construct and parse json messages in blueprint (SciVi Communination format is JSON)
2) Install plugin (Vive SRanipal SDK for UE4)[https://developer.vive.com/resources/vive-sense/eye-and-facial-tracking-sdk/] to project
3) Set Inputs in ProjectSettings:  
  - ActionMappings: RTrigger = {Vive(R) Trigger}, Walking  = {Vive(R) Trackpad Up}  
  - AxisMappings: CameraMove_RightLeft = {MouseX}, CameraMove_UpDown = {MouseY} - it's for debug, to launch game without headset  
4) Create two SoundSubmixes in Content directory: CaptureSubmix and MuteSubmix. Link them in CaptureSubmix -> MuteSubmix way.And set OutputVolume in MuteSubmix to 0.0. They needs for sound recording.
5) Create ForceFeedbackEffect in Content Directory. Create a Curve. To understand why - read (this)[https://docs.unrealengine.com/4.27/en-US/InteractiveExperiences/ForceFeedback/]
7) Create Informant blueprint class (inherit from BaseInformant). Set Controllers meshes (by default UE4 has HTC Controller Meshes)

# Interactions  
BaseInformant can interact with actors scene by default. It provides some kinds of interaction: eye tracking, controller ray hover/leave actor, trigger pressed/released on actor, actor had close/far to informant.  
All actors player can interact inherit from InteractableActor. So you're sopposed to create blueprint actor from InteractableActor and define necessary events.  
Full list of events in InteractableActor Blueprint:  
- Had Eye Focus - calls when informant has just looked at actor  
- Lost Eye Focus  - calls when informant has just looked away eyes from actor  
- Eye Track Tick - calls every tick while informant is looking at actor (here you can send GazeInfo to SciVi)  
- On RTrigger Pressed - calls when you press RTrigger on InteractableActor  
- On RTrigger Released - calls when you release RTrigger  
- Had Controller Focus - calls when actor is pointed by controller's ray  
- Lost Controller Focus - calls when actor is no longer pointed by controller's ray  
- Controller Focus Tick - calls every tick while Informant is pointing to Actor by controller's ray  
- Had Close To Player - calls when Actor become in InteractionRadius  
- Had Far To player - calls when Actor is out of InteractionRadius  

# UI Interaction
Plugin provides interaction with UMG(with all default widgets). All interactable Widgets must be 3D actors and placed in level.
## How to create Interactable UI?
1) Create new Widget Blueprint (Add -> UserInterface -> Widget Blueprint). Limit the size with current values of width and height (use custom size).
2) Place all widgets in limited area
3) Create new Blueprint Actor(inherit from UI_Blank). UI_Blank it's InteractableActor which has WidgetComponent. Choose WidgetComponent and assign created in step 1 Widget class to  WidgetClass setting(in UserInterface section). Also assign Widget's width and height to DrawSize setting.
4) In BeginPlay event of actor you can set events on buttons and other widgets. For that, get WidgetComponent->GetWidget() -> Cast To WidgetClass from step 1. Then get elements like variable, find Bind Event to Clicked(for example) and set custom event.
5) Place created UI_Blank actor in the level.

# Communication with SciVi
SciVi is platform for collecting data and experiment's controlling.
VRGameMode class can communicate with SciVi via next mehtods:
- SendToSciVi(FString json_message), where json_message is stringifed json object/value, where name of json is message ID. f.e "Gaze" : {<gaze data>}.
- OnSciViMessageReceived(JsonObject msg_json) - event, calls when SciVi sends message as json value/object, where name of object is command and body of object is command's arguments. F.e. "HideController": "left". In Event you can understand which command you get and process it.
