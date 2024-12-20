#pragma once

#include <chrono>

class FixFPS
{
public:
	//FPS固定初期化
	void InitialezeFixFPS();
	//FPS固定更新
	void UpdateFixFPS();

private:
	//記録時間（FPS固定用）
	std::chrono::steady_clock::time_point reference_;

	
};

