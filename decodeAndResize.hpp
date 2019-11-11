#pragma once

// Finally decided to just use FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <switch.h>
}

class DecodeAndResize() {
	private:
	AVCodec* codec;
	// The first step, decoding
	AVCodecContext* context;
	AVFrame* picture;
	// Resizing context
	SwsContext* resize;
	// Contains input H264 buffer
	AVPacket avpkt;
	// Dimensions
	int inputWidth;
	int inputHeight;
	int outputWidth:
	int outputHeight;
	// Output picture
	AVFrame* outputPicture;
	// Output buffer, freed by this program
	u8* returnedImageData;

	void setupFrameBuffers() {
		int num_bytes = avpicture_get_size(AV_PIX_FMT_RGBA, outputWidth, outputHeight);
		// Allocate the memory for the returned image data
		returnedImageData = (u8*) malloc(num_bytes * sizeof(u8));
		// Neccessary so the width and height are ready
		avpicture_fill((AVPicture*) outputPicture, returnedImageData,
			AV_PIX_FMT_RGBA, outputWidth, outputHeight);
	}

	public:
	DecodeAndResize(int width, int height, int targetWidth, int targetHeight) {
		inputWidth = width;
		inputHeight = height;
		outputWidth = targetWidth;
		outputHeight = targetHeight;
		// Initialize the input packet
		av_init_packet(&avpkt);
		// Set codec to H264
		codec = avcodec_find_decoder(AV_CODEC_ID_H264);
		// Allocate AVCodecContext
		context = avcodec_alloc_context3(codec);
		// Allocate picture that will be written
		picture = av_frame_alloc();
		// Set context width and height
		context->height = width;
    	context->width = height;
		context->pix_fmt = AV_PIX_FMT_RGBA;
		// I don't know why this is needed, but everybody is doing it
		//if (codec->capabilities & CODEC_CAP_TRUNCATED) {
			// Don't send complete frames ???
			//context->flags |= CODEC_FLAG_TRUNCATED;
		//}
		// Same thing for the image
		picture->height = width;
    	picture->width = height;
		picture->format = context->pix_fmt;
		// Init the context now
		avcodec_open2(context, codec, NULL);
		// Create resizing context
		// SWS_FAST_BILINEAR is the fastest
		// https://ffmpeg.org/doxygen/2.7/swscale_8h_source.html#l00055
		// The last 3 nulls are completely useless for now
		resize = sws_getContext(width, height,
			AV_PIX_FMT_RGBA, targetWidth, targetHeight,
			AV_PIX_FMT_RGBA, SWS_FAST_BILINEAR,
			NULL, NULL, NULL);
		// Create buffers for frames
		setupFrameBuffers();
	}

	~DecodeAndResize() {
		// Take apart all this cruft
		avcodec_close(context);
    	av_free(context);
		av_freep(&picture->data[0]);
    	av_frame_free(&picture);
		av_freep(&outputPicture->data[0]);
		av_frame_free(&outputPicture);
		sws_freeContext(resize);
		av_packet_unref(&avpkt);
		free(returnedImageData);
	}

	u8* decodeAndResize(u8* inputH264Data, size_t sizeOfBuf) {
		// Set data of input packet
		avpkt.data = inputH264Data;
		// I think this is the right size, but it isn't clear
		avpkt.size = sizeOfBuf + FF_INPUT_BUFFER_PADDING_SIZE;
		// I think this means success
		int gotPicture;
		avcodec_decode_video2(context, picture, &got_picture, &avpkt);
		// Picture obtained, time to resize
		sws_scale(resize, picture->data, picture->linesize, 0,
			inputHeight, outputPicture->data, outputPicture->linesize);
		// Free memory of the input data
		free(inputH264Data);
		// Return the pointer to the RGBA output buffer
		return returnedImageData;
	}
}