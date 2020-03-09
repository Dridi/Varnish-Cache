/*-
 * Copyright (c) 2010-2011 Varnish Software AS
 * All rights reserved.
 *
 * Author: Sanjoy Das <sanjoy@playingwithpointers.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * XXX: It might make sense to use just a single global list of all files
 * XXX: and use the call-private pointer to point to the file instance on
 * XXX: that list.
 * XXX: Duplicates in the global list can be avoided by examining the
 * XXX: dev+inode fields of the stat structure.
 * XXX: Individual files would need to be refcounted, so they can be
 * XXX: deleted when no VCL's reference them.
 *
 * XXX: We should periodically stat(2) the filename and check if the
 * XXX: underlying file has been updated.
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "cache/cache.h"

#include "vfil.h"

#include "vcc_if.h"

struct frfile {
	unsigned			magic;
#define CACHED_FILE_MAGIC 0xa8e9d87a
	char				*file_name;
	char				*contents;
	size_t				size;
	int				refcount;
	VTAILQ_ENTRY(frfile)		list;
};

static VTAILQ_HEAD(, frfile)	frlist = VTAILQ_HEAD_INITIALIZER(frlist);
static pthread_mutex_t		frmtx = PTHREAD_MUTEX_INITIALIZER;

static void
unref_frfile(struct frfile **frfp)
{
	struct frfile *frf;

	AN(frfp);
	frf = *frfp;
	if (frf == NULL)
		return;
	CHECK_OBJ(frf, CACHED_FILE_MAGIC);
	assert(frf->refcount > 0);
	if (--frf->refcount > 0)
		*frfp = NULL;
	else
		VTAILQ_REMOVE(&frlist, frf, list);
}

static void
destroy_frfile(struct frfile **frfp)
{
	struct frfile *frf;

	TAKE_OBJ_NOTNULL(frf, frfp, CACHED_FILE_MAGIC);
	free(frf->contents);
	free(frf->file_name);
	FREE_OBJ(frf);
}

static void
free_frfile(void *ptr)
{
	struct frfile *frf;

	CAST_OBJ_NOTNULL(frf, ptr, CACHED_FILE_MAGIC);

	AZ(pthread_mutex_lock(&frmtx));
	unref_frfile(&frf);
	AZ(pthread_mutex_unlock(&frmtx));
	if (frf != NULL)
		destroy_frfile(&frf);
}

static struct frfile *
find_frfile(struct frfile *old, VCL_STRING file_name)
{
	struct frfile *frf;
	char *s;
	ssize_t sz;

	CHECK_OBJ_ORNULL(old, CACHED_FILE_MAGIC);

	if (file_name == NULL)
		return (NULL);

	if (old != NULL && !strcmp(file_name, old->file_name))
		return (old);

	AZ(pthread_mutex_lock(&frmtx));
	unref_frfile(&old);
	VTAILQ_FOREACH(frf, &frlist, list) {
		if (!strcmp(file_name, frf->file_name)) {
			frf->refcount++;
			break;
		}
	}
	AZ(pthread_mutex_unlock(&frmtx));
	if (old != NULL)
		destroy_frfile(&old);
	if (frf != NULL)
		return (frf);

	s = VFIL_readfile(NULL, file_name, &sz);
	if (s != NULL) {
		assert(sz > 0);
		ALLOC_OBJ(frf, CACHED_FILE_MAGIC);
		AN(frf);
		REPLACE(frf->file_name, file_name);
		frf->refcount = 1;
		frf->contents = s;
		frf->size = (size_t)sz;
		AZ(pthread_mutex_lock(&frmtx));
		VTAILQ_INSERT_HEAD(&frlist, frf, list);
		AZ(pthread_mutex_unlock(&frmtx));
	}
	return (frf);
}

VCL_STRING v_matchproto_(td_std_fileread)
vmod_fileread(VRT_CTX, struct vmod_priv *priv,
    VCL_STRING file_name)
{
	struct frfile *frf;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	AN(priv);

	frf = find_frfile(priv->priv, file_name);
	priv->priv = frf;
	priv->free = free_frfile;
	if (frf == NULL)
		return (NULL);
	return (frf->contents);
}
