#pragma once

#include <string.h>
#include <switch.h>
#include <stdlib.h>
#include <NvPipe.h>
// This is needed for image resizing
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "ThirdParty/stb_image_resize.h"

class UI() {
	private:
	// 0x32000 is the size of the video buffer
	// I'm not quite sure why it is that, but it is
	constexpr u8* Vbuf = aligned_alloc(0x1000, 0x32000);
	Service grcdVideo;
	// NVidia H264 decoder
	NvPipe* nvidiaDecoder;
	// Dimensions of the screen
	constexpr int width = 1280;
	constexpr int height = 720;
	// Dimensions of the game window in the corner
	constexpr int gameWidth = 960;
	constexpr int gameHeight = 540;
	// RGBA buffers
	u8* inputRgbaBuffer = malloc(width * height * 4);
	u8* outputRgbaBuffer = malloc(gameWidth * gameHeight * 4);

// Borrowed from sysDVR
Result grcdServiceOpen(Service* out) {
    if (serviceIsActive(out))
        return 0;

    Result rc = smGetService(out, "grc:d");

    if (R_FAILED(rc)) grcdExit();

    return rc;
}

// Borrowed from sysDVR
void grcdServiceClose(Service* svc){
	serviceClose(svc);
}

// Borrowed from sysDVR
Result grcdServiceBegin(Service* svc){
    return _grcCmdNoIO(svc, 1);
}

// Borrowed from sysDVR
Result grcdServiceRead(Service* svc, GrcStream stream, void* buffer, size_t size, u32 *unk, u32 *data_size, u64 *timestamp) {
    IpcCommand c;
    ipcInitialize(&c);

    ipcAddRecvBuffer(&c, buffer, size, BufferType_Normal);

    struct {
        u64 magic;
        u64 cmd_id;
        u32 stream;
    } *raw;

    raw = serviceIpcPrepareHeader(svc, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 2;
    raw->stream = stream;

    Result rc = serviceIpcDispatch(svc);

    if (R_SUCCEEDED(rc)) {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
            u32 unk;
            u32 data_size;
            u64 timestamp;
        } *resp;

        serviceIpcParse(svc, &r, sizeof(*resp));
        resp = r.Raw;

        rc = resp->result;

        if (R_SUCCEEDED(rc)) {
            if (unk) *unk = resp->unk;
            if (data_size) *data_size = resp->data_size;
            if (timestamp) *timestamp = resp->timestamp;
        }
    }

    return rc;
}

// Borrowed from sysDVR
Result _grcCmdNoIO(Service* srv, u64 cmd_id) {
	IpcCommand c;
	ipcInitialize(&c);

	struct {
		u64 magic;
		u64 cmd_id;
	} *raw;

	raw = serviceIpcPrepareHeader(srv, &c, sizeof(*raw));

	raw->magic = SFCI_MAGIC;
	raw->cmd_id = cmd_id;

	Result rc = serviceIpcDispatch(srv);

	if (R_SUCCEEDED(rc)) {
		IpcParsedCommand r;
		struct {
			u64 magic;
			u64 result;
		} *resp;

		serviceIpcParse(srv, &r, sizeof(*resp));
		resp = r.Raw;

		rc = resp->result;
	}

	return rc;
}

	public:
	NxTASUI() {
		// Open video service if it is not already open
		Result rc;
		rc = grcdServiceOpen(&grcdVideo);
		if (R_FAILED(rc)) fatalSimple(rc);
		rc = grcdServiceBegin(&grcdVideo);
		if (R_FAILED(rc)) fatalSimple(rc);
		// Create Nvenc decoder
		nvidiaDecoder = NvPipe_CreateDecoder(NVPIPE_RGBA32, NVPIPE_H264, width, height);
	}

	void getCurrentFrame() {
		// This function produces H264 frames in Vbuf with a size of 1280 x 720
		// I'm not exactly sure what this is, but it's always 0
		u32 unk = 0;
		u32 VOutSz = 0;
		u64 timestamp = 0;
		// GrcStream_Video is a constant from LibNX
		// 0x32000 is VBufZ, size of the video buffer
		int fails = 0;
		// Runs multiple times to make sure it works
		// From sysDVR
		while (true) {
			Result res = grcdServiceRead(&grcdVideo, GrcStream_Video, Vbuf, 0x32000, &unk, &VOutSz, &timestamp)
			if (R_SUCCEEDED(res) && VOutSz > 0) break;
			VOutSz = 0;
			if (fails++ > 8 && !IsThreadRunning) break;
		}
		// Decode the H264 frame with Nvenc, hardware decoding
		NvPipe_Decode(nvidiaDecoder, Vbuf, sizeof(Vbuf), inputRgbaBuffer, width, height);
		// Put the RGBA data into the buffer
		// Assuming it's 4 bytes per pixel
		stbir_resize_uint8_srgb(inputRgbaBuffer, width, height, 0, outputRgbaBuffer, gameWidth, gameHeight, 0, 4, STBIR_FLAG_ALPHA_PREMULTIPLIED, 0);
		// At this point, an RGBA buffer of the resized game window is in `outputRgbaBuffer`
	}

	~NxTASUI() {
		// Stop service
		grcdServiceClose(&grcdVideo);
		// Free the video buffers, both H264 and raw RGBA
		free(Vbuf);
		free(inputRgbaBuffer);
		free(outputRgbaBuffer);
		// Close the Nvidia decoder
		NvPipe_Destroy(decoder);
	}
}