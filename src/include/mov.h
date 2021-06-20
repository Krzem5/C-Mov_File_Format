#ifndef __MOV_H__
#define __MOV_H__ 1
#include <stdint.h>



#define MOV_FILE_TRACK_FLAG_VIDEO 1
#define MOV_FILE_TRACK_FLAG_AUDIO 2
#define MOV_FILE_TRACK_FLAG_SUBTITLES 3
#define MOV_FILE_TRACK_FLAG_TYPE_MASK 3
#define MOV_FILE_TRACK_FLAG_ENABLED 4
#define MOV_FILE_TRACK_FLAG_IN_MOVIE 8
#define MOV_FILE_TRACK_FLAG_IN_PREVIEW 16
#define MOV_FILE_TRACK_FLAG_IN_POSTER 32



typedef struct __MOV_FILE_VERSION{
	uint32_t t;
	uint32_t v;
	uint8_t sll;
	uint32_t sl[];
} mov_file_version_t;



typedef struct __MOV_FILE_TRACK{
	uint32_t id;
	uint32_t c_tm;
	uint32_t m_tm;
	uint32_t fps;
	uint32_t d;
	uint32_t w;
	uint32_t h;
	uint8_t f;
} mov_file_track_t;



typedef struct __MOV_FILE_TRACK_LIST{
	mov_file_track_t* dt;
	uint32_t l;
} mov_file_track_list_t;



typedef struct ___MOV_FILE_DATA{
	uint32_t off;
	uint32_t sz;
	uint8_t* bf;
} _mov_file_data_t;



typedef struct ___MOV_FILE_DATA_LIST{
	_mov_file_data_t* dt;
	uint32_t l;
} _mov_file_data_list_t;



typedef struct __MOV_FILE{
	mov_file_version_t* v;
	uint32_t c_tm;
	uint32_t m_tm;
	uint32_t fps;
	uint32_t d;
	mov_file_track_list_t tl;
	_mov_file_data_list_t _dt;
} mov_file_t;



uint8_t load_mov(const char* fp,mov_file_t* o);



void free_mov(mov_file_t* f);



#endif
