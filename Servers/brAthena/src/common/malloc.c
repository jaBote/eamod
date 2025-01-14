/****************************************************************************!
*                _           _   _   _                                       *    
*               | |__  _ __ / \ | |_| |__   ___ _ __   __ _                  *  
*               | '_ \| '__/ _ \| __| '_ \ / _ \ '_ \ / _` |                 *   
*               | |_) | | / ___ \ |_| | | |  __/ | | | (_| |                 *    
*               |_.__/|_|/_/   \_\__|_| |_|\___|_| |_|\__,_|                 *    
*                                                                            *
*                                                                            *
* \file src/common/malloc.c                                                  *
* Descri��o Prim�ria.                                                        *
* Descri��o mais elaborada sobre o arquivo.                                  *
* \author brAthena, rAthena                                                  *
* \date ?                                                                    *
* \todo ?                                                                    *  
*****************************************************************************/

#include "../common/malloc.h"
#include "../common/core.h"
#include "../common/showmsg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

////////////// Memory Libraries //////////////////

#if defined(MEMWATCH)

#   include <string.h>
#   include "memwatch.h"
#   define MALLOC(n,file,line,func) mwMalloc((n),(file),(line))
#   define CALLOC(m,n,file,line,func)   mwCalloc((m),(n),(file),(line))
#   define REALLOC(p,n,file,line,func)  mwRealloc((p),(n),(file),(line))
#   define STRDUP(p,file,line,func) mwStrdup((p),(file),(line))
#   define FREE(p,file,line,func)       mwFree((p),(file),(line))
#   define MEMORY_USAGE()   0
#   define MEMORY_VERIFY(ptr)   mwIsSafeAddr(ptr, 1)
#   define MEMORY_CHECK() CHECK()

#elif defined(DMALLOC)

#   include <string.h>
#   include <stdlib.h>
#   include "dmalloc.h"
#   define MALLOC(n,file,line,func) dmalloc_malloc((file),(line),(n),DMALLOC_FUNC_MALLOC,0,0)
#   define CALLOC(m,n,file,line,func)   dmalloc_malloc((file),(line),(m)*(n),DMALLOC_FUNC_CALLOC,0,0)
#   define REALLOC(p,n,file,line,func)  dmalloc_realloc((file),(line),(p),(n),DMALLOC_FUNC_REALLOC,0)
#   define STRDUP(p,file,line,func) strdup(p)
#   define FREE(p,file,line,func)       free(p)
#   define MEMORY_USAGE()   dmalloc_memory_allocated()
#   define MEMORY_VERIFY(ptr)   (dmalloc_verify(ptr) == DMALLOC_VERIFY_NOERROR)
#   define MEMORY_CHECK()   dmalloc_log_stats(); dmalloc_log_unfreed()

#elif defined(GCOLLECT)

#   include "gc.h"
#   ifdef GC_ADD_CALLER
#       define RETURN_ADDR 0,
#   else
#       define RETURN_ADDR
#   endif
#   define MALLOC(n,file,line,func) GC_debug_malloc((n), RETURN_ADDR (file),(line))
#   define CALLOC(m,n,file,line,func)   GC_debug_malloc((m)*(n), RETURN_ADDR (file),(line))
#   define REALLOC(p,n,file,line,func)  GC_debug_realloc((p),(n), RETURN_ADDR (file),(line))
#   define STRDUP(p,file,line,func) GC_debug_strdup((p), RETURN_ADDR (file),(line))
#   define FREE(p,file,line,func)       GC_debug_free(p)
#   define MEMORY_USAGE()   GC_get_heap_size()
#   define MEMORY_VERIFY(ptr)   (GC_base(ptr) != NULL)
#   define MEMORY_CHECK()   GC_gcollect()

#else

#   define MALLOC(n,file,line,func) malloc(n)
#   define CALLOC(m,n,file,line,func)   calloc((m),(n))
#   define REALLOC(p,n,file,line,func)  realloc((p),(n))
#   define STRDUP(p,file,line,func) strdup(p)
#   define FREE(p,file,line,func)       free(p)
#   define MEMORY_USAGE()   0
#   define MEMORY_VERIFY(ptr)   true
#   define MEMORY_CHECK()

#endif

void *aMalloc_(size_t size, const char *file, int line, const char *func)
{
	void *ret = MALLOC(size, file, line, func);
	// ShowMessage("%s:%d: in func %s: aMalloc %d\n",file,line,func,size);
	if(ret == NULL) {
		ShowFatalError(read_message("Source.common.amalloc"),file,line,func);
		exit(EXIT_FAILURE);
	}

	return ret;
}
void *aCalloc_(size_t num, size_t size, const char *file, int line, const char *func)
{
	void *ret = CALLOC(num, size, file, line, func);
	// ShowMessage("%s:%d: in func %s: aCalloc %d %d\n",file,line,func,num,size);
	if(ret == NULL) {
		ShowFatalError(read_message("Source.common.acalloc"), file, line, func);
		exit(EXIT_FAILURE);
	}
	return ret;
}
void *aRealloc_(void *p, size_t size, const char *file, int line, const char *func)
{
	void *ret = REALLOC(p, size, file, line, func);
	// ShowMessage("%s:%d: in func %s: aRealloc %p %d\n",file,line,func,p,size);
	if(ret == NULL) {
		ShowFatalError(read_message("Source.common.arealloc"),file,line,func);
		exit(EXIT_FAILURE);
	}
	return ret;
}
char *aStrdup_(const char *p, const char *file, int line, const char *func)
{
	char *ret = STRDUP(p, file, line, func);
	// ShowMessage("%s:%d: in func %s: aStrdup %p\n",file,line,func,p);
	if(ret == NULL) {
		ShowFatalError(read_message("Source.common.astrdup"), file, line, func);
		exit(EXIT_FAILURE);
	}
	return ret;
}
void aFree_(void *p, const char *file, int line, const char *func)
{
	// ShowMessage("%s:%d: in func %s: aFree %p\n",file,line,func,p);
	if(p)
		FREE(p, file, line, func);

	p = NULL;
}


#ifdef USE_MEMMGR

#if defined(DEBUG)
#define DEBUG_MEMMGR
#endif

/* USE_MEMMGR */

/*
 * �������}�l�[�W��
 *     malloc , free �̏����������I�ɏo�����悤�ɂ������́B
 *     ���G�ȏ������s��Ă����̂ŁA�኱�d���Ȃ邩�����܂����B
 *
 * �f�[�^�\���Ȃǁi�������ł����܂���^^; �j
 *     �E�������𕡐��́u�u���b�N�v�ɕ����āA�����Ƀu���b�N�𕡐��́u���j�b�g�v
 *       �ɕ����Ă��܂��B���j�b�g�̃T�C�Y�́A�P�u���b�N�̗e�ʂ𕡐��ɋϓ��z��
 *       �������̂ł��B���Ƃ��΁A�P���j�b�g32KB�̏ꍇ�A�u���b�N�P��32Byte�̃�
 *       �j�b�g���A1024�W�܂�ďo���Ă������A64Byte�̃��j�b�g�� 512�W�܂��
 *       �o���Ă����肵�܂��B�ipadding,unit_head ����j
 *
 *     �E�u���b�N���m�̓����N���X�g(block_prev,block_next) �łȂ����A�����T�C
 *       �Y��u���b�N���m�����N���X�g(hash_prev,hash_nect) �ł�
 *       ����Ă��܂��B�����ɂ����A�s�v�ƂȂ���������̍ė��p�������I�ɍs���܂��B
 */

/* �u���b�N�̃A���C�����g */
#define BLOCK_ALIGNMENT1    16
#define BLOCK_ALIGNMENT2    64

/* �u���b�N�ɓ����f�[�^�� */
#define BLOCK_DATA_COUNT1   128
#define BLOCK_DATA_COUNT2   608

/* �u���b�N�̑傫��: 16*128 + 64*576 = 40KB */
#define BLOCK_DATA_SIZE1    ( BLOCK_ALIGNMENT1 * BLOCK_DATA_COUNT1 )
#define BLOCK_DATA_SIZE2    ( BLOCK_ALIGNMENT2 * BLOCK_DATA_COUNT2 )
#define BLOCK_DATA_SIZE     ( BLOCK_DATA_SIZE1 + BLOCK_DATA_SIZE2 )

/* ���x�Ɋm�ۂ����u���b�N�̐��B */
#define BLOCK_ALLOC     104

/* �u���b�N */
struct block {
	struct block *block_next;       /* ���Ɋm�ۂ����̈� */
	struct block *unfill_prev;      /* ���̖��܂�Ă��Ȃ��̈� */
	struct block *unfill_next;      /* ���̖��܂�Ă��Ȃ��̈� */
	unsigned short unit_size;       /* ���j�b�g�̑傫�� */
	unsigned short unit_hash;       /* ���j�b�g�̃n�b�V�� */
	unsigned short unit_count;      /* ���j�b�g�̌� */
	unsigned short unit_used;       /* �g�p���j�b�g�� */
	unsigned short unit_unfill;     /* ���g�p���j�b�g�̏ꏊ */
	unsigned short unit_maxused;    /* �g�p���j�b�g�̍ő��l */
	char   data[ BLOCK_DATA_SIZE ];
};

struct unit_head {
	struct block   *block;
	const  char   *file;
	unsigned short line;
	unsigned short size;
	long           checksum;
};

static struct block *hash_unfill[BLOCK_DATA_COUNT1 + BLOCK_DATA_COUNT2 + 1];
static struct block *block_first, *block_last, block_head;

/* ���������g���񂹂Ȃ��̈��p�̃f�[�^ */
struct unit_head_large {
	size_t                  size;
	struct unit_head_large *prev;
	struct unit_head_large *next;
	struct unit_head        unit_head;
};

static struct unit_head_large *unit_head_large_first = NULL;

static struct block *block_malloc(unsigned short hash);
static void          block_free(struct block *p);
static size_t        memmgr_usage_bytes;

#define block2unit(p, n) ((struct unit_head*)(&(p)->data[ p->unit_size * (n) ]))
#define memmgr_assert(v) do { if(!(v)) { ShowError(read_message("Source.common.memmgr_assert")); } } while(0)

static unsigned short size2hash(size_t size)
{
	if(size <= BLOCK_DATA_SIZE1) {
		return (unsigned short)(size + BLOCK_ALIGNMENT1 - 1) / BLOCK_ALIGNMENT1;
	} else if(size <= BLOCK_DATA_SIZE) {
		return (unsigned short)(size - BLOCK_DATA_SIZE1 + BLOCK_ALIGNMENT2 - 1) / BLOCK_ALIGNMENT2
		       + BLOCK_DATA_COUNT1;
	} else {
		return 0xffff;  // �u���b�N���𒴂����ꍇ�� hash �ɂ��Ȃ�
	}
}

static size_t hash2size(unsigned short hash)
{
	if(hash <= BLOCK_DATA_COUNT1) {
		return hash * BLOCK_ALIGNMENT1;
	} else {
		return (hash - BLOCK_DATA_COUNT1) * BLOCK_ALIGNMENT2 + BLOCK_DATA_SIZE1;
	}
}

void *_mmalloc(size_t size, const char *file, int line, const char *func)
{
	struct block *block;
	short size_hash = size2hash(size);
	struct unit_head *head;

	if(((long) size) < 0) {
		ShowError("_mmalloc: %d\n", size);
		return NULL;
	}

	if(size == 0) {
		return NULL;
	}
	memmgr_usage_bytes += size;

	/* �u���b�N���𒴂����̈��̊m�ۂɂ́Amalloc() ���p���� */
	/* ���̍ہAunit_head.block �� NULL ���������ċ��ʂ��� */
	if(hash2size(size_hash) > BLOCK_DATA_SIZE - sizeof(struct unit_head)) {
		struct unit_head_large *p = (struct unit_head_large *)MALLOC(sizeof(struct unit_head_large)+size,file,line,func);
		if(p != NULL) {
			p->size            = size;
			p->unit_head.block = NULL;
			p->unit_head.size  = 0;
			p->unit_head.file  = file;
			p->unit_head.line  = line;
			p->prev = NULL;
			if(unit_head_large_first == NULL)
				p->next = NULL;
			else {
				unit_head_large_first->prev = p;
				p->next = unit_head_large_first;
			}
			unit_head_large_first = p;
			*(long *)((char *)p + sizeof(struct unit_head_large) - sizeof(long) + size) = 0xdeadbeaf;
			return (char *)p + sizeof(struct unit_head_large) - sizeof(long);
		} else {
			ShowFatalError(read_message("Source.common.memmgr_alloc"), sizeof(struct unit_head_large), size, file, line);
			exit(EXIT_FAILURE);
		}
	}

	/* �����T�C�Y�̃u���b�N���m�ۂ����Ă��Ȃ����A�V���Ɋm�ۂ��� */
	if(hash_unfill[size_hash]) {
		block = hash_unfill[size_hash];
	} else {
		block = block_malloc(size_hash);
	}

	if(block->unit_unfill == 0xFFFF) {
		// free�ςݗ̈悪�c��Ă��Ȃ�
		memmgr_assert(block->unit_used <  block->unit_count);
		memmgr_assert(block->unit_used == block->unit_maxused);
		head = block2unit(block, block->unit_maxused);
		block->unit_used++;
		block->unit_maxused++;
	} else {
		head = block2unit(block, block->unit_unfill);
		block->unit_unfill = head->size;
		block->unit_used++;
	}

	if(block->unit_unfill == 0xFFFF && block->unit_maxused >= block->unit_count) {
		// ���j�b�g���g���ʂ������̂ŁAunfill���X�g�����폜
		if(block->unfill_prev == &block_head) {
			hash_unfill[ size_hash ] = block->unfill_next;
		} else {
			block->unfill_prev->unfill_next = block->unfill_next;
		}
		if(block->unfill_next) {
			block->unfill_next->unfill_prev = block->unfill_prev;
		}
		block->unfill_prev = NULL;
	}

#ifdef DEBUG_MEMMGR
	{
		size_t i, sz = hash2size(size_hash);
		for(i=0; i<sz; i++) {
			if(((unsigned char *)head)[ sizeof(struct unit_head) - sizeof(long) + i] != 0xfd) {
				if(head->line != 0xfdfd) {
					ShowError(read_message("Source.common.debug_memmgr"), head->file,head->line);
				} else {
					ShowError(read_message("Source.common.debug_memmgr2"));
				}
				break;
			}
		}
		memset((char *)head + sizeof(struct unit_head) - sizeof(long), 0xcd, sz);
	}
#endif

	head->block = block;
	head->file  = file;
	head->line  = line;
	head->size  = (unsigned short)size;
	*(long *)((char *)head + sizeof(struct unit_head) - sizeof(long) + size) = 0xdeadbeaf;
	return (char *)head + sizeof(struct unit_head) - sizeof(long);
}

void *_mcalloc(size_t num, size_t size, const char *file, int line, const char *func)
{
	void *p = _mmalloc(num * size,file,line,func);
	memset(p,0,num * size);
	return p;
}

void *_mrealloc(void *memblock, size_t size, const char *file, int line, const char *func)
{
	size_t old_size;
	if(memblock == NULL) {
		return _mmalloc(size,file,line,func);
	}

	old_size = ((struct unit_head *)((char *)memblock - sizeof(struct unit_head) + sizeof(long)))->size;
	if(old_size == 0) {
		old_size = ((struct unit_head_large *)((char *)memblock - sizeof(struct unit_head_large) + sizeof(long)))->size;
	}
	if(old_size > size) {
		// �T�C�Y�k�� -> ���̂܂ܕԂ��i�蔲���j
		return memblock;
	}  else {
		// �T�C�Y�g��
		void *p = _mmalloc(size,file,line,func);
		if(p != NULL) {
			memcpy(p,memblock,old_size);
		}
		_mfree(memblock,file,line,func);
		return p;
	}
}

char *_mstrdup(const char *p, const char *file, int line, const char *func)
{
	if(p == NULL) {
		return NULL;
	} else {
		size_t len = strlen(p);
		char *string  = (char *)_mmalloc(len + 1,file,line,func);
		memcpy(string,p,len+1);
		return string;
	}
}

void _mfree(void *ptr, const char *file, int line, const char *func)
{
	struct unit_head *head;

	if(ptr == NULL)
		return;

	head = (struct unit_head *)((char *)ptr - sizeof(struct unit_head) + sizeof(long));
	if(head->size == 0) {
		/* malloc() �Œ��Ɋm�ۂ��ꂽ�̈� */
		struct unit_head_large *head_large = (struct unit_head_large *)((char *)ptr - sizeof(struct unit_head_large) + sizeof(long));
		if(
		    *(long *)((char *)head_large + sizeof(struct unit_head_large) - sizeof(long) + head_large->size)
		    != 0xdeadbeaf) {
			ShowError(read_message("Source.common.mfree"), ptr, file, line);
		} else {
			head->size = 0xFFFF;
			if(head_large->prev) {
				head_large->prev->next = head_large->next;
			} else {
				unit_head_large_first  = head_large->next;
			}
			if(head_large->next) {
				head_large->next->prev = head_large->prev;
			}
			memmgr_usage_bytes -= head_large->size;
#ifdef DEBUG_MEMMGR
			// set freed memory to 0xfd
			memset(ptr, 0xfd, head_large->size);
#endif
			FREE(head_large,file,line,func);
		}
	} else {
		/* ���j�b�g���� */
		struct block *block = head->block;
		if((char *)head - (char *)block > sizeof(struct block)) {
			ShowError(read_message("Source.common.mfree2"), ptr, file, line);
		} else if(head->block == NULL) {
			ShowError(read_message("Source.common.mfree3"), ptr, file, line, func);
		} else if(*(long *)((char *)head + sizeof(struct unit_head) - sizeof(long) + head->size) != 0xdeadbeaf) {
			ShowError(read_message("Source.common.mfree"), ptr, file, line);
		} else {
			memmgr_usage_bytes -= head->size;
			head->block         = NULL;
#ifdef DEBUG_MEMMGR
			memset(ptr, 0xfd, block->unit_size - sizeof(struct unit_head) + sizeof(long));
			head->file = file;
			head->line = line;
#endif
			memmgr_assert(block->unit_used > 0);
			if(--block->unit_used == 0) {
				/* �u���b�N�̉��� */
				block_free(block);
			} else {
				if(block->unfill_prev == NULL) {
					// unfill ���X�g�ɒǉ�
					if(hash_unfill[ block->unit_hash ]) {
						hash_unfill[ block->unit_hash ]->unfill_prev = block;
					}
					block->unfill_prev = &block_head;
					block->unfill_next = hash_unfill[ block->unit_hash ];
					hash_unfill[ block->unit_hash ] = block;
				}
				head->size     = block->unit_unfill;
				block->unit_unfill = (unsigned short)(((uintptr_t)head - (uintptr_t)block->data) / block->unit_size);
			}
		}
	}
}

/* �u���b�N���m�ۂ��� */
static struct block *block_malloc(unsigned short hash) {
	int i;
	struct block *p;
	if(hash_unfill[0] != NULL) {
		/* �u���b�N�p�̗̈��͊m�ۍς� */
		p = hash_unfill[0];
		hash_unfill[0] = hash_unfill[0]->unfill_next;
	} else {
		/* �u���b�N�p�̗̈����V���Ɋm�ۂ��� */
		p = (struct block *)MALLOC(sizeof(struct block) * (BLOCK_ALLOC), __FILE__, __LINE__, __func__);
		if(p == NULL) {
			ShowFatalError(read_message("Source.common.block_alloc"));
			exit(EXIT_FAILURE);
		}

		if(block_first == NULL) {
			/* �����m�� */
			block_first = p;
		} else {
			block_last->block_next = p;
		}
		block_last = &p[BLOCK_ALLOC - 1];
		block_last->block_next = NULL;
		/* �u���b�N���A�������� */
		for(i=0; i<BLOCK_ALLOC; i++) {
			if(i != 0) {
				// p[0] �͂��ꂩ���g���̂Ń����N�ɂ͉���Ȃ�
				p[i].unfill_next = hash_unfill[0];
				hash_unfill[0]   = &p[i];
				p[i].unfill_prev = NULL;
				p[i].unit_used = 0;
			}
			if(i != BLOCK_ALLOC -1) {
				p[i].block_next = &p[i+1];
			}
		}
	}

	// unfill �ɒǉ�
	memmgr_assert(hash_unfill[ hash ] == NULL);
	hash_unfill[ hash ] = p;
	p->unfill_prev  = &block_head;
	p->unfill_next  = NULL;
	p->unit_size    = (unsigned short)(hash2size(hash) + sizeof(struct unit_head));
	p->unit_hash    = hash;
	p->unit_count   = BLOCK_DATA_SIZE / p->unit_size;
	p->unit_used    = 0;
	p->unit_unfill  = 0xFFFF;
	p->unit_maxused = 0;
#ifdef DEBUG_MEMMGR
	memset(p->data, 0xfd, sizeof(p->data));
#endif
	return p;
}

static void block_free(struct block *p)
{
	if(p->unfill_prev) {
		if(p->unfill_prev == &block_head) {
			hash_unfill[ p->unit_hash ] = p->unfill_next;
		} else {
			p->unfill_prev->unfill_next = p->unfill_next;
		}
		if(p->unfill_next) {
			p->unfill_next->unfill_prev = p->unfill_prev;
		}
		p->unfill_prev = NULL;
	}

	p->unfill_next = hash_unfill[0];
	hash_unfill[0] = p;
}

size_t memmgr_usage(void)
{
	return memmgr_usage_bytes / 1024;
}

#ifdef LOG_MEMMGR
static char memmer_logfile[128];
static FILE *log_fp;

static void memmgr_log(char *buf)
{
	if(!log_fp) {
		time_t raw;
		struct tm *t;

		log_fp = fopen(memmer_logfile,"at");
		if(!log_fp) log_fp = stdout;

		time(&raw);
		t = localtime(&raw);
		fprintf(log_fp, (read_message("Source.common.memmgr_log")),(t->tm_year+1900), (t->tm_mon+1), t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, get_svn_revision());
	}
	fprintf(log_fp, "%s", buf);
	return;
}
#endif /* LOG_MEMMGR */

/// Returns true if the memory location is active.
/// Active means it is allocated and points to a usable part.
///
/// @param ptr Pointer to the memory
/// @return true if the memory is active
bool memmgr_verify(void *ptr)
{
	struct block *block = block_first;
	struct unit_head_large *large = unit_head_large_first;

	if(ptr == NULL)
		return false;// never valid

	// search small blocks
	while(block) {
		if((char *)ptr >= (char *)block && (char *)ptr < ((char *)block) + sizeof(struct block)) {
			// found memory block
			if(block->unit_used && (char *)ptr >= block->data) {
				// memory block is being used and ptr points to a sub-unit
				size_t i = (size_t)((char *)ptr - block->data)/block->unit_size;
				struct unit_head *head = block2unit(block, i);
				if(i < block->unit_maxused && head->block != NULL) {
					// memory unit is allocated, check if ptr points to the usable part
					return ((char *)ptr >= ((char *)head) + sizeof(struct unit_head) - sizeof(long)
					        && (char *)ptr < ((char *)head) + sizeof(struct unit_head) - sizeof(long) + head->size);
				}
			}
			return false;
		}
		block = block->block_next;
	}

	// search large blocks
	while(large) {
		if((char *)ptr >= (char *)large && (char *)ptr < ((char *)large) + large->size) {
			// found memory block, check if ptr points to the usable part
			return ((char *)ptr >= ((char *)large) + sizeof(struct unit_head_large) - sizeof(long)
			        && (char *)ptr < ((char *)large) + sizeof(struct unit_head_large) - sizeof(long) + large->size);
		}
		large = large->next;
	}
	return false;
}

static void memmgr_final(void)
{
	struct block *block = block_first;
	struct unit_head_large *large = unit_head_large_first;

#ifdef LOG_MEMMGR
	int count = 0;
#endif /* LOG_MEMMGR */

	while(block) {
		if(block->unit_used) {
			int i;
			for(i = 0; i < block->unit_maxused; i++) {
				struct unit_head *head = block2unit(block, i);
				if(head->block != NULL) {
					char *ptr = (char *)head + sizeof(struct unit_head) - sizeof(long);
#ifdef LOG_MEMMGR
					char buf[1024];
					sprintf(buf,(read_message("Source.common.memmgr_log2")), ++count, head->file, head->line, (unsigned long)head->size, ptr);
					memmgr_log(buf);
#endif /* LOG_MEMMGR */
					// get block pointer and free it [celest]
					_mfree(ptr, ALC_MARK);
				}
			}
		}
		block = block->block_next;
	}

	while(large) {
		struct unit_head_large *large2;
#ifdef LOG_MEMMGR
		char buf[1024];
		sprintf(buf,(read_message("Source.common.memmgr_log2")), ++count,large->unit_head.file, large->unit_head.line, (unsigned long)large->size, &large->unit_head.checksum);
		memmgr_log(buf);
#endif /* LOG_MEMMGR */
		large2 = large->next;
		FREE(large,file,line,func);
		large = large2;
	}
#ifdef LOG_MEMMGR
	if(count == 0) {
		ShowInfo(read_message("Source.common.memmgr_log3"));
	} else {
		ShowWarning(read_message("Source.common.memmgr_log4"));
		fclose(log_fp);
	}
#endif /* LOG_MEMMGR */
}

static void memmgr_init(void)
{
#ifdef LOG_MEMMGR
	sprintf(memmer_logfile, "log/%s.leaks", SERVER_NAME);
	ShowStatus(read_message("Source.common.memmgr_init"), memmer_logfile);
	memset(hash_unfill, 0, sizeof(hash_unfill));
#endif /* LOG_MEMMGR */
}
#endif /* USE_MEMMGR */


/*======================================
 * Initialise
 *--------------------------------------
 */


/// Tests the memory for errors and memory leaks.
void malloc_memory_check(void)
{
	MEMORY_CHECK();
}


/// Returns true if a pointer is valid.
/// The check is best-effort, false positives are possible.
bool malloc_verify_ptr(void *ptr)
{
#ifdef USE_MEMMGR
	return memmgr_verify(ptr) && MEMORY_VERIFY(ptr);
#else
	return MEMORY_VERIFY(ptr);
#endif
}


size_t malloc_usage(void)
{
#ifdef USE_MEMMGR
	return memmgr_usage();
#else
	return MEMORY_USAGE();
#endif
}

void malloc_final(void)
{
#ifdef USE_MEMMGR
	memmgr_final();
#endif
	MEMORY_CHECK();
}

void malloc_init(void)
{
#if defined(DMALLOC) && defined(CYGWIN)
	// http://dmalloc.com/docs/latest/online/dmalloc_19.html
	dmalloc_debug_setup(getenv("DMALLOC_OPTIONS"));
#endif
#ifdef GCOLLECT
	// don't garbage collect, only report inaccessible memory that was not deallocated
	GC_find_leak = 1;
	GC_INIT();
#endif
#ifdef USE_MEMMGR
	memmgr_init();
#endif
}
