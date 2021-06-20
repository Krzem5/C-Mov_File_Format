#include <mov.h>



int main(int argc,const char** argv){
	mov_file_t f={0};
	load_mov("rsrc/video.mov",&f);
	free_mov(&f);
	return 0;
}
