#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#include <mov.h>
#include <inttypes.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>



#define ASSERT(x) \
	do{ \
		if (!(x)){ \
			printf("%s:%"PRId32" (%s): %s: Assertion Failed\n",__FILE__,__LINE__,__func__,#x); \
			raise(SIGABRT); \
		} \
	} while (0)
#define BOX_HEADER_SIZE 8
#define BOX_HEADER_TYPE(a,b,c,d) CREATE_32BIT_BIG_ENDIAN((#a)[0],(#b)[0],(#c)[0],(#d)[0])
#define CREATE_32BIT_BIG_ENDIAN(a,b,c,d) ((((uint32_t)(a))<<24)|(((uint32_t)(b))<<16)|(((uint32_t)(c))<<8)|((uint32_t)(d)))
#define DREF_REF_VERSION 0
#define DREF_VERSION 0
#define GET_16BIT_BIG_ENDIAN(f) ((((uint32_t)(fgetc(f)))<<8)|((uint32_t)(fgetc(f))))
#define GET_24BIT_BIG_ENDIAN(f) ((((uint32_t)(fgetc(f)))<<16)|(((uint32_t)(fgetc(f)))<<8)|((uint32_t)(fgetc(f))))
#define GET_32BIT_BIG_ENDIAN(f) (((uint32_t)(fgetc(f))<<24)|(((uint32_t)(fgetc(f)))<<16)|(((uint32_t)(fgetc(f)))<<8)|((uint32_t)(fgetc(f))))
#define HDLR_VERSION 0
#define MDHD_VERSION 0
#define MVHD_VERSION 0
#define TRHD_VERSION 0
#define VMHD_FLAG_AFTER_QT_1_0 1
#define VMHD_VERSION 0



typedef struct __BOX_HEADER{
	uint32_t sz;
	uint32_t t;
} box_header_t;



uint8_t _read_box(FILE* f,box_header_t* b){
	uint8_t bf[BOX_HEADER_SIZE];
	if (fread((void*)bf,sizeof(uint8_t),BOX_HEADER_SIZE,f)!=BOX_HEADER_SIZE){
		return 0;
	}
	b->sz=(((uint32_t)(bf[0]))<<24)|(((uint32_t)(bf[1]))<<16)|(((uint32_t)(bf[2]))<<8)|((uint32_t)(bf[3]));
	b->t=(((uint32_t)(bf[4]))<<24)|(((uint32_t)(bf[5]))<<16)|(((uint32_t)(bf[6]))<<8)|((uint32_t)(bf[7]));
	if (b->sz<2){
		ASSERT(!"Unimplemented");
	}
	return 1;
}



uint8_t _parse_hdlr(FILE* f,uint32_t b_sz,mov_file_track_t* tr,const char* p){
	if (fgetc(f)!=HDLR_VERSION){
		ASSERT(!"Unimplemented");
	}
	fseek(f,3,SEEK_CUR);
	uint32_t c_t=GET_32BIT_BIG_ENDIAN(f);
	uint32_t c_st=GET_32BIT_BIG_ENDIAN(f);
	fseek(f,b_sz-BOX_HEADER_SIZE-12,SEEK_CUR);
	if (c_t==BOX_HEADER_TYPE(m,h,l,r)){
		if (c_st==BOX_HEADER_TYPE(v,i,d,e)){
			tr->f|=MOV_FILE_TRACK_FLAG_VIDEO;
		}
		else if (c_st==BOX_HEADER_TYPE(s,o,u,n)){
			tr->f|=MOV_FILE_TRACK_FLAG_AUDIO;
		}
		else if (c_st==BOX_HEADER_TYPE(s,u,b,t)){
			tr->f|=MOV_FILE_TRACK_FLAG_SUBTITLES;
		}
		else{
			printf("Warning: Unknown mhlr Type in moov:trak:%s:hdlr:mhlr %.8"PRIx32" (%c%c%c%c)\n",p,c_st,c_st>>24,(c_st>>16)&0xff,(c_st>>8)&0xff,c_st&0xff);
		}
	}
	else if (c_t==BOX_HEADER_TYPE(d,h,l,r)){
		printf("Warning: Unknown dhlr Type in moov:trak:%s:hdlr:dhlr %.8"PRIx32" (%c%c%c%c)\n",p,c_st,c_st>>24,(c_st>>16)&0xff,(c_st>>8)&0xff,c_st&0xff);
	}
	else{
		printf("Warning: Unknown hdlr Type in moov:trak:%s:hdlr %.8"PRIx32" (%c%c%c%c)\n",p,c_t,c_t>>24,(c_t>>16)&0xff,(c_t>>8)&0xff,c_t&0xff);
	}
	return 1;
}



uint8_t load_mov(const char* fp,mov_file_t* o){
	o->v=NULL;
	o->c_tm=0;
	o->m_tm=0;
	o->fps=0;
	o->d=0;
	o->tl.dt=NULL;
	o->tl.l=0;
	o->_dt.dt=NULL;
	o->_dt.l=0;
	FILE* f=fopen(fp,"rb");// lgtm [cpp/path-injection]
	if (!f){
		goto _error;
	}
	box_header_t b;
	if (!_read_box(f,&b)){
		goto _error;
	}
	if (b.t!=BOX_HEADER_TYPE(f,t,y,p)){
		goto _error;
	}
	o->v=malloc(sizeof(mov_file_version_t)+((b.sz-16)>>2)*sizeof(uint32_t));
	o->v->t=GET_32BIT_BIG_ENDIAN(f);
	o->v->v=GET_32BIT_BIG_ENDIAN(f);
	o->v->sll=(b.sz-16)>>2;
	for (uint32_t i=0;i<(b.sz-16)>>2;i++){
		o->v->sl[i]=GET_32BIT_BIG_ENDIAN(f);
	}
	if (o->v->t!=CREATE_32BIT_BIG_ENDIAN('q','t',' ',' ')){
		ASSERT(!"Unimplemented");
	}
	while (1){
		if (!_read_box(f,&b)){
			break;
		}
		if (b.t==BOX_HEADER_TYPE(m,o,o,v)){
			uint32_t sz=b.sz-BOX_HEADER_SIZE;
			while (sz){
				if (!_read_box(f,&b)){
					goto _error;
				}
				sz-=b.sz;
				if (b.t==BOX_HEADER_TYPE(m,v,h,d)){
					if (fgetc(f)!=MVHD_VERSION){
						ASSERT(!"Unimplemented");
					}
					fseek(f,3,SEEK_CUR);
					o->c_tm=GET_32BIT_BIG_ENDIAN(f);
					o->m_tm=GET_32BIT_BIG_ENDIAN(f);
					o->fps=GET_32BIT_BIG_ENDIAN(f);
					o->d=GET_32BIT_BIG_ENDIAN(f);
					fseek(f,b.sz-BOX_HEADER_SIZE-20,SEEK_CUR);
				}
				else if (b.t==BOX_HEADER_TYPE(t,r,a,k)){
					mov_file_track_t tr={0};
					uint32_t trak_sz=b.sz-BOX_HEADER_SIZE;
					while (trak_sz){
						if (!_read_box(f,&b)){
							break;
						}
						trak_sz-=b.sz;
						if (b.t==BOX_HEADER_TYPE(t,k,h,d)){
							if (fgetc(f)!=TRHD_VERSION){
								ASSERT(!"Unimplemented");
							}
							uint32_t fl=GET_24BIT_BIG_ENDIAN(f);
							if (fl&1){
								tr.f|=MOV_FILE_TRACK_FLAG_ENABLED;
							}
							if (fl&2){
								tr.f|=MOV_FILE_TRACK_FLAG_IN_MOVIE;
							}
							if (fl&4){
								tr.f|=MOV_FILE_TRACK_FLAG_IN_PREVIEW;
							}
							if (fl&8){
								tr.f|=MOV_FILE_TRACK_FLAG_IN_POSTER;
							}
							tr.c_tm=GET_32BIT_BIG_ENDIAN(f);
							tr.m_tm=GET_32BIT_BIG_ENDIAN(f);
							tr.id=GET_32BIT_BIG_ENDIAN(f);
							fseek(f,4,SEEK_CUR);
							tr.d=GET_32BIT_BIG_ENDIAN(f);
							fseek(f,52,SEEK_CUR);
							tr.w=GET_32BIT_BIG_ENDIAN(f);
							tr.h=GET_32BIT_BIG_ENDIAN(f);
						}
						else if (b.t==BOX_HEADER_TYPE(m,d,i,a)){
							uint32_t mdia_sz=b.sz-BOX_HEADER_SIZE;
							while (mdia_sz){
								if (!_read_box(f,&b)){
									break;
								}
								mdia_sz-=b.sz;
								if (b.t==BOX_HEADER_TYPE(m,d,h,d)){
									if (fgetc(f)!=MDHD_VERSION){
										ASSERT(!"Unimplemented");
									}
									fseek(f,3,SEEK_CUR);
									tr.c_tm=GET_32BIT_BIG_ENDIAN(f);
									tr.m_tm=GET_32BIT_BIG_ENDIAN(f);
									tr.fps=GET_32BIT_BIG_ENDIAN(f);
									tr.d=GET_32BIT_BIG_ENDIAN(f);
									fseek(f,b.sz-BOX_HEADER_SIZE-20,SEEK_CUR);
								}
								else if (b.t==BOX_HEADER_TYPE(h,d,l,r)){
									if (!_parse_hdlr(f,b.sz,&tr,"mdia")){
										goto _error;
									}
								}
								else if (b.t==BOX_HEADER_TYPE(m,i,n,f)){
									uint32_t minf_sz=b.sz-BOX_HEADER_SIZE;
									while (minf_sz){
										if (!_read_box(f,&b)){
											break;
										}
										minf_sz-=b.sz;
										if (b.t==BOX_HEADER_TYPE(v,m,h,d)){
											if (fgetc(f)!=VMHD_VERSION){
												ASSERT(!"Unimplemented");
											}
											if (GET_24BIT_BIG_ENDIAN(f)!=VMHD_FLAG_AFTER_QT_1_0){
												ASSERT(!"Unimplemented");
											}
											uint16_t gm=GET_16BIT_BIG_ENDIAN(f);
											if (gm){
												ASSERT(!"Unimplemented");
											}
											fseek(f,6,SEEK_CUR);
										}
										else if (b.t==BOX_HEADER_TYPE(h,d,l,r)){
											if (!_parse_hdlr(f,b.sz,&tr,"mdia:minf")){
												goto _error;
											}
										}
										else if (b.t==BOX_HEADER_TYPE(d,i,n,f)){
											if (!_read_box(f,&b)){
												goto _error;
											}
											if (b.t==BOX_HEADER_TYPE(d,r,e,f)){
												if (fgetc(f)!=DREF_VERSION){
													ASSERT(!"Unimplemented");
												}
												fseek(f,3,SEEK_CUR);
												uint32_t sz=GET_32BIT_BIG_ENDIAN(f);
												for (uint32_t i=0;i<sz;i++){
													uint32_t r_sz=GET_32BIT_BIG_ENDIAN(f);
													fseek(f,4,SEEK_CUR);
													if (fgetc(f)!=DREF_REF_VERSION){
														ASSERT(!"Unimplemented");
													}
													fseek(f,3,SEEK_CUR);
													uint32_t bfl=r_sz-12;
													if (bfl){
														ASSERT(!"Unimplemented");
													}
												}
											}
											else{
												printf("Warning: Unknown dinf Type in moov:trak:mdia:minf:dinf %.8"PRIx32" (%c%c%c%c) (%"PRIu32" bytes)\n",b.t,b.t>>24,(b.t>>16)&0xff,(b.t>>8)&0xff,b.t&0xff,b.sz);
												fseek(f,b.sz-BOX_HEADER_SIZE,SEEK_CUR);
											}
										}
										else if (b.t==BOX_HEADER_TYPE(s,t,b,l)){
											uint32_t stbl_sz=b.sz-BOX_HEADER_SIZE;
											while (stbl_sz){
												if (!_read_box(f,&b)){
													goto _error;
												}
												stbl_sz-=b.sz;
												printf("Warning: Unknown stbl Type in moov:trak:mdia:minf:stbl %.8"PRIx32" (%c%c%c%c) (%"PRIu32" bytes)\n",b.t,b.t>>24,(b.t>>16)&0xff,(b.t>>8)&0xff,b.t&0xff,b.sz);
												fseek(f,b.sz-BOX_HEADER_SIZE,SEEK_CUR);
											}
										}
										else{
											printf("Warning: Unknown minf Type in moov:trak:mdia:minf %.8"PRIx32" (%c%c%c%c) (%"PRIu32" bytes)\n",b.t,b.t>>24,(b.t>>16)&0xff,(b.t>>8)&0xff,b.t&0xff,b.sz);
											fseek(f,b.sz-BOX_HEADER_SIZE,SEEK_CUR);
										}
									}
								}
								else{
									printf("Warning: Unknown mdia Type in moov:trak:mdia %.8"PRIx32" (%c%c%c%c) (%"PRIu32" bytes)\n",b.t,b.t>>24,(b.t>>16)&0xff,(b.t>>8)&0xff,b.t&0xff,b.sz);
									fseek(f,b.sz-BOX_HEADER_SIZE,SEEK_CUR);
								}
							}
						}
						else{
							printf("Warning: Unknown trak Type in moov:trak %.8"PRIx32" (%c%c%c%c) (%"PRIu32" bytes)\n",b.t,b.t>>24,(b.t>>16)&0xff,(b.t>>8)&0xff,b.t&0xff,b.sz);
							fseek(f,b.sz-BOX_HEADER_SIZE,SEEK_CUR);
						}
					}
				}
				else{
					printf("Warning: Unknown moov Type in moov %.8"PRIx32" (%c%c%c%c) (%"PRIu32" bytes)\n",b.t,b.t>>24,(b.t>>16)&0xff,(b.t>>8)&0xff,b.t&0xff,b.sz);
					fseek(f,b.sz-BOX_HEADER_SIZE,SEEK_CUR);
				}
			}
		}
		else if (b.t==BOX_HEADER_TYPE(m,d,a,t)){
			o->_dt.l++;
			o->_dt.dt=realloc(o->_dt.dt,o->_dt.l*sizeof(_mov_file_data_t));
			(o->_dt.dt+o->_dt.l-1)->off=0;
			(o->_dt.dt+o->_dt.l-1)->sz=b.sz-BOX_HEADER_SIZE;
			(o->_dt.dt+o->_dt.l-1)->bf=malloc((b.sz-BOX_HEADER_SIZE)*sizeof(uint8_t));
			if (fread((void*)((o->_dt.dt+o->_dt.l-1)->bf),sizeof(uint8_t),b.sz-BOX_HEADER_SIZE,f)!=b.sz-BOX_HEADER_SIZE){
				goto _error;
			}
		}
		else{
			printf("Warning: Unknown Type %.8"PRIx32" (%c%c%c%c) (%"PRIu32" bytes)\n",b.t,b.t>>24,(b.t>>16)&0xff,(b.t>>8)&0xff,b.t&0xff,b.sz);
			fseek(f,b.sz-BOX_HEADER_SIZE,SEEK_CUR);
		}
	}
	if (f){
		fclose(f);
	}
	return 1;
_error:
	if (f){
		fclose(f);
	}
	if (o->v){
		free(o->v);
		o->v=NULL;
	}
	if (o->tl.dt){
		free(o->tl.dt);
		o->tl.dt=NULL;
	}
	if (o->_dt.dt){
		for (uint32_t i=0;i<o->_dt.l;i++){
			free((o->_dt.dt+i)->bf);
		}
		free(o->_dt.dt);
		o->_dt.dt=NULL;
	}
	return 0;
}



void free_mov(mov_file_t* f){
	if (f->v){
		free(f->v);
		f->v=NULL;
	}
	if (f->tl.dt){
		free(f->tl.dt);
		f->tl.dt=NULL;
	}
	if (f->_dt.dt){
		for (uint32_t i=0;i<f->_dt.l;i++){
			free((f->_dt.dt+i)->bf);
		}
		free(f->_dt.dt);
		f->_dt.dt=NULL;
	}
}
