/*
 * Command.cpp
 *
 * Simplified concrete command implementations for practice.
 * Original SqlPluginGui commands executed real backup/restore CLI tools.
 * Here we simulate work with sleep + console output.
 */
#include "Command.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>

std::string BackupCmd::Execute() {
    std::ostringstream oss;
    oss << "[BackupCmd] Executing: " << m_cmd
        << " (thread: " << std::this_thread::get_id() << ")";
    std::cout << oss.str() << std::endl;

    // Simulate backup work
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::string result = "Backup completed: " + m_cmd;
    std::cout << "[BackupCmd] " << result << std::endl;
    return result;
}

std::string RestoreCmd::Execute() {
    std::ostringstream oss;
    oss << "[RestoreCmd] Executing: " << m_cmd
        << " (thread: " << std::this_thread::get_id() << ")";
    std::cout << oss.str() << std::endl;

    // Simulate restore work
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::string result = "Restore completed: " + m_cmd;
    std::cout << "[RestoreCmd] " << result << std::endl;
    return result;
}

std::string ListJobsCmd::Execute() {
    std::ostringstream oss;
    oss << "[ListJobsCmd] Executing: " << m_cmd
        << " (thread: " << std::this_thread::get_id() << ")";
    std::cout << oss.str() << std::endl;

    // Simulate list jobs
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::string result = "ListJobs completed: " + m_cmd;
    std::cout << "[ListJobsCmd] " << result << std::endl;
    return result;
}
