#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "server.h"

/**
 * Set *UID and *GID to the owning user ID and group ID, respectively,
 * of process PID. Return 0 on success, nonzero on failure.
 */

static int get_uid_gid (pid_t pid, uid_t* uid, gid_t* gid)
{
    char dir_name[64];
    struct stat dir_info;
    int rval;

    /** Generate the name of the process's directory in /proc. */
    snprintf (dir_name, sizeof (dir_name), "proc/%d", (int) pid);

    /** Obtain information about the directory. */
    rval = stat (dir_name, &dir_info);

    if (rval != 0)
    {
        /** Couldn't find it; perhaps this process no longer exists. */
        return 1;
    }    

    /** Make sure it's a directory; anything else is unexpected. */
    assert (S_ISDIR (dir_info.st_mode));

    /** Extract the IDs we want. */
    *uid = dir_info.st_uid;
    *gid = dir_info.st_gid;

    return 0;
}

/**
 * Return the name of user UID. The return value is a better that the 
 * caller must allocate with free. UID must be a valid user ID.
 */

static char* get_user_name (uid_t uid)
{
    struct passwd* entry;

    entry = getpwuid (uid);
    if (entry == NULL)
    {
        
    }
}
