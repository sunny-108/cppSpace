# IPC Command Pattern - Simple Explanation

## What is IPC?

**IPC = Inter-Process Communication**

Think of it like two people in different rooms who can't talk directly to each other. They need to pass notes under the door to communicate.

### Real-World Example:

```
You (Program A)  →  [Note under door]  →  Your Friend (Program B)
     ↓                                           ↓
  Writes note                                Reads note
     ↓                                           ↓
  "Can you get                               "Sure! Here's
   me coffee?"                                your coffee"
     ↑                                           ↓
  Reads reply   ←  [Note under door]  ←    Writes reply
```

In computers:
- **Program A** = SAP HANA database
- **Program B** = Backup plugin
- **Note** = File on disk

---

## What is Command Pattern?

The Command Pattern treats each action as a separate "command object" - like having different types of request forms.

### Real-World Example: Restaurant Orders

Instead of the waiter remembering everything in their head, they write each order on a slip:

```
┌──────────────────────┐
│  ORDER SLIP #1       │
│  Table: 5            │
│  Item: Burger        │
│  Special: No onions  │
└──────────────────────┘

┌──────────────────────┐
│  ORDER SLIP #2       │
│  Table: 3            │
│  Item: Pizza         │
│  Special: Extra cheese│
└──────────────────────┘
```

**Benefits:**
- Clear instructions (no confusion)
- Can queue orders (process one at a time)
- Can track order status
- Can cancel specific orders
- Can redo orders if needed

---

## IPC + Command Pattern Together

Combining both = **Passing structured command messages between programs**

### Simple Example: Coffee Shop App

Imagine you have a mobile app that talks to a coffee machine:

```
┌─────────────────┐                    ┌─────────────────┐
│   Mobile App    │                    │ Coffee Machine  │
│  (Your Phone)   │                    │   (Kitchen)     │
└─────────────────┘                    └─────────────────┘
        │                                      │
        │  1. Write order to file              │
        │     "order.txt":                     │
        │     MAKE_COFFEE                      │
        │     SIZE: LARGE                      │
        │     TYPE: LATTE                      │
        ├─────────────────────────────────────►│
        │                                      │
        │                                      │ 2. Read file
        │                                      │    Execute command
        │                                      │    Make latte
        │                                      │
        │  3. Write result to file             │
        │     "result.txt":                    │
        │     STATUS: SUCCESS                  │
        │     READY_IN: 2 minutes              │
        │◄─────────────────────────────────────┤
        │                                      │
        │  4. Read result                      │
        │     Show: "Coffee ready!"            │
        │                                      │
```

### The Files Are the "IPC" Part:
- Programs communicate by reading/writing files
- Programs don't need to run at the same time
- Programs can be in different languages

### The Structured Commands Are the "Command Pattern" Part:
- Each command has a clear format
- Easy to add new commands (MAKE_TEA, MAKE_ESPRESSO)
- Easy to log what was requested
- Easy to handle errors

---

## Real Code Example: Simple Calculator

Let's build a simple calculator using IPC Command Pattern:

### Program A: Command Sender (command_sender.py)

```python
# Write command to file
with open("command.txt", "w") as f:
    f.write("ADD\n")
    f.write("10\n")
    f.write("20\n")

print("Command sent: ADD 10 and 20")

# Read result from file
with open("result.txt", "r") as f:
    result = f.read()
    print(f"Result: {result}")
```

### Program B: Command Executor (command_executor.py)

```python
# Read command from file
with open("command.txt", "r") as f:
    lines = f.readlines()
    command = lines[0].strip()
    num1 = int(lines[1].strip())
    num2 = int(lines[2].strip())

# Execute command
if command == "ADD":
    result = num1 + num2
elif command == "MULTIPLY":
    result = num1 * num2
elif command == "SUBTRACT":
    result = num1 - num2

# Write result to file
with open("result.txt", "w") as f:
    f.write(str(result))

print(f"Executed {command}: {result}")
```

### Running It:

```bash
# Terminal 1: Send command
$ python command_sender.py
Command sent: ADD 10 and 20

# Terminal 2: Process command
$ python command_executor.py
Executed ADD: 30

# Terminal 1 now reads result:
Result: 30
```

---

## How SAP HANA Plugin Uses This

### The Setup:

```
┌─────────────────┐                    ┌─────────────────┐
│   SAP HANA      │                    │  Backup Plugin  │
│   Database      │                    │  (Our Code)     │
└─────────────────┘                    └─────────────────┘
```

### Step 1: SAP HANA Writes Command File

**File: /tmp/backup_input.txt**
```
#PIPE /backup/pipe1
#PIPE /backup/pipe2
#PIPE /backup/pipe3
```

Translation: "Please backup these 3 data streams"

### Step 2: SAP HANA Calls Plugin

```bash
backint -f /tmp/backup_input.txt -o /tmp/backup_output.txt -u BACKUP
```

Translation: 
- `-f`: Input file (commands to execute)
- `-o`: Output file (where to write results)
- `-u`: Function (BACKUP operation)

### Step 3: Plugin Reads and Executes Commands

```cpp
// Plugin code (simplified)
void processBackupCommands() {
    // 1. PARSE INPUT FILE (IPC)
    vector<string> pipes = readInputFile("/tmp/backup_input.txt");
    // Result: ["/backup/pipe1", "/backup/pipe2", "/backup/pipe3"]
    
    // 2. CREATE COMMAND OBJECTS (Command Pattern)
    for (each pipe in pipes) {
        BackupCommand* cmd = new BackupCommand(pipe);
        commands.push_back(cmd);
    }
    
    // 3. EXECUTE COMMANDS
    for (each cmd in commands) {
        cmd->execute();  // Backup the data
    }
}
```

### Step 4: Plugin Writes Result File

**File: /tmp/backup_output.txt**
```
#SOFTWAREID "HPE Data Protector / 1.0.0"
#SAVED EBID001 /backup/pipe1 104857600
#SAVED EBID002 /backup/pipe2 209715200
#SAVED EBID003 /backup/pipe3 157286400
```

Translation: 
- Successfully backed up all 3 streams
- Each has a unique ID and size in bytes

### Step 5: SAP HANA Reads Results

SAP HANA opens `/tmp/backup_output.txt` and confirms all backups succeeded!

---

## Different Command Types

The plugin supports three main commands:

### 1. BACKUP Command

**Input:**
```
#PIPE /path/to/data
```

**What it does:** Read data from the pipe and save to backup storage

**Output:**
```
#SAVED EBID_12345 /path/to/data 1073741824
```

### 2. RESTORE Command

**Input:**
```
EBID_12345 #PIPE /path/to/restore
```

**What it does:** Read backed-up data and write to the pipe

**Output:**
```
#RESTORED EBID_12345 /path/to/restore
```

### 3. DELETE Command

**Input:**
```
EBID_12345
```

**What it does:** Delete the backup with this ID

**Output:**
```
#DELETED EBID_12345
```
or
```
#NOTFOUND EBID_12345
```

---

## Why Is This Design Good?

### 1. **Separation of Concerns**

SAP HANA doesn't need to know HOW backups work. It just writes commands and reads results.

```
SAP HANA: "I need these backed up"
Plugin: "Done! Here are the results"
```

### 2. **Easy to Test**

You can test by manually creating input files:

```bash
# Create test command
echo "#PIPE /test/data" > test_input.txt

# Run plugin
./backint -f test_input.txt -o test_output.txt -u BACKUP

# Check result
cat test_output.txt
```

### 3. **Easy to Debug**

All commands and results are in text files - just open and read!

```bash
# See what SAP HANA requested
cat /tmp/backup_input.txt

# See what plugin responded
cat /tmp/backup_output.txt
```

### 4. **Language Independent**

SAP HANA is written in C++, plugin is written in C++, but they could be ANY language:
- Python script could send commands
- Java program could execute them
- Files are the universal interface

### 5. **Extensible**

Need a new command? Just add it:

```cpp
// Add new command type
if (command == "VERIFY") {
    verifyBackup();
}
```

Update the input format:
```
VERIFY
EBID_12345
```

Update the output format:
```
#VERIFIED EBID_12345 OK
```

---

## Visual Summary

```
┌──────────────────────────────────────────────────────────────┐
│                    IPC COMMAND PATTERN                       │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│  Program A                         Program B                │
│  ┌────────┐                        ┌────────┐               │
│  │ Writer │                        │ Reader │               │
│  └───┬────┘                        └───▲────┘               │
│      │                                 │                    │
│      │ 1. Write Commands               │ 2. Read Commands   │
│      │    (Structured text)            │    (Parse format)  │
│      ▼                                 │                    │
│  ┌─────────────────────────────────────┴─────┐              │
│  │         COMMAND FILE                      │              │
│  │  ┌──────────────────────────────────┐     │              │
│  │  │ COMMAND_TYPE                     │     │              │
│  │  │ PARAMETER_1                      │     │              │
│  │  │ PARAMETER_2                      │     │              │
│  │  └──────────────────────────────────┘     │              │
│  └────────┬────────────────────────────────────┘             │
│           │                                 ▲                │
│           │ 4. Write Results                │ 3. Execute     │
│           │    (Status, data)               │                │
│           ▼                                 │                │
│  ┌────────────────────────────────────────────┐              │
│  │         RESULT FILE                       │              │
│  │  ┌──────────────────────────────────┐     │              │
│  │  │ STATUS: SUCCESS                  │     │              │
│  │  │ DATA: Result values              │     │              │
│  │  └──────────────────────────────────┘     │              │
│  └──────────┬──────────────────────────────────┘             │
│             │                                                │
│             │ 5. Read Results                                │
│             ▼                                                │
│         ┌────────┐                                           │
│         │  Done! │                                           │
│         └────────┘                                           │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

---

## Key Takeaways

1. **IPC** = Programs talking to each other (using files, pipes, sockets, etc.)

2. **Command Pattern** = Treating each action as a structured object

3. **Together** = Structured messages passed between programs

4. **Benefits:**
   - Programs stay independent
   - Easy to debug (inspect files)
   - Easy to test (create fake commands)
   - Easy to extend (add new commands)
   - Language agnostic

5. **Real-world analogy:** Like ordering food through a drive-through:
   - You speak into speaker (IPC)
   - You give a structured order (Command Pattern)
   - Kitchen gets order slip
   - Kitchen makes food
   - You get result

---

## Practice Exercise

Try creating your own IPC Command Pattern system!

**Task:** Build a simple TODO list with two programs:

1. **add_task.py** - Writes tasks to `tasks.txt`
2. **list_tasks.py** - Reads and displays tasks from `tasks.txt`

**File format:**
```
ADD Buy milk
ADD Call dentist
ADD Read book
```

Try it yourself, then expand:
- Add REMOVE command
- Add COMPLETE command
- Add PRIORITY levels

This is exactly how professional systems work - just more complex!
