#pragma once

#include "Audio.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "Model.h"
//#include "Player.h"
#include "Sprite.h"
#include "ViewProjection.h"
#include "WorldTransform.h"
#include <vector>
#include "DebugCamera.h"
#include "Skydome.h"

/// <summary>
/// ゲームシーン
/// </summary>
class GameScene {

public: // メンバ関数
	/// <summary>
	/// コンストクラタ
	/// </summary>
	GameScene();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~GameScene();
	std::vector<std::vector<WorldTransform*>> worldTransformBlocks_;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

private: // メンバ変数
	DirectXCommon* dxCommon_ = nullptr;
	Input* input_ = nullptr;
	Audio* audio_ = nullptr;

	uint32_t textureHandle_ = 0;    // テクスチャハンドル
	Model* model_ = nullptr;        // 3Dモデル
	ViewProjection viewProjection_; // ビュープロジェクション

	Skydome* skydome_ = nullptr; // 天球
	Model* modelSkydome_ = nullptr; // 天球3Dモデル

//	Player* player_ = nullptr; // 自キャラ

	Model* blockModel_ = nullptr; // ブロック3Dモデル

	bool isDebugCameraActive_ = false; // デバッグカメラ有効
	DebugCamera* debugCamera_ = nullptr; // デバッグカメラ


	/// <summary>
	/// ゲームシーン用
	/// </summary>
};