#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <assert.h>
#include <memory.h>

#define MAX_MALLOC_SIZE (16 * 1024 * 1024)              /* arbitrary - change for your application */

#define MEM_ALLOC_PATTERN       0xe7
#define MEM_FREE_PATTERN        0xd7

#define M_HEADER_MAGIC 0xf10c367a
typedef struct m_header {
        unsigned int m_magic;
        unsigned int m_size;
} m_header_t;
#define M_TRAILER_MAGIC 0xf10c3a29
typedef struct m_trailer {
        unsigned int m_magic;
        unsigned int m_size;
} m_trailer_t;

static inline size_t roundup_size(size_t size)
{
        return (size + sizeof (long)) & (~(sizeof (long) - 1));
}

static inline size_t calculate_actual_size(size_t sz)
{
        return sizeof (m_header_t) + roundup_size(sz) + sizeof (m_trailer_t);
}

static void *header_to_buf(m_header_t *hdr)
{
        return (void *)hdr + sizeof (m_header_t);
}

static void *buf_to_header(void *p)
{
        return (m_header_t *)(p - sizeof (m_header_t));
}

static void *header_to_trailer(m_header_t *hdr)
{
        return (m_trailer_t *)((void *)hdr + sizeof (m_header_t) + roundup_size(hdr->m_size));
}

void *memalloc(size_t size)
{
        size_t actual_size;
        m_header_t *hdr;
        m_trailer_t *tlr;
        void *data;
        assert(size <= MAX_MALLOC_SIZE);
        actual_size = calculate_actual_size(roundup_size(size));
        data = malloc(actual_size);
        if (data == NULL)
                return NULL;
        memset(data, MEM_ALLOC_PATTERN, actual_size);
        hdr = (m_header_t *)data;
        hdr->m_magic = M_HEADER_MAGIC;
        hdr->m_size = size;
        tlr = header_to_trailer(hdr);
        tlr->m_magic = M_TRAILER_MAGIC;
        tlr->m_size = size;
        return header_to_buf(hdr);
}

void memfree(void *data)
{
        m_header_t *hdr;
        m_trailer_t *tlr;
        if (data == NULL)
                return;
        hdr = buf_to_header(data);
        assert(hdr->m_magic == M_HEADER_MAGIC);
        assert(hdr->m_size <= MAX_MALLOC_SIZE);
        tlr = header_to_trailer(hdr);
        assert(tlr->m_magic == M_TRAILER_MAGIC);
        assert(tlr->m_size <= MAX_MALLOC_SIZE);
        assert(hdr->m_size == tlr->m_size);
        memset(hdr, MEM_FREE_PATTERN, calculate_actual_size(hdr->m_size));
        free(hdr);
}
