#pragma once

#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <switch.h>
#include <bitset>

class GetGameScreenshot {
private:
	// Global result variable for all functions
	Result rc;
	// 0x32000 is the size of the video buffer
	// I'm not quite sure why it is that, but it is
	u8* Vbuf;
	Service grcdVideo;

	// Borrowed from sysDVR
	Result grcdServiceOpen (Service* out) {
		if (serviceIsActive (out))
			return 0;

		rc = smGetService (out, "grc:d");

		if (R_FAILED (rc))
			grcdExit ();

		return rc;
	}

	// Borrowed from sysDVR
	void grcdServiceClose (Service* svc) {
		serviceClose (svc);
	}

	// Borrowed from sysDVR
	Result grcdServiceBegin (Service* svc) {
		return _grcCmdNoIO (svc, 1);
	}

	// Borrowed from sysDVR
	Result grcdServiceRead (Service* svc, GrcStream stream, void* buffer,
		size_t size, u32* unk, u32* data_size,
		u64* timestamp) {
		IpcCommand c;
		ipcInitialize (&c);

		ipcAddRecvBuffer (&c, buffer, size, BufferType_Normal);

		struct {
			u64 magic;
			u64 cmd_id;
			u32 stream;
		} * raw;

		raw = serviceIpcPrepareHeader (svc, &c, sizeof (*raw));

		raw->magic  = SFCI_MAGIC;
		raw->cmd_id = 2;
		raw->stream = stream;

		rc = serviceIpcDispatch (svc);

		if (R_SUCCEEDED (rc)) {
			IpcParsedCommand r;
			struct {
				u64 magic;
				u64 result;
				u32 unk;
				u32 data_size;
				u64 timestamp;
			} * resp;

			serviceIpcParse (svc, &r, sizeof (*resp));
			resp = r.Raw;

			rc = resp->result;

			if (R_SUCCEEDED (rc)) {
				if (unk)
					*unk = resp->unk;
				if (data_size)
					*data_size = resp->data_size;
				if (timestamp)
					*timestamp = resp->timestamp;
			}
		}

		return rc;
	}

	// Borrowed from sysDVR
	Result _grcCmdNoIO (Service* srv, u64 cmd_id) {
		IpcCommand c;
		ipcInitialize (&c);

		struct {
			u64 magic;
			u64 cmd_id;
		} * raw;

		raw = serviceIpcPrepareHeader (srv, &c, sizeof (*raw));

		raw->magic  = SFCI_MAGIC;
		raw->cmd_id = cmd_id;

		rc = serviceIpcDispatch (srv);

		if (R_SUCCEEDED (rc)) {
			IpcParsedCommand r;
			struct {
				u64 magic;
				u64 result;
			} * resp;

			serviceIpcParse (srv, &r, sizeof (*resp));
			resp = r.Raw;

			rc = resp->result;
		}

		return rc;
	}

public:
	GetGameScreenshot () {
		Vbuf = aligned_alloc (0x1000, 0x32000);
		// Open video service if it is not already open
		rc = grcdServiceOpen (&grcdVideo);
		if (R_FAILED (rc))
			fatalSimple (rc);
		rc = grcdServiceBegin (&grcdVideo);
		if (R_FAILED (rc))
			fatalSimple (rc);
	}

	u8* readFrame () {
		// This function produces H264 frames in Vbuf with a size of 1280 x 720
		// I'm not exactly sure what this is, but it's always 0
		u32 unk       = 0;
		u32 VOutSz    = 0;
		u64 timestamp = 0;
		// GrcStream_Video is a constant from LibNX
		// 0x32000 is VBufZ, size of the video buffer
		int fails = 0;
		// Runs multiple times to make sure it works
		// From sysDVR
		/*
    while (true) {
            Result res = grcdServiceRead(&grcdVideo, GrcStream_Video, Vbuf,
    0x32000, &unk, &VOutSz, &timestamp) if (R_SUCCEEDED(res) && VOutSz > 0)
    break; VOutSz = 0; if (fails++ > 8 && !IsThreadRunning) break;
    }
    */
		rc = grcdServiceRead (&grcdVideo, GrcStream_Video, Vbuf, 0x32000, &unk,
			&VOutSz, &timestamp);
		// Return pointer to data
		return Vbuf;
	}

	~GetGameScreenshot () {
		grcdServiceClose (&grcdVideo);
		free (Vbuf);
	}
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
	// Buffer
	uint8_t inbuf[INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];
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
		int num_bytes = avpicture_get_size(AV_PIX_FMT_GRAY16, outputWidth, outputHeight);
		// Allocate the memory for the returned image data
		returnedImageData = (u8*) malloc(num_bytes * sizeof(u8));
		// Neccessary so the width and height are ready
		// We want grayscale too
		avpicture_fill((AVPicture*) outputPicture, returnedImageData,
			AV_PIX_FMT_GRAY16, outputWidth, outputHeight);
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
		// Complete frames are not sent, so this is needed
		if(codec->capabilities&CODEC_CAP_TRUNCATED) {
        		c->flags|= CODEC_FLAG_TRUNCATED;
		}
		// Allocate picture that will be written
		picture = av_frame_alloc();
		// Set context width and height
		context->width = width;
		context->height = height;
		context->pix_fmt = AV_PIX_FMT_RGBA;
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
			// We want speed
			AV_PIX_FMT_GRAY16, SWS_FAST_BILINEAR,
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
		// Return the pointer to the AV_PIX_FMT_GRAY16 output buffer
		return returnedImageData;
	}
}

class GetScreenshotHash {
	private:
		GetGameScreenshot* gameScreenshotInstance;
		DecodeAndResize* decodeAndResizeInstance;
	
		constexpr int screenshotWidth = 1280;
		constexpr int screenshotHeight = 720;
	
		constexpr int hashWidth = 17;
		constexpr int hashHeight = 16;
	public:
	GetScreenshotHash() {
		gameScreenshotInstance = new GetGameScreenshot();
		// Number of bits of hash is 256 :) bytes is 32
		// We want accuracy
		// Target width and height is 17 x 16 (because 16 x 16 = 256)
		// Screenshot widths and heights and the size of the dhash buffer
		// It will also make the image grayscale
		decodeAndResizeInstance = new DecodeAndResize(screenshotWidth, screenshotHeight, hashWidth, hashHeight);
	}
	
	 std::bitset getDHash(u8* imageBuffer) {
		// Use std bitsets to hold the bits
		// The size should be 32 bytes
		int bitsetSize = ((hashWidth - 1) * hashHeight);
		std::bitset<bitsetSize> bits;
		int bitIndex = 0;
		for (int y = 0; y < hashHeight; y++) {
			for (int x = 0; x < (hashWidth - 1); x++) {
				int left = imageBuffer[hashWidth * y + x];
				// Shifted to the right 1
				int right = imageBuffer[hashWidth * y + x + 1];
				int left = px(pixels, width, col, row);
				int right = px(pixels, width, col + 1, row);
				// Set the bit in the hash
				bits.set(bitIndex, left < right);
				bitIndex++;
			}
		}
	}
	
	size_t getHammingDistance(std::bitset firstHash, std::bitset secondHash) {
		// This is actually it
		// https://stackoverflow.com/a/26168871 and https://gist.github.com/rogerpoon/fdb8a3317e20f8c110eb1e43ed5adafa
		return (firstHash ^ secondHash).count();
	}
	
	~GetScreenshotHash() {
		delete gameScreenshotInstance;
		delete decodeAndResizeInstance;
	}
}
