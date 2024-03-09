#pragma once
#include "Ember.h"
enum ELogCategory
{
	Verbose,
	Debug,
	Info,
	Warning,
	Error,
	Critical,
};

static SDL_LogPriority GetLogPriority(ELogCategory Category)
{
	switch (Category)
	{
		case Verbose:  return SDL_LOG_PRIORITY_VERBOSE;
		case Debug:    return SDL_LOG_PRIORITY_DEBUG;
		case Info:     return SDL_LOG_PRIORITY_INFO;
		case Warning:  return SDL_LOG_PRIORITY_WARN;
		case Error:    return SDL_LOG_PRIORITY_ERROR;
		case Critical: return SDL_LOG_PRIORITY_CRITICAL;
	}

	return SDL_LOG_PRIORITY_INFO;
}

static void EmberLog(ELogCategory Category, const char* Fmt, ...)
{
	va_list Args;
	va_start(Args, Fmt);
	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, GetLogPriority(Category), Fmt, Args);
	va_end(Args);
}

#define EMBER_LOG(Category, Fmt, ...) EmberLog(Category, Fmt, __VA_ARGS__)