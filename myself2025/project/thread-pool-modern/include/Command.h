/*
 * Command.h
 *
 * Modern C++17 version of the Command pattern from SqlPluginGui.
 *
 * Original (Windows API):
 *   - Raw pointer ownership (Command*)
 *   - Static bool locks for backup/restore
 *
 * Modern C++17 changes:
 *   - std::unique_ptr ownership throughout
 *   - Removed static bool locks (use proper synchronization externally)
 *   - std::string_view where appropriate
 */
#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <memory>

class Command {
public:
    explicit Command(const std::string& cmd) : m_cmd(cmd) {}
    virtual ~Command() = default;

    // Non-copyable, movable
    Command(const Command&) = delete;
    Command& operator=(const Command&) = delete;
    Command(Command&&) = default;
    Command& operator=(Command&&) = default;

    virtual std::string Execute() = 0;
    virtual std::string Type() const { return "Command"; }

    void setCmd(const std::string& cmd) { m_cmd = cmd; }
    const std::string& getCmd() const { return m_cmd; }

protected:
    std::string m_cmd;
};

// --- Concrete Commands (simplified for practice) ---

class BackupCmd : public Command {
public:
    explicit BackupCmd(const std::string& cmd) : Command(cmd) {}
    std::string Execute() override;
    std::string Type() const override { return "BackupCmd"; }
};

class RestoreCmd : public Command {
public:
    explicit RestoreCmd(const std::string& cmd) : Command(cmd) {}
    std::string Execute() override;
    std::string Type() const override { return "RestoreCmd"; }
};

class ListJobsCmd : public Command {
public:
    explicit ListJobsCmd(const std::string& cmd) : Command(cmd) {}
    std::string Execute() override;
    std::string Type() const override { return "ListJobsCmd"; }
};

#endif // COMMAND_H
