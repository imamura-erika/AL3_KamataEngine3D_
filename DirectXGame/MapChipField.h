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
public:
	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};
	// 範囲矩形
	struct Rect {
		float left; // 左端
		float right; // 右端
		float bottom; // 下端
		float top; // 上端
	};

	void ResetMapChipData();                          // リセット
	void LoadMapChipCsv(const std::string& filePath); // 読み込み

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex); // マップチップ種別の取得
	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex); // マップチップ座標の取得

	uint32_t GetNumBlockVertical();
	uint32_t GetNumBlockHorizontal();

	IndexSet GetMapChipIndexSetByPosition(const Vector3& position);
	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);


private:
	// 1ブロックのサイズ
	static inline const float kBlockWidth = 2.0f;
	static inline const float kBlockHeight = 2.0f;
	// ブロックの個数
	static inline const uint32_t kNumBlockVertical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

	MapChipData mapChipData_;
};