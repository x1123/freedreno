/*
 * Copyright (c) 2012 Rob Clark <robdclark@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef KGSL_H_
#define KGSL_H_

#include <stdint.h>

#include <freedreno_drmif.h>
#include <freedreno_ringbuffer.h>

#include "a2xx_reg.h"
#include "freedreno_a2xx_reg.h"
#include "adreno_pm4types.h"

#include "util.h"

#define LOG_DWORDS 0


static inline void
OUT_RING(struct fd_ringbuffer *ring, uint32_t data)
{
	if (LOG_DWORDS) {
		DEBUG_MSG("ring[%p]: OUT_RING   %04x:  %08x\n", ring,
				(uint32_t)(ring->cur - ring->last_start), data);
	}
	*(ring->cur++) = data;
}

static inline void
OUT_RELOC(struct fd_ringbuffer *ring, struct fd_bo *bo,
		uint32_t offset, uint32_t or)
{
	if (LOG_DWORDS) {
		DEBUG_MSG("ring[%p]: OUT_RELOC  %04x:  %p+%u", ring,
				(uint32_t)(ring->cur - ring->last_start), bo, offset);
	}
	fd_ringbuffer_emit_reloc(ring, bo, offset, or);
}

static inline void BEGIN_RING(struct fd_ringbuffer *ring, uint32_t ndwords)
{
	if ((ring->cur + ndwords) >= ring->end) {
		/* this probably won't really work if we have multiple tiles..
		 * but it is ok for 2d..  we might need different behavior
		 * depending on 2d or 3d pipe.
		 */
		WARN_MSG("uh oh..");
	}
}

static inline void
OUT_PKT0(struct fd_ringbuffer *ring, uint16_t regindx, uint16_t cnt)
{
	BEGIN_RING(ring, cnt+1);
	OUT_RING(ring, CP_TYPE0_PKT | ((cnt-1) << 16) | (regindx & 0x7FFF));
}

static inline void
OUT_PKT3(struct fd_ringbuffer *ring, uint8_t opcode, uint16_t cnt)
{
	BEGIN_RING(ring, cnt+1);
	OUT_RING(ring, CP_TYPE3_PKT | ((cnt-1) << 16) | ((opcode & 0xFF) << 8));
}

static inline void
OUT_IB(struct fd_ringbuffer *ring,
		struct fd_ringbuffer *dst_ring)
{
	OUT_PKT3(ring, CP_INDIRECT_BUFFER_PFD, 2);
	fd_ringbuffer_emit_reloc_ring(ring, dst_ring,
			(uint8_t *)dst_ring->last_start - (uint8_t *)dst_ring->start);
	OUT_RING(ring, dst_ring->cur - dst_ring->last_start);
}

/* convert float to dword */
static inline uint32_t f2d(float f)
{
	union {
		float f;
		uint32_t d;
	} u = {
		.f = f,
	};
	return u.d;
}

/* convert float to 12.4 fixed point */
static inline uint32_t f2d12_4(float f)
{
	return (uint32_t)(f * 8.0);
}

/* convert x,y to dword */
static inline uint32_t xy2d(uint16_t x, uint16_t y)
{
	return ((y & 0x3fff) << 16) | (x & 0x3fff);
}

#endif /* KGSL_H_ */
