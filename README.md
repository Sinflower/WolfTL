# WolfTL

This is a simple tool for extracting data relevant for translation from WolfRPG `.dat` and `.mps` files and storing them in JSON files.

## Usage

```bash
WolfTL.exe <DATA-FOLDER> <OUTPUT-FOLDER> <MODE>
```

Possible modes are:<br>
`create` - Create the Patch<br>
`patch`  - Apply the Patch

After creating the Patch, the `OUTPUT-FOLDER` will contain a folder called `dump`, which contains folders for the three different types (CommonEvents, Databases, and Maps).
After applying the Patch, a new folder called `patched` will be created, containing the `data` folder with the updated files inside.

### Example Execution

```bash
# Create the Patch for a game located at D:\Wolf\Game and write the output to D:\Work\Game
WolfTL.exe D:\Wolf\Game\Data D:\Work\Game create
# Apply the Patch
WolfTL.exe D:\Wolf\Game\Data D:\Work\Game patch
```

If any of the paths contains a space, the path needs to be enclosed in	quotation marks, e.g.:
```bash
WolfTL.exe "D:\Path with Spaces\Game\Data" "D:\Other Path with Spaces\Game" create
```

## Note

Very new, and WolfPro games are currently not supported. If the game is not supported, an error message regarding invalid magic should be printed.
