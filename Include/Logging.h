#pragma once
#include <map>
#include <SDL/SDL_log.h>

enum ELogCategory
{
	Verbose,
	Debug,
	Info,
	Warning,
	Error,
	Critical
};

static const std::map<ELogCategory, SDL_LogPriority> LogCategoryMap =
{
	{ Verbose,	SDL_LOG_PRIORITY_VERBOSE },
	{ Debug,	SDL_LOG_PRIORITY_DEBUG },
	{ Info,		SDL_LOG_PRIORITY_INFO },
	{ Warning,	SDL_LOG_PRIORITY_WARN },
	{ Error,	SDL_LOG_PRIORITY_ERROR },
	{ Critical, SDL_LOG_PRIORITY_CRITICAL },
};

static void EmberLog(ELogCategory Category, const char* Fmt, ...)
{
	va_list Args;
	va_start(Args, Fmt);
	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, LogCategoryMap.at(Category), Fmt, Args);
	va_end(Args);
}

#define EMBER_LOG(Category, Fmt, ...) EmberLog(Category, Fmt, __VA_ARGS__)