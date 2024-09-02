#pragma once
#include "Model.h"
#include "WorldTransform.h"
#include "ViewProjection.h"

// 自キャラ
class Player {

public: // メンバ関数
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Player();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Player();

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialise(Model* model, ViewProjection* viewProjection, const Vector3& position);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();


	const Vector3& GetVelocity() const { return velocity_; }

	const WorldTransform& GetWorldTransform() const { return worldTransform_; }


private: // メンバ変数
	// ワールド変換データ
	WorldTransform worldTransform_;
	// モデル
	Model* model_ = nullptr;
	// テクスチャハンドル
	uint32_t textureHandle_ = 0;
	// ビュープロジェクション
	ViewProjection* viewProjection_ = nullptr;


	// 移動
	Vector3 velocity_ = {};

	static inline const float kAcceleration = 0.1f; // 慣性移動
	static inline const float kAttenuation = 0.1f; // 速度減衰率
	static inline const float kLimitRunSpeed = 1.0f; // 最大速度制限

	// 振り向き
	// 左右
	enum class LRDirection {
		kRight,
		kLeft,
	};
	LRDirection lrDirection_ = LRDirection::kRight;

	float turnFirstRotationY_ = 0.0f; // 旋回開始時の角度
	float turnTimer_ = 0.0f; // 旋回タイマー
	static inline const float kTimeTurn = 0.3f; // 旋回時間<秒>

	
	// ジャンプ・着地
	bool onGround_ = true; // 接地状態フラグ

	static inline const float kGravityAcceleration = 0.1f; // 重力加速度(下方向)
	static inline const float kLimitFallSpeed = 0.8f; // 最大落下速度(下方向)
	static inline const float kJumpAcceleration = 0.5f; // ジャンプ初速(上方向)
};