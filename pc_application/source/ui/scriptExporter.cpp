#include "scriptExporter.hpp"

ScriptExporter::ScriptExporter(std::shared_ptr<ProjectHandler> projHandler, , std::string data) {
	projectHandler = projHandler;
	dataToSave     = data;

	// TODO
	// IP selector (text)
	// Port Selecter (number with range of ports)
	// Path (text with leading slash required)
	// Submit button for FTP (reject if invalid path)

	// Filesystem selector (simple file selector, allow creation)
}