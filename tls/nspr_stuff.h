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

#ifdef __cplusplus
}
#endif
