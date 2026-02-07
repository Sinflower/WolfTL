# WolfTL

This is a simple tool for extracting translation-relevant data from WolfRPG `.dat` and `.mps` files and storing them in JSON files.<BR>
The data parsing code is based on [Wolf Trans](https://github.com/elizagamedev/wolftrans).

## Usage

```bash
WolfTL.exe <DATA-FOLDER> <OUTPUT-FOLDER> <MODE> [OPTION]
```

Possible modes are:<br>
`create`   - Create the Patch<br>
`patch`    - Apply the Patch<br>
`patch_ip` - Apply the Patch in place, i.e., override the original data files

Possible options are:<br>
`no_gd`    - Skip Game.dat

After creating the Patch, the `OUTPUT-FOLDER` will contain a folder called `dump`, which contains folders for the three types (CommonEvents, Databases, and Maps).
After applying the Patch, a new folder called `patched` will be created, containing the `data` folder with the updated files inside.

### Example Execution

```bash
# Create the Patch for a game located at D:\Wolf\Game and write the output to D:\Work\Game
WolfTL.exe D:\Wolf\Game\Data D:\Work\Game create
# Apply the Patch
WolfTL.exe D:\Wolf\Game\Data D:\Work\Game patch
```

If any of the paths contain a space, the path needs to be enclosed in	quotation marks, e.g.:
```bash
WolfTL.exe "D:\Path with Spaces\Game\Data" "D:\Other Path with Spaces\Game" create
```

## Note

WolfPro games are supported. After patching, the files will be unencrypted, allowing them to be opened in any Wolf RPG editor.

## WolfRPG Commands

The following is a list of all the different commands used within WolfRPG and their respective code.

| Command               | Code | Description                                                       |
|-----------------------|------|-------------------------------------------------------------------|
| Blank                 | 0    | Empty command; used as padding or placeholder                      |
| Checkpoint            | 99   | Sets a resume or checkpoint position within the event             |
| Message               | 101  | Displays a message window with text                                |
| Choices               | 102  | Displays a choice selection to the player                          |
| Comment               | 103  | Developer comment; has no effect at runtime                        |
| ForceStopMessage      | 105  | Immediately closes the current message window                     |
| DebugMessage          | 106  | Displays a debug-only message                                     |
| ClearDebugText        | 107  | Clears accumulated debug text                                     |
| VariableCondition     | 111  | Branch based on a variable’s value                                 |
| StringCondition       | 112  | Branch based on a string variable’s value                          |
| SetVariable           | 121  | Assigns or modifies a numeric variable                             |
| SetString             | 122  | Assigns or modifies a string variable                              |
| InputKey              | 123  | Waits for or checks player key input                               |
| SetVariableEx         | 124  | Advanced variable operations such as calculations or references   |
| AutoInput             | 125  | Simulates player input automatically                               |
| BanInput              | 126  | Disables specific player inputs                                    |
| Teleport              | 130  | Moves the player to another map or location                        |
| Sound                 | 140  | Plays or controls sound effects or background music               |
| Picture               | 150  | Displays, moves, or removes pictures                               |
| ChangeColor           | 151  | Changes screen color tone or applies filters                       |
| SetTransition         | 160  | Sets the screen transition effect                                  |
| PrepareTransition     | 161  | Preloads the selected transition                                   |
| ExecuteTransition     | 162  | Executes the prepared transition                                   |
| StartLoop             | 170  | Begins an infinite loop block                                      |
| BreakLoop             | 171  | Exits the current loop                                             |
| BreakEvent            | 172  | Terminates the current event execution                              |
| EraseEvent            | 173  | Erases the event from the map until reload                          |
| ReturnToTitle         | 174  | Returns the game to the title screen                               |
| EndGame               | 175  | Ends the game and closes the application                           |
| StartLoop2            | 176  | Begins an alternative loop type used internally by the engine     |
| StopNonPic            | 177  | Stops non-picture drawing or updates                               |
| ResumeNonPic          | 178  | Resumes non-picture drawing or updates                             |
| LoopTimes             | 179  | Repeats a loop a specified number of times                         |
| Wait                  | 180  | Waits for a specified duration                                    |
| Move                  | 201  | Moves an event or character                                       |
| WaitForMove           | 202  | Waits until movement is complete                                   |
| CommonEvent           | 210  | Executes a common event by ID                                      |
| CommonEventReserve    | 211  | Reserves a common event to run later                                |
| SetLabel              | 212  | Defines a jump label within the event                              |
| JumpLabel             | 213  | Jumps execution to a specified label                               |
| SaveLoad              | 220  | Opens the save or load selection screen                            |
| LoadGame              | 221  | Loads a saved game                                                 |
| SaveGame              | 222  | Saves the current game state                                       |
| MoveDuringEventOn     | 230  | Allows player movement during events                               |
| MoveDuringEventOff    | 231  | Disables player movement during events                             |
| Chip                  | 240  | Changes a map chip tile at runtime                                  |
| ChipSet               | 241  | Changes tileset or chip configuration                              |
| Database              | 250  | Modifies database values at runtime                                |
| ImportDatabase        | 251  | Imports external database data                                     |
| Party                 | 270  | Manages party members, such as adding or removing characters      |
| MapEffect             | 280  | Applies effects tied to the map                                    |
| ScrollScreen          | 281  | Scrolls the screen view                                            |
| Effect                | 290  | Executes visual or special effects                                 |
| CommonEventByName     | 300  | Executes a common event by name                                    |
| ChoiceCase            | 401  | Branch case for a specific choice                                  |
| SpecialChoiceCase     | 402  | Special branch case, such as default or extra                      |
| ElseCase              | 420  | Else branch of a conditional                                       |
| CancelCase            | 421  | Branch executed when a choice is canceled                          |
| LoopEnd               | 498  | Marks the end of a loop block                                       |
| BranchEnd             | 499  | Marks the end of a conditional branch                              |
| Default               | 999  | Fallback or undefined command handler                              |
| ProFeature            | 1000 | Feature available only in the Pro version                          |
