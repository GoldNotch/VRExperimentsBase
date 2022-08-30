# VRExperimentsBase  
Plugin for Unreal Engine 4 to quick start VR project on C++ (in SciVI environment)  

It provides VR character(BaseInformant) which has setuped VR headset, controllers and can interact to scene objects by default.  Also it collects information about EyeTrack and interactions and sends it in SciVi (via websocket).  

# Editor setup  
Plugin requires some setup to work correctrly.  
1) Install plugin [JsonBlueprint](https://www.unrealengine.com/marketplace/en-US/product/json-blueprint)  
2) Set Inputs in ProjectSettings:  
  - ActionMappings: RTrigger = {Vive(R) Trigger}, Walking  = {Vive(R) Trackpad Up}  
  - AxisMappings: CameraMove_RightLeft = {MouseX}, CameraMove_UpDown = {MouseY} - it's for debug, to launch game without headset  

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
