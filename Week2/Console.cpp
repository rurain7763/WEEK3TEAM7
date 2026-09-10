#include <format>

#include "ImGui/imgui.h"

#include "Console.h"

ConsoleWindow::ConsoleWindow()
	: mTitle("Console Window")
	, mMaxLines(100)
	, mbFirstFrame(true)
{

}

ConsoleWindow& ConsoleWindow::GetInstance()
{
	static ConsoleWindow instance;
	return instance;
}

void ConsoleWindow::Init(std::string_view title, int maxLines)
{
	mTitle = title;
	mMaxLines = maxLines;
	mConsoleBuffer.reserve(maxLines);
}

void ConsoleWindow::Draw(float panelWidth)
{
	ImGuiIO& io = ImGui::GetIO();

	float consolHeight = io.DisplaySize.y * HEIGHT_RATIO;

	if (mbFirstFrame)
	{
		mbFirstFrame = false;
	}

	ImGui::SetNextWindowPos(
		ImVec2(panelWidth, io.DisplaySize.y - consolHeight),
		ImGuiCond_Always
	);

	ImGui::SetNextWindowSize(
		ImVec2(io.DisplaySize.x - panelWidth, consolHeight),
		ImGuiCond_Always
	);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;

	ImGui::Begin(mTitle.CStr(), nullptr, flags);
	// Draw console buffer
	if (ImGui::BeginChild("ConsoleMessage", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true))
	{
		// Auto-scroll to bottom if enabled
		const bool bWasAtBottom = (ImGui::GetScrollY() >= ImGui::GetScrollMaxY());

		for (const auto& message : mConsoleBuffer)
		{
			ImGui::TextUnformatted(message.CStr());
		}

		if (mbAutoScroll && bWasAtBottom)
		{
			ImGui::SetScrollHereY(1.0f);
		}
	}

	ImGui::EndChild();
	ImGui::End();
}

void ConsoleWindow::addLog(std::string_view message)
{
	if (mConsoleBuffer.size() >= mMaxLines)
	{
		mConsoleBuffer.erase(mConsoleBuffer.begin());
	}

	mConsoleBuffer.emplace_back(message);
}
