#pragma once

#include <vector>
#include <format>

#include "Core.h"

#define UE_LOG(fmt, ...) ConsoleWindow::GetInstance().AddLogPrintf(fmt, ##__VA_ARGS__)
#define UE_LOG_F(fmt, ...) ConsoleWindow::GetInstance().AddLogFormat(fmt, ##__VA_ARGS__)

class ConsoleWindow
{
public:
	ConsoleWindow();

	// Singleton pattern
	ConsoleWindow(const ConsoleWindow&) = delete;
	ConsoleWindow& operator=(const ConsoleWindow&) = delete;
	ConsoleWindow(ConsoleWindow&&) = delete;
	ConsoleWindow& operator=(ConsoleWindow&&) = delete;

	static ConsoleWindow& GetInstance();

	void Init(std::string_view title, int maxLines);

	template<typename... Args>
	void AddLogFormat(std::string_view fmt, Args&&... args)
	{
		addLog(
			std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...))
		);
	}

	template<typename... Args>
	void AddLogPrintf(const char* fmt, Args&&... args)
	{
		char buffer[512];
		snprintf(buffer, sizeof(buffer), fmt, std::forward<Args>(args)...);
		addLog(buffer);
	}

	void Draw(float panelWidth);

	static constexpr float HEIGHT_RATIO = 0.3f;

private:
	bool mbFirstFrame;

	// Configs
	FString mTitle;
	int mMaxLines;

	// Runtime data
	bool mbAutoScroll = true;
	std::vector<FString> mConsoleBuffer;
	
	void addLog(std::string_view message);
};
