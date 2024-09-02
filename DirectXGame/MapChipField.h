#pragma once
#include <vector>
#include <Vector3.h>
#include <cstdint>
#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

enum class MapChipType {
	kBlank, // 空白
	kBlock, // ブロック
};

typedef struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
} MapChipData;

class MapChipField {

	// 1ブロックのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;
	// ブロックの個数
	static inline const uint32_t kNumBlockVertical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

	MapChipData mapChipData_;
	void ResetMapChipData(); // リセット
	void LoadMapChipCsv(const std::string& filePath); // 読み込み

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex); // マップチップ種別の取得
	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex); // マップチップ座標の取得
};