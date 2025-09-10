#include "PluginAPI.h"

#include "Definitions.hpp"
#include "CrashLogger.hpp"
#include "Logging.hpp"
#include <iostream>

IDebugLog    gLog("CrashLogger.log");
PluginHandle g_pluginHandle = kPluginHandle_Invalid;

std::filesystem::path GetCurPath()
{
    char buffer[MAX_PATH] = { 0 };
    GetModuleFileName(nullptr, buffer, MAX_PATH);
    const std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos);
}

void InitLog(const std::filesystem::path& path = "")
{
    auto logPath = GetCurPath();
    if (!path.empty()) logPath /= path;
    logPath /= CrashLogger_LOG;

    auto logFolderPath = GetCurPath();
    if (!path.empty()) logFolderPath /= path;
    logFolderPath /= CrashLogger_FLD;
    logFolderPath /= CrashLogger_LOG;

    Logger::AddDestinations(logPath, CrashLogger_STR, LogLevel::LogFile);
    Logger::PrepareCopy(logPath, logFolderPath);

    Log(LogLevel::LogConsole) << CrashLogger_VERSION_STR;

    Log(LogLevel::LogWarning) << "Crash Logger Improved version " << CrashLogger_VERSION_STR << " at " + std::format("{0:%F} {0:%T}", std::chrono::time_point(std::chrono::system_clock::now())) <<
        '\n'
        << "If this file is empty, then your game didn't crash or something went so wrong even crash logger was useless!" <<
        '\n'
        << "Topmost stack module is NOT ALWAYS the crash reason! Exercise caution when speculating!" << '\n';

    Log(LogLevel::LogWarning) << "When asking for help, please send the whole log file!\n";
}

extern "C" {
   bool OBSEPlugin_Query(const OBSEInterface* obse, PluginInfo* info) {
      // fill out the info structure
      info->infoVersion = PluginInfo::kInfoVersion;
      info->name        = "CrashLoggerImproved";
      info->version     = 0x01060100; // major, minor, patch, build

      {  // log our version number -- be helpful!
         auto v = info->version;
         UInt8 major = v >> 0x18;
         UInt8 minor = (v >> 0x10) & 0xFF;
         UInt8 patch = (v >> 0x08) & 0xFF;
         UInt8 build = v & 0xFF;
         _MESSAGE("Crash Logger Improved Version %d.%d.%d, build %d.", major, minor, patch, build);
         _MESSAGE("If this file is empty, then your game didn't crash! Probably!\n");
      }

      // version checks
      if(!obse->isEditor) {
         if(obse->obseVersion < 21) {
            _ERROR("OBSE version too old (got %08X; expected at least %08X).", obse->obseVersion, 21);
            return false;
         }
         if(obse->oblivionVersion != OBLIVION_VERSION_1_2_416) {
             _ERROR("incorrect Oblivion version (got %08X; need %08X).", obse->oblivionVersion, OBLIVION_VERSION_1_2_416);
             return false;
         }
      } else {
         // no version checks needed for editor
      }
      // version checks pass
      return true;
   }

   bool OBSEPlugin_Load(const OBSEInterface* obse) {
      g_pluginHandle = obse->GetPluginHandle();
      
      if (!obse->isEditor){
          InitLog(GetCurPath());
          Inits();
      }
          //CobbPatches::CrashLog::Apply(false);
      return true;
   }
};
