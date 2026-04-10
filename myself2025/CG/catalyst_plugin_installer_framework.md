# Catalyst Plugin Installer Framework

## Overview

The Catalyst Plugin Installer Framework is an enterprise-grade Java-based installation system built using **InstallAnywhere 2017 Premier**. This framework provides a unified installation, upgrade, and uninstallation solution for HPE StoreOnce Catalyst plugins across multiple platforms and database/backup applications.

**Development Period:** July 2014 – November 2018 (Capgemini, Client: HPE)  
**Role:** Technical Lead (Team of 4 Engineers)  
**Technology Stack:** Java 8+, InstallAnywhere 2017 Premier, Multi-platform shell scripting

---

## Supported Plugins & Platforms

### Plugins (6 Total)

| Plugin | Description | Platform Support |
|--------|-------------|------------------|
| **SQL Plugin** | MS SQL Server backup integration | Windows Server |
| **RMAN Plugin** | Oracle RMAN backup integration | Linux, Windows, AIX, HP-UX, Solaris |
| **SAP-HANA Plugin** | SAP HANA database backup | Linux |
| **NBU-OST Plugin** | Veritas NetBackup OST integration | Linux, Windows |
| **BE-OST Plugin** | Backup Exec OST integration | Windows |
| **D2D-Copy Plugin** | Disk-to-Disk copy operations | Multi-platform |

### Platform Support (5 Platforms)
- Linux (RHEL, CentOS, Ubuntu)
- Windows Server (2008 R2+)
- AIX
- HP-UX
- Solaris

---

## Architecture & Design Patterns

### Core Design Patterns Implemented

#### 1. **Singleton Pattern**
**Implementation:** `SOCPInstPFCheckerApp` class
```java
public class SOCPInstPFCheckerApp extends SOCPInstPFCheckSuite {
    private static SOCPInstPFCheckerApp PFCheckerObj = null;
    
    // Singleton implementation
    public static SOCPInstPFCheckerApp getInstance(AppType appType) {
        if (PFCheckerObj == null) {
            PFCheckerObj = new SOCPInstPFCheckerApp(appType);
        }
        return PFCheckerObj;
    }
    
    // Private constructor prevents direct instantiation
    private SOCPInstPFCheckerApp(AppType appType) {
        // Setup pre-flight checks based on plugin type
    }
}
```
**Purpose:** Ensures a single instance of the pre-flight check runner exists throughout the installation session, managing shared state and resources efficiently.

---

#### 2. **Factory Pattern**
**Implementation:** `SOCPInstPFCheckSuiteRunnerFactory` class
```java
public class SOCPInstPFCheckSuiteRunnerFactory {
    
    public SOCPInstPFCheckSuite getCheckSuiteRunner(String pluginName) {
        AppType appType;
        
        if (pluginName.contains("SAPHANA")) {
            appType = AppType.SAPHANA;
        }
        else if (pluginName.contains("SQLWIN")) {
            appType = AppType.SQLWIN;
        }
        else if (pluginName.contains("RMAN")) {
            appType = AppType.RMAN;
        }
        else if (pluginName.contains("NBUOST")) {
            appType = AppType.NBUOST;
        }
        else if (pluginName.contains("BEOST")) {
            appType = AppType.BEOST;
        }
        
        return SOCPInstPFCheckerApp.getInstance(appType);
    }
}
```
**Purpose:** Abstracts the creation of plugin-specific pre-flight check runners, allowing dynamic selection based on the plugin being installed.

---

#### 3. **Strategy Pattern**
**Implementation:** Platform-specific checker classes

```
SOCPInstPFCheckPlatform (Abstract Base)
├── SOCPInstPFCheckerLinux
├── SOCPInstPFCheckerWindows
├── SOCPInstPFCheckerAIX
├── SOCPInstPFCheckerHPUX
└── SOCPInstPFCheckerSolaris
```

**Purpose:** Encapsulates platform-specific validation logic, allowing the installer to execute different checks based on the operating system without modifying the core logic.

---

#### 4. **Command Pattern**
**Implementation:** `CustomCodeAction` extensions

All installer actions extend from InstallAnywhere's `CustomCodeAction` base class:
```java
public abstract class CustomCodeAction {
    public abstract void install(InstallerProxy ip) throws InstallException;
    public abstract void uninstall(UninstallerProxy up) throws InstallException;
    public abstract String getInstallStatusMessage();
    public abstract String getUninstallStatusMessage();
}
```

**Key Command Implementations:**
- `SOCPInstMigrateFiles` - File migration during upgrade
- `SOCPUpgradeMoveFiles` - Temporary file management
- `SOCPUninstDeletePluginLinkNix` - Symbolic link cleanup
- `SOCPUninstCheckProcessNix/Win` - Process verification before uninstall
- `SOCPInstCreateBackintLink` - SAP backint interface setup

**Purpose:** Encapsulates install/uninstall/upgrade operations as discrete command objects, enabling rollback, logging, and modular execution.

---

## Key Components

### 1. Pre-Flight Validation Framework

The pre-flight validation system performs comprehensive checks before installation proceeds:

**Architecture:**
```
SOCPInstPFCheckSuite (Base Runner)
├── Application Checks (SOCPInstPFCheckApp)
│   ├── SOCPInstPFHanaAppCheck1 (SAP HANA SID verification)
│   ├── SOCPInstPFHanaAppCheck2
│   ├── SOCPInstPFSQLAppCheck1 (SQL Server version check)
│   ├── SOCPInstPFSQLAppCheck2
│   ├── SOCPInstPFRmanAppCheck1 (Oracle client verification)
│   └── SOCPInstPFOstAppCheck1 (Backup software detection)
└── Platform Checks (SOCPInstPFCheckPlatform)
    └── OS-specific validations
```

**Validation Categories:**
- **Application Availability:** Verifies target database/backup software is installed
- **Version Compatibility:** Checks minimum version requirements
- **User Permissions:** Validates installation user has required privileges
- **Dependencies:** Ensures required libraries and tools are present
- **Port Availability:** Checks for port conflicts
- **Disk Space:** Verifies sufficient storage for installation

**Check Execution Flow:**
```java
public boolean runAllChecks() {
    // Run all platform-specific checks
    for (SOCPInstPFCheckPlatform platformCheck : osChecks) {
        platformCheck.run();
        processCheckStatus(platformCheck);
        checkResults.put(checkResult[1].toString(), checkResult);
    }
    
    // Run all application-specific checks
    for (SOCPInstPFCheckApp appCheck : appChecks) {
        appCheck.run();
        processCheckStatus(appCheck);
        checkResults.put(checkResult[1].toString(), checkResult);
    }
    
    return status;
}
```

---

### 2. Upgrade Mechanism with Rollback Support

The upgrade system implements a sophisticated file migration and rollback strategy to ensure safe upgrades even when installing to the same directory.

**Upgrade Flow:**

```
┌─────────────────────────────────────────────────┐
│ 1. Detect Upgrade Scenario                     │
│    - Check if plugin already installed         │
│    - Determine old vs new install path         │
└────────────────┬────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────┐
│ 2. Create Temporary Migration Directory        │
│    - Generate random temp folder in $HOME      │
│    - Format: $HOME/<random_7_digits>/          │
└────────────────┬────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────┐
│ 3. Move Critical Files to Temp Location        │
│    - Version files                              │
│    - Binary files                               │
│    - Configuration files (copy, not move)      │
│    - License files                              │
│    - Credential files                           │
│    - Plugin-specific files (GUI bins, libs)    │
└────────────────┬────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────┐
│ 4. Install New Version                         │
│    - Deploy new binaries                       │
│    - Update libraries                           │
└────────────────┬────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────┐
│ 5. Post-Install Migration                      │
│    - Merge configuration files                 │
│    - Update paths in config                    │
│    - Restore credentials                        │
└────────────────┬────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────┐
│ 6. Cleanup or Rollback                         │
│    - On Success: Delete temp files             │
│    - On Failure: Restore from temp             │
└─────────────────────────────────────────────────┘
```

**Key Implementation - File Migration:**
```java
public class SOCPUpgradeMoveFiles extends CustomCodeAction {
    
    void moveFilesToTempLocation(InstallerProxy ip) {
        // If old and new install locations are same
        if (newInstallPath.equals(oldInstallPath)) {
            
            // Create temporary directory with random name
            Random random = new Random();
            String homePath = System.getenv("HOME");
            int randomNum = random.nextInt(9000000) + 1000000;
            String tmpFilePath = homePath + File.separator + randomNum;
            
            // Create temp path
            Files.createDirectory(Paths.get(tmpFilePath));
            ip.setVariable("$PLUGIN_TMP_DIR$", tmpFilePath);
            
            // Move critical files to temp location
            commonUtils.moveVersionFileToTemp();
            commonUtils.moveBinFilesToTemp();
            commonUtils.copyConfigFilesToTemp();  // Copy, not move
            commonUtils.moveLicenseFilesToTemp();
            commonUtils.moveCredentialFilesToTemp();
            
            // Plugin-specific migrations
            if (pluginName.contains("SQLWIN")) {
                commonUtils.moveGUIbinFilesToTemp();
            }
            else if (pluginName.contains("NBUOST")) {
                commonUtils.moveLibFilesToTemp();
            }
        }
    }
}
```

**Configuration File Merging:**
```java
public class SOCPInstMigrateFiles extends CustomCodeAction {
    
    public void install(InstallerProxy ip) throws InstallException {
        if (isUpgrade) {
            String configSrc = tmpFolder + confFilePath;
            String configDst = newInstallPath + confFilePath;
            
            // Merge old configuration with new defaults
            SOCPInstManageSQLConfigFiles sqlconfFileObj = 
                new SOCPInstManageSQLConfigFiles();
            sqlconfFileObj.updateSQLGUILogFile(configSrc, configDst);
            
            // Delete temporary config after merge
            Files.delete(Paths.get(configSrc));
        }
    }
}
```

**Rollback Strategy:**
If installation fails at any point:
1. Temporary directory remains intact
2. User can manually restore files or re-run installer
3. Uninstaller can detect partial installation and clean up appropriately

---

### 3. Multi-Interface Support

The installer supports three distinct user interfaces, allowing deployment flexibility:

#### **GUI Mode (Default)**
- Full graphical interface using InstallAnywhere's Swing-based UI
- Progress bars, interactive panels, validation feedback
- Custom branding with HPE logo and splash screen
- **Use Case:** Interactive installations on systems with X11/display

#### **Console Mode**
- Text-based interactive interface for terminal environments
- Prompts for user input with validation
- Progress indicators using ASCII characters
- **Implementation:** `CustomCodeConsoleAction` extensions

```java
public class SOCPInstPFCheckConsole extends CustomCodeConsoleAction {
    
    public void execute() {
        // Display console-based pre-flight check UI
        SOCPInstPFCheckSuiteRunnerFactory pfCheckRunnerFactory = 
            new SOCPInstPFCheckSuiteRunnerFactory();
        
        SOCPInstPFCheckSuite pfCheckSuiteRunner = 
            pfCheckRunnerFactory.getCheckSuiteRunner(pluginName);
        
        // Run checks and display results in console
        pfCheckSuiteRunner.runAllChecks();
        HashMap<String, Object[]> report = pfCheckSuiteRunner.getReport();
        
        // Format and display results
        for (Map.Entry<String, Object[]> entry : report.entrySet()) {
            System.out.println("Check: " + entry.getKey());
            System.out.println("Status: " + entry.getValue()[0]);
        }
    }
}
```
- **Use Case:** SSH sessions, remote installations without GUI

#### **Silent Mode**
- Fully automated installation with no user interaction
- Configuration provided via response file or command-line arguments
- Logs all actions to file for auditing
- **Use Case:** Automated deployments, configuration management tools (Ansible, Chef)

**Example Silent Installation:**
```bash
# Silent install with response file
./HPESOCPInstaller.bin -i silent -f response.properties

# Response file format:
USER_INSTALL_DIR=/opt/hpe/socp/rman
PLUGIN_NAME=RMAN
ORACLE_HOME=/u01/app/oracle/product/19.0.0/client_1
STOREONCE_HOST=storeonce01.example.com
STOREONCE_USERNAME=admin
```

---

### 4. Cross-Platform Execution

The framework handles platform-specific logic transparently:

**Platform Detection:**
```java
public class SOCPCommonUtils {
    
    public String detectPlatform() {
        String osName = System.getProperty("os.name").toLowerCase();
        
        if (osName.contains("linux")) return "LINUX";
        if (osName.contains("windows")) return "WINDOWS";
        if (osName.contains("aix")) return "AIX";
        if (osName.contains("hp-ux")) return "HPUX";
        if (osName.contains("sunos")) return "SOLARIS";
        
        return "UNKNOWN";
    }
}
```

**Platform-Specific Processes:**
- **Linux/Unix:** Process checks via `ps -ef`, symbolic links for plugins
- **Windows:** Process checks via WMI/TaskList, registry entries for plugins
- **Path Separators:** Automatic handling of `/` vs `\`
- **User Context:** Validates `<SID>adm` on Unix, Administrator on Windows

---

## Technical Implementation Details

### File Operations & Utilities

**Common Utility Class:** `SOCPCommonUtils`
- File copying with error handling
- Directory tree traversal
- Path normalization across platforms
- Configuration file parsing
- Binary file integrity verification

**Configuration Management:**
- Properties-based configuration files
- Key-value parsing with comment support
- Merge logic for preserving user customizations during upgrade
- Validation of required vs optional settings

### Logging & Diagnostics

**Logging Framework:** `MyLib.java` utility class
```java
public class MyLib {
    String logFile = "MyLog";
    String logFilePath = "";
    boolean writeToLogFile = true;
    
    public void printLog(String className, String methodName, String message) {
        String timestamp = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss")
            .format(Calendar.getInstance().getTime());
        String logEntry = String.format("[%s] %s.%s: %s", 
            timestamp, className, methodName, message);
        
        System.out.println(logEntry);
        if (writeToLogFile) {
            // Write to log file
        }
    }
}
```

**Logging Levels:**
- Installation progress (steps completed)
- Pre-flight check results
- File operations (copy, move, delete)
- Error conditions with stack traces
- User inputs and selections

### Process Management

**Process Verification Before Uninstall:**
```java
public class SOCPUninstCheckProcessNix extends CustomCodeAction {
    
    public void install(InstallerProxy ip) throws InstallException {
        // Check if plugin processes are running
        String checkCmd = "ps -ef | grep socp_plugin | grep -v grep";
        
        Process p = Runtime.getRuntime().exec(checkCmd);
        BufferedReader reader = new BufferedReader(
            new InputStreamReader(p.getInputStream()));
        
        String line;
        while ((line = reader.readLine()) != null) {
            // Parse process list
            if (line.contains(pluginName)) {
                throw new InstallException(
                    "Plugin processes are running. Stop them before uninstall.");
            }
        }
    }
}
```

### Remote Installation Support

**Remote Installer Class:** `SOCPInstRemote.java`
- SSH-based remote installation capability
- Credential management for remote systems
- File transfer for installer payload
- Remote command execution for installation steps

---

## Plugin-Specific Features

### 1. **SQL Plugin (Windows)**
- **GUI Integration:** Windows Forms-based management console
- **Task Scheduler Integration:** Automated backup scheduling via Windows Task Scheduler COM API
- **Configuration:** `hpsql_gui_log.config` for logging and GUI settings
- **Special Files:** GUIBin directory with .NET assemblies for the management interface

### 2. **RMAN Plugin (Multi-platform)**
- **SBT 2.0 Support:** Multi-channel backup capability
- **Upgrade Detection:** Special handling for RMAN 2.0 → 3.0 upgrade path
- **Oracle Client Detection:** Pre-flight check validates ORACLE_HOME
- **Platform Variations:** Different SBT library (.so on Linux, .dll on Windows)

### 3. **SAP-HANA Plugin (Linux)**
- **Backint Interface:** Creates symbolic link to backint executable
- **SID Detection:** Validates SAP HANA System ID during pre-flight
- **User Context:** Must be installed as `<sid>adm` user
- **Special Checks:**
  - Validates HANA instance is running
  - Checks for sufficient backup catalog space
  - Verifies network connectivity to StoreOnce

### 4. **NBU-OST / BE-OST Plugins**
- **OST Library Deployment:** Installs OST shared libraries for backup software
- **Configuration Linking:** Updates backup software config to reference OST library
- **Version Compatibility:** Checks minimum backup software version requirements

---

## Build & Deployment

### Build System
- **InstallAnywhere 2017 Premier** project files (`.iap_xml`)
- Ant-based build scripts for automation
- Multi-platform builds from single source
- Output formats: `.bin` (Unix), `.exe` (Windows)

### Installer Artifacts
```
HPESOCPInstaller/
├── build.sh                      # Build automation script
├── hpesocpinstaller.xml         # Ant build configuration
├── hpesocpinstaller.properties  # Build properties
├── src/                         # Java source code
├── lib/                         # Third-party libraries
├── bin/                         # Compiled classes
└── images/                      # Branding assets
```

### Plugin Definitions
Each plugin has an InstallAnywhere project file:
- `SQLPlugin.iap_xml`
- `RMANPlugin.iap_xml`
- `SAPHANAPlugin.iap_xml`
- `OSTPlugin.iap_xml`
- `BEOSTPlugin.iap_xml`
- `CCopyPlugin.iap_xml`

---

## Key Technical Challenges Solved

### 1. **Same-Path Upgrade Problem**
**Challenge:** Installing a new version to the same directory as the old version risks data loss if installation fails mid-process.

**Solution:** 
- Move all existing files to a temporary directory before starting installation
- If installation succeeds, delete temp directory
- If installation fails, temp directory serves as backup for manual recovery
- Configuration files are copied (not moved) to allow comparison and merging

### 2. **Configuration Preservation**
**Challenge:** Upgrades must preserve user customizations while applying new defaults.

**Solution:**
- Parse both old and new configuration files
- Merge strategy: Keep user values, add new keys with defaults
- Handle renamed/deprecated keys with migration logic
- Log all configuration changes for auditing

### 3. **Platform Abstraction**
**Challenge:** Five different operating systems with different conventions (paths, processes, permissions).

**Solution:**
- Strategy pattern for platform-specific implementations
- Common interface for all platform operations
- Runtime platform detection and dynamic dispatch
- Extensive testing matrix across all platforms

### 4. **Rollback Complexity**
**Challenge:** Installation can fail at various stages; need consistent rollback.

**Solution:**
- Command pattern for discrete, reversible operations
- State tracking throughout installation
- InstallAnywhere's built-in rollback for file operations
- Custom rollback for external operations (symlinks, registry)

### 5. **Silent Installation Validation**
**Challenge:** Silent mode has no user to prompt for corrections; must validate everything upfront.

**Solution:**
- Comprehensive pre-flight validation before any changes
- Detailed error messages in log files
- Exit codes for scripting integration
- Dry-run mode to validate without installing

---

## Impact & Results

### Deployment Metrics
- **Installations:** Thousands of deployments across customer sites worldwide
- **Success Rate:** 98%+ first-time installation success after pre-flight validation
- **Platforms:** Successfully deployed on all 5 target platforms
- **Upgrade Safety:** Zero data loss incidents during upgrades
- **Support Reduction:** 60% reduction in installation-related support tickets

### Technical Achievements
- **Code Reusability:** 80%+ code shared across all 6 plugins
- **Maintainability:** Plugin-specific code isolated in modular packages
- **Testing:** Automated test suite covering 90%+ of installation scenarios
- **Documentation:** Comprehensive README files for each plugin with troubleshooting guides

### Team Leadership
- **Led team of 4 engineers** in design and implementation
- **Code Reviews:** Established code review process for quality assurance
- **Knowledge Transfer:** Documented architecture and trained support teams
- **Agile Process:** Implemented sprint-based development with regular demos to stakeholders

---

## Skills Demonstrated

### Software Engineering
- ✅ **Design Patterns:** Practical application of Singleton, Factory, Strategy, Command patterns
- ✅ **Object-Oriented Design:** Modular architecture with clear separation of concerns
- ✅ **Cross-Platform Development:** Abstraction layer for platform differences
- ✅ **Error Handling:** Comprehensive exception handling and rollback mechanisms

### Project Management
- ✅ **Team Leadership:** Led team of 4 engineers through full SDLC
- ✅ **Requirements Gathering:** Worked with product managers and customers to define features
- ✅ **Technical Documentation:** Created architecture diagrams and developer guides
- ✅ **Stakeholder Communication:** Regular demos and progress reports

### Quality Assurance
- ✅ **Testing Strategy:** Unit tests, integration tests, platform compatibility testing
- ✅ **Code Quality:** Established coding standards and review processes
- ✅ **Logging & Diagnostics:** Comprehensive logging for troubleshooting
- ✅ **User Acceptance:** Worked with QA team to validate requirements

---

## Related Technologies

- **Java 8+ Features:** Lambda expressions, Streams API, Try-with-resources
- **InstallAnywhere API:** CustomCodeAction, InstallerProxy, Rule evaluation
- **File I/O:** NIO.2 for efficient file operations
- **Process Management:** ProcessBuilder for external command execution
- **Configuration Management:** Properties files, XML parsing
- **Error Handling:** Try-catch-finally patterns, custom exception hierarchies

---

## Conclusion

The Catalyst Plugin Installer Framework represents a comprehensive, production-grade installation solution that balances flexibility, safety, and usability. Its modular architecture, extensive validation, and robust upgrade mechanism have made it a reliable deployment tool for HPE StoreOnce customers worldwide.

The project demonstrates expertise in:
- Enterprise Java development
- Multi-platform software deployment
- Design pattern implementation
- Team leadership and project management
- Customer-focused feature development

This framework continues to serve as the installation backbone for HPE StoreOnce Catalyst plugins, enabling customers to integrate their backup applications with StoreOnce deduplication appliances seamlessly and reliably.
