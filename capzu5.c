#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <time.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define NB_BUFFER   4

struct vd {
	int fd;
	char *videodev;
	struct v4l2_capability cap;
	struct v4l2_format fmt;
	struct v4l2_requestbuffers rb;
	struct v4l2_buffer buf;
	void *memstart[NB_BUFFER];
	int width;
	int height;
	int formatin;
	int framesizein;
	unsigned char * framebuffer;
	int isstreaming;
};

int initvd(struct vd *vdin)
{
	int i;
	int ret = 0;
	int type;
	struct v4l2_plane planes[VIDEO_MAX_PLANES];

	//open dev.
	vdin->fd = open(vdin->videodev, O_RDWR);
	if (vdin->fd < 0) {
		printf("Error open v4l device.\n");
		goto err;
	}

	//get capability.
	memset(&vdin->cap, 0, sizeof(struct v4l2_capability));
	ret = ioctl(vdin->fd, VIDIOC_QUERYCAP, &vdin->cap);
	if (ret < 0) {
		printf("Unable to query device.\n");
		goto err;
	}
	if ((vdin->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
		printf("video capture not supported.\n");
		goto err;
	}
	if ((vdin->cap.capabilities & V4L2_CAP_READWRITE)) {
		printf("I/O read not supported.\n");
		goto err;
	}

	//set format.
	memset(&vdin->fmt, 0, sizeof(struct v4l2_format));
	vdin->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vdin->fmt.fmt.pix_mp.width = vdin->width;
	vdin->fmt.fmt.pix_mp.height = vdin->height;
	vdin->fmt.fmt.pix_mp.pixelformat = vdin->formatin;
	vdin->fmt.fmt.pix_mp.field = V4L2_FIELD_ANY;
	ret = ioctl(vdin->fd, VIDIOC_S_FMT, &vdin->fmt);
	if (ret < 0) {
		printf("Unable to set format.\n");
		goto err;
	}

	// requset buffers.
	memset(&vdin->rb, 0, sizeof(struct v4l2_requestbuffers));
	vdin->rb.count = NB_BUFFER;
	vdin->rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vdin->rb.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(vdin->fd, VIDIOC_REQBUFS, &vdin->rb);	
	if (ret < 0) {
		printf("Unable to alloc buffers.\n");
		goto err;
	}

	// map buffers.
	for (i = 0; i < NB_BUFFER; i++) {
		memset(&vdin->buf, 0, sizeof(struct v4l2_buffer));
		vdin->buf.index = i;
		vdin->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		vdin->buf.memory = V4L2_MEMORY_MMAP;
		//vdin->buf.m.planes = planes;
		//vdin->buf.length = 1;
		memset(planes, 0, sizeof(planes));
		ret = ioctl(vdin->fd, VIDIOC_QUERYBUF, &vdin->buf);
		if (ret < 0) {
			printf("Unable to query buffer.\n");
			goto err;
		}

		vdin->memstart[i] = mmap(0, vdin->buf.length, PROT_READ, MAP_SHARED,
				vdin->fd, vdin->buf.m.offset);
		if (vdin->memstart[i] == MAP_FAILED) {
			printf("Unable to map buffer.\n");
			goto err;
		}
	}

	// queue buffers.
	for (i = 0; i < NB_BUFFER; i++) {
		memset(&vdin->buf, 0, sizeof(struct v4l2_buffer));
		vdin->buf.index = i;
		vdin->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		vdin->buf.memory = V4L2_MEMORY_MMAP;
		//vdin->buf.m.planes = planes;
		//vdin->buf.length = 1;
		//memset(planes, 0, sizeof(planes));
		ret = ioctl(vdin->fd, VIDIOC_QBUF, &vdin->buf);
		if (ret < 0) {
			printf("Unable to queue buffer.\n");
			goto err;
		}
	}

	vdin->framesizein = vdin->width * vdin->height * 3;
	vdin->framebuffer = (unsigned char *)calloc(1, (size_t)(vdin->framesizein));
	if(!vdin->framebuffer) {
		printf("calloc failed\n");
		goto err;
	}

	//start stream.
	//printf("!!!! %s0\n", __func__);
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(vdin->fd, VIDIOC_STREAMON, &type);
	if (ret < 0) {
		printf("Unable to start capture.\n");
		goto err;
	}
	vdin->isstreaming = 1;

	//printf("!!!! %s1\n", __func__);
	return 0;

 err:
	return -1;
}

int capturevd(struct vd *vdin)
{
	int ret;
//	struct v4l2_plane planes[VIDEO_MAX_PLANES];

	printf("! %s\n", __func__);
	memset(&vdin->buf, 0, sizeof(struct v4l2_buffer));
	vdin->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vdin->buf.memory = V4L2_MEMORY_MMAP;
	//vdin->buf.m.planes = planes;
	//vdin->buf.length = 1;
	//memset(planes, 0, sizeof(planes));
	ret = ioctl(vdin->fd, VIDIOC_DQBUF, &vdin->buf);
	if (ret < 0) {
		printf(" Unable to dequeue buffer.\n");
		goto errc;
	}

	if (vdin->buf.bytesused > vdin->framesizein) {
		printf("! %s0\n", __func__);
		memcpy(vdin->framebuffer, vdin->memstart[vdin->buf.index],
			(size_t)vdin->framesizein);	
	} else {
		printf("! %s1\n", __func__);
		memcpy(vdin->framebuffer, vdin->memstart[vdin->buf.index],
			(size_t)vdin->buf.bytesused);
	}
	
	ret = ioctl(vdin->fd, VIDIOC_QBUF, &vdin->buf);
	if (ret < 0) {
		printf(" Unable to requeue buffer.\n");
		goto errc;
	}

	return 0;

 errc:
	return -1;
} 

int closevd(struct vd *vdin)
{
	int i;
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret;

	if (vdin->isstreaming) {
		ret = ioctl(vdin->fd, VIDIOC_STREAMOFF, &type);
		if (ret < 0) {
			printf("Unable to stop capture.\n");
			goto errcls;
		}
		vdin->isstreaming = 0;
	}

	for (i = 0; i < NB_BUFFER; i++) {
		munmap(vdin->memstart[i], vdin->buf.length);
	}

	free(vdin->framebuffer);
	close(vdin->fd);
	return 0;

 errcls:
	return -1;
}

int main(int argc, char **argv)
{
	char *videodev = "/dev/video1";
	char *outputfile = "rgb888";
	int width = 1920;
	int height = 1080;
	//int formatin = V4L2_PIX_FMT_YUYV;
	int formatin = V4L2_PIX_FMT_RGB24;
	struct vd vdin; 
	time_t ref_time;
//	int quality = 95;
	int delay = 0;
	FILE *file;
//	int ret = 0;
	char buf[6];
	char linebuf[3840];
	int i, j;

	vdin.videodev = videodev;
	vdin.width = width;
	vdin.height = height;
	vdin.formatin = formatin;
	vdin.isstreaming = 0; 
	if (initvd(&vdin) < 0) {
		printf("Error init v4l device.\n");
		exit(1);
	}

	memset(&linebuf[0], 0x0, 3840);

	ref_time = time(NULL);
	while (1) {
		if (capturevd(&vdin) < 0) {
			printf("Error capture v4l stream.\n");
			closevd(&vdin);	
			exit(1);
		}
		
		if (difftime(time(NULL), ref_time) > delay || delay == 0) {
			file = fopen(outputfile, "wb");
			if (file != NULL) {
				for(j = 0; j < 1080; j++) {
					for(i = 0; i < 1920; i++) {
						memcpy(&buf[0], vdin.framebuffer + j * 1920 * 3 + i * 3, 3); 	
						buf[4] = ((buf[1] << 3) & 0xe0) | ((buf[0] >> 3) & 0x1f);
						buf[5] = ((buf[1] >> 5) & 0x7) | (buf[2] & 0xf8);	
						fwrite(&buf[4], 1, 2, file);
					}
					fwrite(&linebuf[0], 1, 3840, file);
				}
			}
			fclose(file);
		}

		ref_time = time(NULL);		
		if (delay == 0)	break;
	}

	closevd(&vdin);
		
	return 0;
}
