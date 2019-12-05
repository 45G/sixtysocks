/* Copyright (c) NSPR Contributors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <errno.h>
#include <nspr.h>

#ifdef __cplusplus
extern "C"
{
#endif

inline void _MD_unix_map_connect_error(int err);

inline void _MD_unix_map_getsockname_error(int err);

inline void _MD_unix_map_getpeername_error(int err);

inline void _MD_unix_map_default_error(int err);

inline void _MD_unix_map_closedir_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case EINVAL:
		prError = PR_BAD_DESCRIPTOR_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_readdir_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case 0:
	case ENOENT:
		prError = PR_NO_MORE_FILES_ERROR;
		break;
#ifdef EOVERFLOW
	case EOVERFLOW:
		prError = PR_IO_ERROR;
		break;
#endif
	case EINVAL:
		prError = PR_IO_ERROR;
		break;
	case ENXIO:
		prError = PR_IO_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_unlink_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case EPERM:
		prError = PR_IS_DIRECTORY_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_stat_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case ETIMEDOUT:
		prError = PR_REMOTE_FILE_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_fstat_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case ETIMEDOUT:
		prError = PR_REMOTE_FILE_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_rename_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case EEXIST:
		prError = PR_DIRECTORY_NOT_EMPTY_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_access_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case ETIMEDOUT:
		prError = PR_REMOTE_FILE_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_mkdir_error(int err)
{
	_MD_unix_map_default_error(err);
}

inline void _MD_unix_map_rmdir_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	/*
	 * On AIX 4.3, ENOTEMPTY is defined as EEXIST.
	 */
#if ENOTEMPTY != EEXIST
	case ENOTEMPTY:
		prError = PR_DIRECTORY_NOT_EMPTY_ERROR;
		break;
#endif
	case EEXIST:
		prError = PR_DIRECTORY_NOT_EMPTY_ERROR;
		break;
	case EINVAL:
		prError = PR_DIRECTORY_NOT_EMPTY_ERROR;
		break;
	case ETIMEDOUT:
		prError = PR_REMOTE_FILE_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_read_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case EINVAL:
		prError = PR_INVALID_METHOD_ERROR;
		break;
	case ENXIO:
		prError = PR_INVALID_ARGUMENT_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_write_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case EINVAL:
		prError = PR_INVALID_METHOD_ERROR;
		break;
	case ENXIO:
		prError = PR_INVALID_METHOD_ERROR;
		break;
	case ETIMEDOUT:
		prError = PR_REMOTE_FILE_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_lseek_error(int err)
{
	_MD_unix_map_default_error(err);
}

inline void _MD_unix_map_fsync_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case ETIMEDOUT:
		prError = PR_REMOTE_FILE_ERROR;
		break;
	case EINVAL:
		prError = PR_INVALID_METHOD_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_close_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case ETIMEDOUT:
		prError = PR_REMOTE_FILE_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_socket_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case ENOMEM:
		prError = PR_INSUFFICIENT_RESOURCES_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_socketavailable_error(int err)
{
	PR_SetError(PR_BAD_DESCRIPTOR_ERROR, err);
}

inline void _MD_unix_map_default_error(int err);

inline void _MD_unix_map_default_error(int err);

inline void _MD_unix_map_accept_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case ENODEV:
		prError = PR_NOT_TCP_SOCKET_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_connect_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
#if defined(UNIXWARE)
	/*
	 * On some platforms, if we connect to a port on the local host
	 * (the loopback address) that no process is listening on, we get
	 * EIO instead of ECONNREFUSED.
	 */
	case EIO:
		prError = PR_CONNECT_REFUSED_ERROR;
		break;
#endif
	case ENXIO:
		prError = PR_IO_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_bind_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case EINVAL:
		prError = PR_SOCKET_ADDRESS_IS_BOUND_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_socketpair_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case ENOMEM:
		prError = PR_INSUFFICIENT_RESOURCES_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_getsockname_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case ENOMEM:
		prError = PR_INSUFFICIENT_RESOURCES_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_getpeername_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case ENOMEM:
		prError = PR_INSUFFICIENT_RESOURCES_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_getsockopt_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case EINVAL:
		prError = PR_BUFFER_OVERFLOW_ERROR;
		break;
	case ENOMEM:
		prError = PR_INSUFFICIENT_RESOURCES_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_setsockopt_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case EINVAL:
		prError = PR_BUFFER_OVERFLOW_ERROR;
		break;
	case ENOMEM:
		prError = PR_INSUFFICIENT_RESOURCES_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_open_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case EAGAIN:
		prError = PR_INSUFFICIENT_RESOURCES_ERROR;
		break;
	case EBUSY:
		prError = PR_IO_ERROR;
		break;
	case ENODEV:
		prError = PR_FILE_NOT_FOUND_ERROR;
		break;
	case ENOMEM:
		prError = PR_INSUFFICIENT_RESOURCES_ERROR;
		break;
#ifdef EOVERFLOW
	case EOVERFLOW:
		prError = PR_FILE_TOO_BIG_ERROR;
		break;
#endif
	case ETIMEDOUT:
		prError = PR_REMOTE_FILE_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_mmap_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case EAGAIN:
		prError = PR_INSUFFICIENT_RESOURCES_ERROR;
		break;
	case EMFILE:
		prError = PR_INSUFFICIENT_RESOURCES_ERROR;
		break;
	case ENODEV:
		prError = PR_OPERATION_NOT_SUPPORTED_ERROR;
		break;
	case ENXIO:
		prError = PR_INVALID_ARGUMENT_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_flock_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case EINVAL:
		prError = PR_BAD_DESCRIPTOR_ERROR;
		break;
	case EWOULDBLOCK:
		prError = PR_FILE_IS_LOCKED_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_lockf_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case EACCES:
		prError = PR_FILE_IS_LOCKED_ERROR;
		break;
	case EDEADLK:
		prError = PR_INSUFFICIENT_RESOURCES_ERROR;
		break;
	default:
		_MD_unix_map_default_error(err);
		return;
	}
	PR_SetError(prError, err);
}

#ifdef __cplusplus
}
#endif
