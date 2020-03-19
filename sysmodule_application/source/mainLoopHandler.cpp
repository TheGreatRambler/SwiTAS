#include "mainLoopHandler.hpp"

MainLoop::MainLoop() {
	// Start networking with set queues
	networkInstance = std::make_shared<CommunicateWithNetwork>(
		[](CommunicateWithNetwork* self) {
			SEND_QUEUE_DATA(RecieveDone)
			SEND_QUEUE_DATA(RecieveGameInfo)
			SEND_QUEUE_DATA(RecieveGameFramebuffer)
		},
		[](CommunicateWithNetwork* self) {
			RECIEVE_QUEUE_DATA(SendStart)
			RECIEVE_QUEUE_DATA(SendRunFrame)
		});
}

void MainLoop::mainLoopHandler() {
	rc = pmdmntGetApplicationProcessId(&applicationProcessId);

	// Lifted from switchPresense-Rewritten
	if(R_SUCCEEDED(rc)) {
		// Application connected
		if(!applicationOpened) {
			// Get application info
			char* gameName;
			rc = pminfoGetProgramId(&applicationProgramId, applicationProcessId);
			if(R_SUCCEEDED(rc)) {
				gameName = getAppName(applicationProgramId);
				// Start the whole main loop
			}
			// Let PC know

			applicationOpened = true;
		}
	} else {
		// I believe this means that there is no application running
		// If there was just an application open, let the PC know
		if(applicationOpened) {

			applicationOpened = false;
		}
	}

	svcSleepThread(5e+9);
}

char* MainLoop::getAppName(u64 application_id) {
	static NsApplicationControlData appControlData = { 0 };
	size_t appControlDataSize                      = 0;
	NacpLanguageEntry* languageEntry               = nullptr;

	if(R_SUCCEEDED(nsGetApplicationControlData(NsApplicationControlSource_Storage, application_id, &appControlData, sizeof(NsApplicationControlData), &appControlDataSize))) {
		if(R_SUCCEEDED(nacpGetLanguageEntry(&appControlData.nacp, &languageEntry))) {
			if(languageEntry != nullptr)
				return languageEntry->name;
		}
	}
	return "";
}