/* Copyright (c) NSPR Contributors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <nspr.h>

#ifdef __cplusplus
extern "C"
{
#endif

PRIntn _PR_InvalidInt(void);

PRInt16 _PR_InvalidInt16(void);

PRInt64 _PR_InvalidInt64(void);

PRStatus _PR_InvalidStatus(void);

PRFileDesc *_PR_InvalidDesc(void);


void _MD_unix_map_recv_error(int err);

void _MD_unix_map_send_error(int err);

void _MD_unix_map_connect_error(int err);

void _MD_unix_map_getsockname_error(int err);

void _MD_unix_map_getpeername_error(int err);

void _MD_unix_map_default_error(int err);

void _MD_unix_map_opendir_error(int err);

void _MD_unix_map_closedir_error(int err);

void _MD_unix_readdir_error(int err);

void _MD_unix_map_unlink_error(int err);

void _MD_unix_map_stat_error(int err);

void _MD_unix_map_fstat_error(int err);

void _MD_unix_map_rename_error(int err);

void _MD_unix_map_access_error(int err);

void _MD_unix_map_mkdir_error(int err);

void _MD_unix_map_rmdir_error(int err);

void _MD_unix_map_read_error(int err);

void _MD_unix_map_write_error(int err);

void _MD_unix_map_lseek_error(int err);

void _MD_unix_map_fsync_error(int err);

void _MD_unix_map_close_error(int err);

void _MD_unix_map_socket_error(int err);

void _MD_unix_map_socketavailable_error(int err);

void _MD_unix_map_recv_error(int err);

void _MD_unix_map_recvfrom_error(int err);

void _MD_unix_map_send_error(int err);

void _MD_unix_map_sendto_error(int err);

void _MD_unix_map_writev_error(int err);

void _MD_unix_map_accept_error(int err);

void _MD_unix_map_connect_error(int err);

void _MD_unix_map_bind_error(int err);

void _MD_unix_map_listen_error(int err);

void _MD_unix_map_shutdown_error(int err);

void _MD_unix_map_socketpair_error(int err);

void _MD_unix_map_getsockname_error(int err);

void _MD_unix_map_getpeername_error(int err);

void _MD_unix_map_getsockopt_error(int err);

void _MD_unix_map_setsockopt_error(int err);

void _MD_unix_map_open_error(int err);

void _MD_unix_map_mmap_error(int err);

void _MD_unix_map_gethostname_error(int err);

void _MD_unix_map_select_error(int err);

void _MD_unix_map_flock_error(int err);

void _MD_unix_map_lockf_error(int err);

#ifdef AIX
void _MD_aix_map_sendfile_error(int err);
#endif /* AIX */

#ifdef HPUX11
void _MD_hpux_map_sendfile_error(int err);
#endif /* HPUX11 */

#ifdef SOLARIS
void _MD_solaris_map_sendfile_error(int err);
#endif /* SOLARIS */

#ifdef LINUX
void _MD_linux_map_sendfile_error(int err);
#endif /* LINUX */

#ifdef __cplusplus
}
#endif
