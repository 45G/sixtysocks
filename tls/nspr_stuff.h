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

void mapDefaultError(int err);

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
		mapDefaultError(err);
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
		mapDefaultError(err);
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
		mapDefaultError(err);
		return;
	}
	PR_SetError(prError, err);
}

inline void _MD_unix_map_socketavailable_error(int err)
{
	PR_SetError(PR_BAD_DESCRIPTOR_ERROR, err);
}

inline void _MD_unix_map_accept_error(int err)
{
	PRErrorCode prError;
	
	switch (err) {
	case ENODEV:
		prError = PR_NOT_TCP_SOCKET_ERROR;
		break;
	default:
		mapDefaultError(err);
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
		mapDefaultError(err);
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
		mapDefaultError(err);
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
		mapDefaultError(err);
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
		mapDefaultError(err);
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
		mapDefaultError(err);
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
		mapDefaultError(err);
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
		mapDefaultError(err);
		return;
	}
	PR_SetError(prError, err);
}

#ifdef __cplusplus
}
#endif
