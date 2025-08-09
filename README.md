# GOAP
 A GOAP solution for Unreal Engine 5

# What is it?
### The GOAP system is a plugin for [Unreal engine 5](https://www.unrealengine.com/en-US/) that provides an implemetation for **Goal Oriented Action Planning** for AI in Unreal Engine. This plugin relies on the Struct Utils module.

This plugin is mostly bare bones. It does provide basic goal types though such as Location goals for moving the agent to a location.

The GOAP system plugin is being used in our projects and any improvements we make to the plugin will reflect here.

##Architecture 
* FGOAPContext: A struct that defines a context for anything in the GOAP system. This is a struct with two members: a tag container for all the relevant context tags for this context and a FGOAPValue property to hold the context value. Every context in the GOAP system is derived from the FGOAPContext struct. The FGOAPContext struct itself is usually sufficient in most cases.
* FGOAPValue: A struct that defines a value in the GOAP system. Values used in the GOAP system are all derived from the GOAPValue struct.
* FGOAPGoal: This is a struct that defines a goal for GOAP. The GOAP system allows actions to need "requirements". These are also GOAPGoal structs.
* UGOAPAction: This is a class that defines an action. These are singletons and there will always only be one instance of each action type.
* UGOAPSensor: This is a class that defines a sensor for the GOAP system. These can be anything from checking walls to checking to see if agent is being attacked. They run before planner update and store info on the UGOAPAgentComponent.
* UGOAPSubsystem: Currently useless... This will probably used to batch planner updates in the future. Currently updates are done on tick.
* UGOAPAgentComponent: Add to an actor class to make it a GOAP agent. The agent component takes sensor data and finds the highest priority goal to complete and tells the planner to generate an action plan for that goal.
* FGOAPPlanner: This handles generating an action plan based on the sensor data and goals. Action plan is a branching tree and all actions are handled starting from the leaf nodes and ending at the root node. When a plan is complete a new plan is generated. When a plan fails a new plan is generated. A new plan will not be generated if an exisiting plan can satisfy the current high priority goal.

## Getting Started


# Highlights
### Graph editor
Extremely modular. This comes with the trade-off of being a bit complicated to get used to.
