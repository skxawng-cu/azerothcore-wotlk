/*
 * Copyright (C) 2016+     AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-GPL2
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 */

/// \addtogroup Trinityd Trinity Daemon
/// @{
/// \file

#include <openssl/opensslv.h>
#include <openssl/crypto.h>
#include <ace/Version.h>

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Configuration/Config.h"

#include "Log.h"
#include "Master.h"

#ifndef _ACORE_CORE_CONFIG
#define _ACORE_CORE_CONFIG "worldserver.conf"
#endif

#ifdef _WIN32
#include "ServiceWin32.h"
char serviceName[] = "worldserver";
char serviceLongName[] = "AzerothCore world service";
char serviceDescription[] = "AzerothCore World of Warcraft emulator world service";
/*
 * -1 - not in service mode
 *  0 - stopped
 *  1 - running
 *  2 - paused
 */
int m_ServiceStatus = -1;
#endif

WorldDatabaseWorkerPool WorldDatabase;                      ///< Accessor to the world database
CharacterDatabaseWorkerPool CharacterDatabase;              ///< Accessor to the character database
LoginDatabaseWorkerPool LoginDatabase;                      ///< Accessor to the realm/login database

uint32 realmID;                                             ///< Id of the realm

/// Print out the usage string for this program on the console.
void usage(const char* prog)
{
    sLog->outString("Usage:\n"
                    " %s [<options>]\n"
                    "    -c config_file           use config_file as configuration file\n\r", prog);
#ifdef _WIN32
    sLog->outString("    Running as service functions:\n"
                    "    --service                run as service\n"
                    "    -s install               install service\n"
                    "    -s uninstall             uninstall service\n\r");
#endif
}

/// Launch the Trinity server
extern int main(int argc, char** argv)
{
    ///- Command line parsing to get the configuration file name
    char const* configFile = _ACORE_CORE_CONFIG;
    int count = 1;
    while (count < argc)
    {
        if (strcmp(argv[count], "--dry-run") == 0)
        {
            sConfigMgr->setDryRun(true);
        }

        if (!strcmp(argv[count], "-c"))
        {
            if (++count >= argc)
            {
                printf("Runtime-Error: -c option requires an input argument\n");
                usage(argv[0]);
                return 1;
            }
            else
                configFile = argv[count];
        }

#ifdef _WIN32
        if (strcmp(argv[count], "-s") == 0) // Services
        {
            if (++count >= argc)
            {
                printf("Runtime-Error: -s option requires an input argument\n");
                usage(argv[0]);
                return 1;
            }

            if (strcmp(argv[count], "install") == 0)
            {
                if (WinServiceInstall())
                    printf("Installing service\n");
                return 1;
            }
            else if (strcmp(argv[count], "uninstall") == 0)
            {
                if (WinServiceUninstall())
                    printf("Uninstalling service\n");
                return 1;
            }
            else
            {
                printf("Runtime-Error: unsupported option %s\n", argv[count]);
                usage(argv[0]);
                return 1;
            }
        }

        if (strcmp(argv[count], "--service") == 0)
            WinServiceRun();
#endif
        ++count;
    }

    sConfigMgr->SetConfigList(std::string(configFile), std::string(CONFIG_FILE_LIST));

    if (!sConfigMgr->LoadAppConfigs())
        return 1;

    sLog->outString("Using configuration file %s.", configFile);
    sLog->outString("Using SSL version: %s (library: %s)", OPENSSL_VERSION_TEXT, SSLeay_version(SSLEAY_VERSION));
    sLog->outString("Using ACE version: %s", ACE_VERSION);

    ///- and run the 'Master'
    /// @todo Why do we need this 'Master'? Can't all of this be in the Main as for Realmd?
    int ret = sMaster->Run();

    // at sMaster return function exist with codes
    // 0 - normal shutdown
    // 1 - shutdown at error
    // 2 - restart command used, this code can be used by restarter for restart Trinityd

    return ret;
}

/// @}
