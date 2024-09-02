#define NOMINMAX
#include "Player.h"
#include "Input.h"
#include "cassert"
#include <numbers>
#include <algorithm>
#include <cassert>
#include "Matrix4x4.h"

Player::Player() {}

Player::~Player() {}

void Player::Initialise(Model* model, ViewProjection* viewProjection, const Vector3& position) {
	assert(model);                    // 引数として受け取ったポインタが無効なNULLポインタでないか確認
	model_ = model;                   // モデル

	worldTransform_.Initialize();     // ワールドトランスフォーム

	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f; // 初期回転角
	viewProjection_ = viewProjection; // ビュープロジェクション
}

void Player::Update() {
	// 行列を定数バッファに転送
	//worldTransform_.TransferMatrix();

	bool landing = false; // 着地フラグ

	// 地面との当たり判定
	// 下降中か
	if (velocity_.y < 0.0f) {
		// y座標が地面以下になったら着地
		if (worldTransform_.translation_.y <= 2.0f) {
			landing = true;
		}
	}
	// 移動入力
	// 接地状態
	if (onGround_) {
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			// 空中状態に移行
			onGround_ = false;
		}
		// 左右移動操作
		if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {

			// 左右加速
			Vector3 acceleration = {};
			if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
				// 左移動中の右入力
				if (velocity_.x < 0.0f) {
					// 速度と逆方向に入力中は急ブレーキ
					velocity_.x *= (1.0f - kAttenuation);
				}
				acceleration.x += kAcceleration;
				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;
					// 旋回開始時の角度を記録
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					// 前回タイマーに時間を設定
					turnTimer_ = 0.2f;
				}
			} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
				// 右移動の左入力
				if (velocity_.x > 0.0f) {
					// 速度と逆方向に入力中は急ブレーキ
					velocity_.x *= (1.0f - kAttenuation);
				}
				acceleration.x -= kAcceleration;
				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					// 旋回開始時の角度を記録
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					// 前回タイマーに時間を設定
					turnTimer_ = 0.2f;
				}
			}

			// 加減速
			velocity_ = Add(velocity_, acceleration);

			// 制限速度
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
		} else {
			velocity_.x *= (1.0f - kAttenuation);
		}
		// ジャンプ
		if (Input::GetInstance()->PushKey(DIK_UP)) {
			// ジャンプ初速
			velocity_ = Add(velocity_, {0.0f, kJumpAcceleration, 0.0f});
		}
	} else // 空中
	{
		// 落下速度
		velocity_ = Add(velocity_, Vector3(0, -kGravityAcceleration, 0));
		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -1 * kLimitFallSpeed);
		// 着地
		if (landing) {
			worldTransform_.translation_.y = 2.0f; // めり込み排斥			
			velocity_.x *= (1.0f - kAttenuation); // 摩擦で横方向が減衰
			velocity_.y = 0.0f; // 下方向速度をリセット
			onGround_ = true; // 接地状態に移行
		}
	}

	// 旋回制御
	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60;
		// 左右の自キャラ角度テーブル
		float destinationRotationYTable[] = {
		    std::numbers::pi_v<float> / 2.0f,
		    std::numbers::pi_v<float> * 3.0f / 2.0f,
		};

		// 角度取得
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		float timeRatio = 1 - turnTimer_ / kTimeTurn;
		float easing = timeRatio;
		float nowRotationY = std::lerp(turnFirstRotationY_, destinationRotationY, easing);
		worldTransform_.rotation_.y = nowRotationY;
	}

	worldTransform_.translation_ = Add(worldTransform_.translation_, velocity_);
	worldTransform_.UpdateMatrix();
}

void Player::Draw() {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, *viewProjection_);
}