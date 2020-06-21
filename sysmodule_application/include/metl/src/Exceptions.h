#pragma once


namespace metl 
{
	struct BadLiteralException : public std::runtime_error { explicit BadLiteralException(const std::string& s) :runtime_error(s) {} };
}
