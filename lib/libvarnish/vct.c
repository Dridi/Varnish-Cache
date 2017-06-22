/*-
 * Copyright (c) 2006-2010 Varnish Software AS
 * All rights reserved.
 *
 * Author: Poul-Henning Kamp <phk@phk.freebsd.dk>
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
 * ctype(3) like functions, according to RFC2616
 */

#include "config.h"

#include <stdlib.h>
#include <stdint.h>

#include "vas.h"
#include "vct.h"

/* NB: VCT always operate in ASCII, don't replace 0x0d with \r etc. */

#define VCT_UPALPHA	VCT_ALPHA
#define VCT_LOALPHA	VCT_ALPHA

const uint16_t vct_typtab[256] = {
	[0x00]	=	VCT_CTL,
	[0x01]	=	VCT_CTL,
	[0x02]	=	VCT_CTL,
	[0x03]	=	VCT_CTL,
	[0x04]	=	VCT_CTL,
	[0x05]	=	VCT_CTL,
	[0x06]	=	VCT_CTL,
	[0x07]	=	VCT_CTL,
	[0x08]	=	VCT_CTL,
	[0x09]	=	VCT_CTL | VCT_SP | VCT_SEPARATOR,
	[0x0a]	=	VCT_CTL | VCT_CRLF,
	[0x0b]	=	VCT_CTL,
	[0x0c]	=	VCT_CTL,
	[0x0d]	=	VCT_CTL | VCT_CRLF,
	[0x0e]	=	VCT_CTL,
	[0x0f]	=	VCT_CTL,
	[0x10]	=	VCT_CTL,
	[0x11]	=	VCT_CTL,
	[0x12]	=	VCT_CTL,
	[0x13]	=	VCT_CTL,
	[0x14]	=	VCT_CTL,
	[0x15]	=	VCT_CTL,
	[0x16]	=	VCT_CTL,
	[0x17]	=	VCT_CTL,
	[0x18]	=	VCT_CTL,
	[0x19]	=	VCT_CTL,
	[0x1a]	=	VCT_CTL,
	[0x1b]	=	VCT_CTL,
	[0x1c]	=	VCT_CTL,
	[0x1d]	=	VCT_CTL,
	[0x1e]	=	VCT_CTL,
	[0x1f]	=	VCT_CTL,
	[0x20]  =	VCT_SP | VCT_SEPARATOR,
	[0x21]  =	VCT_TCHAR,
	[0x22]  =	VCT_SEPARATOR,
	[0x23]  =	VCT_TCHAR,
	[0x24]  =	VCT_TCHAR,
	[0x25]  =	VCT_TCHAR,
	[0x26]  =	VCT_TCHAR,
	[0x27]  =	VCT_TCHAR,
	[0x28]  =	VCT_SEPARATOR,
	[0x29]  =	VCT_SEPARATOR,
	[0x2a]  =	VCT_TCHAR,
	[0x2b]  =	VCT_TCHAR,
	[0x2c]  =	VCT_SEPARATOR,
	[0x2d]	=	VCT_XMLNAME | VCT_TCHAR,
	[0x2e]	=	VCT_XMLNAME | VCT_TCHAR,
	[0x2f]  =	VCT_SEPARATOR,
	[0x30]	=	VCT_DIGIT | VCT_HEX | VCT_XMLNAME,
	[0x31]	=	VCT_DIGIT | VCT_HEX | VCT_XMLNAME,
	[0x32]	=	VCT_DIGIT | VCT_HEX | VCT_XMLNAME,
	[0x33]	=	VCT_DIGIT | VCT_HEX | VCT_XMLNAME,
	[0x34]	=	VCT_DIGIT | VCT_HEX | VCT_XMLNAME,
	[0x35]	=	VCT_DIGIT | VCT_HEX | VCT_XMLNAME,
	[0x36]	=	VCT_DIGIT | VCT_HEX | VCT_XMLNAME,
	[0x37]	=	VCT_DIGIT | VCT_HEX | VCT_XMLNAME,
	[0x38]	=	VCT_DIGIT | VCT_HEX | VCT_XMLNAME,
	[0x39]	=	VCT_DIGIT | VCT_HEX | VCT_XMLNAME,
	[0x3a]  =	VCT_SEPARATOR | VCT_XMLNAMESTART,
	[0x3b]  =	VCT_SEPARATOR,
	[0x3c]  =	VCT_SEPARATOR,
	[0x3d]  =	VCT_SEPARATOR,
	[0x3e]  =	VCT_SEPARATOR,
	[0x3f]  =	VCT_SEPARATOR,
	[0x40]  =	VCT_SEPARATOR,
	[0x41]	=	VCT_UPALPHA | VCT_HEX | VCT_XMLNAMESTART,
	[0x42]	=	VCT_UPALPHA | VCT_HEX | VCT_XMLNAMESTART,
	[0x43]	=	VCT_UPALPHA | VCT_HEX | VCT_XMLNAMESTART,
	[0x44]	=	VCT_UPALPHA | VCT_HEX | VCT_XMLNAMESTART,
	[0x45]	=	VCT_UPALPHA | VCT_HEX | VCT_XMLNAMESTART,
	[0x46]	=	VCT_UPALPHA | VCT_HEX | VCT_XMLNAMESTART,
	[0x47]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x48]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x49]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x4a]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x4b]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x4c]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x4d]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x4e]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x4f]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x50]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x51]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x52]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x53]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x54]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x55]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x56]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x57]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x58]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x59]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x5a]	=	VCT_UPALPHA | VCT_XMLNAMESTART,
	[0x5b]  =	VCT_SEPARATOR,
	[0x5c]  =	VCT_SEPARATOR,
	[0x5d]  =	VCT_SEPARATOR,
	[0x5e]  =	VCT_TCHAR,
	[0x5f]	=	VCT_XMLNAMESTART | VCT_TCHAR,
	[0x60]	=	VCT_TCHAR,
	[0x61]	=	VCT_LOALPHA | VCT_HEX | VCT_XMLNAMESTART,
	[0x62]	=	VCT_LOALPHA | VCT_HEX | VCT_XMLNAMESTART,
	[0x63]	=	VCT_LOALPHA | VCT_HEX | VCT_XMLNAMESTART,
	[0x64]	=	VCT_LOALPHA | VCT_HEX | VCT_XMLNAMESTART,
	[0x65]	=	VCT_LOALPHA | VCT_HEX | VCT_XMLNAMESTART,
	[0x66]	=	VCT_LOALPHA | VCT_HEX | VCT_XMLNAMESTART,
	[0x67]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x68]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x69]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x6a]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x6b]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x6c]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x6d]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x6e]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x6f]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x70]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x71]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x72]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x73]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x74]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x75]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x76]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x77]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x78]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x79]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x7a]	=	VCT_LOALPHA | VCT_XMLNAMESTART,
	[0x7b]  =	VCT_SEPARATOR,
	[0x7c]	=	VCT_TCHAR,
	[0x7d]  =	VCT_SEPARATOR,
	[0x7e]	=	VCT_TCHAR,
	[0x7f]	=	VCT_CTL,
	[0xb7]	=	VCT_XMLNAME,
	[0xc0]	=	VCT_XMLNAMESTART,
	[0xc1]	=	VCT_XMLNAMESTART,
	[0xc2]	=	VCT_XMLNAMESTART,
	[0xc3]	=	VCT_XMLNAMESTART,
	[0xc4]	=	VCT_XMLNAMESTART,
	[0xc5]	=	VCT_XMLNAMESTART,
	[0xc6]	=	VCT_XMLNAMESTART,
	[0xc7]	=	VCT_XMLNAMESTART,
	[0xc8]	=	VCT_XMLNAMESTART,
	[0xc9]	=	VCT_XMLNAMESTART,
	[0xca]	=	VCT_XMLNAMESTART,
	[0xcb]	=	VCT_XMLNAMESTART,
	[0xcc]	=	VCT_XMLNAMESTART,
	[0xcd]	=	VCT_XMLNAMESTART,
	[0xce]	=	VCT_XMLNAMESTART,
	[0xcf]	=	VCT_XMLNAMESTART,
	[0xd0]	=	VCT_XMLNAMESTART,
	[0xd1]	=	VCT_XMLNAMESTART,
	[0xd2]	=	VCT_XMLNAMESTART,
	[0xd3]	=	VCT_XMLNAMESTART,
	[0xd4]	=	VCT_XMLNAMESTART,
	[0xd5]	=	VCT_XMLNAMESTART,
	[0xd6]	=	VCT_XMLNAMESTART,
	[0xd8]	=	VCT_XMLNAMESTART,
	[0xd9]	=	VCT_XMLNAMESTART,
	[0xda]	=	VCT_XMLNAMESTART,
	[0xdb]	=	VCT_XMLNAMESTART,
	[0xdc]	=	VCT_XMLNAMESTART,
	[0xdd]	=	VCT_XMLNAMESTART,
	[0xde]	=	VCT_XMLNAMESTART,
	[0xdf]	=	VCT_XMLNAMESTART,
	[0xe0]	=	VCT_XMLNAMESTART,
	[0xe1]	=	VCT_XMLNAMESTART,
	[0xe2]	=	VCT_XMLNAMESTART,
	[0xe3]	=	VCT_XMLNAMESTART,
	[0xe4]	=	VCT_XMLNAMESTART,
	[0xe5]	=	VCT_XMLNAMESTART,
	[0xe6]	=	VCT_XMLNAMESTART,
	[0xe7]	=	VCT_XMLNAMESTART,
	[0xe8]	=	VCT_XMLNAMESTART,
	[0xe9]	=	VCT_XMLNAMESTART,
	[0xea]	=	VCT_XMLNAMESTART,
	[0xeb]	=	VCT_XMLNAMESTART,
	[0xec]	=	VCT_XMLNAMESTART,
	[0xed]	=	VCT_XMLNAMESTART,
	[0xee]	=	VCT_XMLNAMESTART,
	[0xef]	=	VCT_XMLNAMESTART,
	[0xf0]	=	VCT_XMLNAMESTART,
	[0xf1]	=	VCT_XMLNAMESTART,
	[0xf2]	=	VCT_XMLNAMESTART,
	[0xf3]	=	VCT_XMLNAMESTART,
	[0xf4]	=	VCT_XMLNAMESTART,
	[0xf5]	=	VCT_XMLNAMESTART,
	[0xf6]	=	VCT_XMLNAMESTART,
	[0xf8]	=	VCT_XMLNAMESTART,
	[0xf9]	=	VCT_XMLNAMESTART,
	[0xfa]	=	VCT_XMLNAMESTART,
	[0xfb]	=	VCT_XMLNAMESTART,
	[0xfc]	=	VCT_XMLNAMESTART,
	[0xfd]	=	VCT_XMLNAMESTART,
	[0xfe]	=	VCT_XMLNAMESTART,
	[0xff]	=	VCT_XMLNAMESTART,
};

const char *
VCT_bad_name(const char *b, const char *e)
{

	AN(b);
	if (e != NULL)
		assert(b < e);

	if (!vct_isalpha(*b))
		return (b);

	while (++b != e && *b != '\0')
		if (!vct_isalpha(*b) &&
		    !vct_isdigit(*b) &&
		    *b != '_' &&
		    *b != '-')
			return (b);

	if (*b == '\0' && e != NULL)
		return (b);

	return (NULL);
}
