#define NOMINMAX
#include "Player.h"
#include "Input.h"
#include "cassert"
#include <numbers>
#include <algorithm>
#include <cassert>
#include "Matrix4x4.h"
#include "MapChipField.h"

Player::Player() {}

Player::~Player() {}

void Player::Initialise(ViewProjection* viewProjection, const Vector3& position) {
	//assert(model);                    // 引数として受け取ったポインタが無効なNULLポインタでないか確認
	worldTransform_.Initialize();     // ワールドトランスフォームの初期化
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f; // 初期回転角(右向き)
	viewProjection_ = viewProjection; // ビュープロジェクション
	model_ = Model::CreateFromOBJ("Player", true);                  // 引数の内容をメンバ変数に記録
}

// 移動入力
void Player::InputMove() {
	if (onGround_) {
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
					// 旋回開始時の角度
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					// 旋回タイマー
					turnTimer_ = 0.3f;
				}
			} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
				// 右移動中の左入力
				if (velocity_.x > 0.0f) {
					// 速度と逆方向に入力中は急ブレーキ
					velocity_.x *= (1.0f - kAttenuation);
				}
				acceleration.x -= kAcceleration;
				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					// 旋回開始時の角度
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					// 旋回タイマー
					turnTimer_ = 0.3f;
				}
			}
			// 加速/減速
			velocity_.x += acceleration.x;
			velocity_.y += acceleration.y;
			velocity_.z += acceleration.z;
			if (acceleration.x >= 0.01f || acceleration.x <= -0.01f) {
				acceleration.x = 0;
			}

		} else {
			// 非入力時は移動減衰をかける
			velocity_.x *= (1.0f - kAttenuation);
		}
		if (Input::GetInstance()->PushKey(DIK_UP)) {
			// ジャンプ初速
			velocity_ += Vector3(0, kJumpAcceleration , 0);
		}

		// 最大速度制限
		velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

	} else {
		// 落下速度
		velocity_ += Vector3(0, -kGravityAcceleration, 0);
		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
		// 着地フラグ
		landing = false;
		// 地面との当たり判定
		// 下降中？
		if (velocity_.y < 0) {
			// Y座標が地面以下になったら着地
			if (worldTransform_.translation_.y <= 1.0f) {
				landing = true;
			}
		}
	}
	// 接地判定
	if (onGround_) {
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			// 空中状態に移行
			onGround_ = false;
		}
	} else {
		// 着地
		if (landing) {
			// めり込み排斥
			worldTransform_.translation_.y = 1.0f;
			// 摩擦で横方向速度が減衰する
			velocity_.x *= (1.0f - kAttenuation);
			// 下方向速度をリセット
			velocity_.y = 0.0f;
			// 接地状態に移行
			onGround_ = true;
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
}


void Player::Update() {

	InputMove();

	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.move = velocity_;

	// 呼び出し
	// マップ衝突チェック
	CheckMapCollision(collisionMapInfo);
	// 移動
	collisionResult(collisionMapInfo);
	// 天井
	isCeilingCollision(collisionMapInfo);
	// 壁
	isWallCollision(collisionMapInfo);
	// 地面
	isLandingCollision(collisionMapInfo);

	//// 移動
	//worldTransform_.translation_ += collisionMapInfo.move;
	//// 天井接触による落下開始
	//if (collisionMapInfo.ceiling) {
	//	velocity_.y = 0;
	//}
	//// 壁接触による減速
	//if (collisionMapInfo.hitWall) {
	//	velocity_.x *= (1.0f - kAttenuationWall);
	//}

	//  行列計算
	worldTransform_.UpdateMatrix();
	// 行列を定数バッファに転送
	//	worldTransform_.TransferMatrix();
}

void Player::Draw() {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, *viewProjection_);
}

Vector3 Player::CornerPostion(const Vector3& center, Corner corner) {
	Vector3 offsetTable[kNumCorners] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0}, // kRightBottom
        {-kWidth / 2.0f, -kHeight / 2.0f, 0}, // kLeftBottom
        {+kWidth / 2.0f, +kHeight / 2.0f, 0}, // kRightTop
        {-kWidth / 2.0f, +kHeight / 2.0f, 0} // kLeftTop
    };
	return Add(center, offsetTable[static_cast<uint32_t>(corner)]);
};

// マップ衝突判定
void Player::CheckMapCollision(CollisionMapInfo& info) {
	CheckMapCollisionTop(info); // 上
	CheckMapCollisionBottom(info); // 下
	CheckMapCollisionRight(info); // 右
	CheckMapCollisionLeft(info); // 左
}

// マップ衝突判定上方向
void Player::CheckMapCollisionTop(CollisionMapInfo& info) {

	if (info.move.y <= 0) {
		return;
	}

	// 4つの角の座標計算
	std::array<Vector3, kNumCorners> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPostion(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	//真上の当たり判定
	bool hit = false;

	MapChipField::IndexSet indexSet;
	// 左上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	// ブロックに衝突か
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(0, +kHeight / 2.0f, 0));
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		// 移動量Y=上にいるブロックの下端-プレイヤーY座標-(プレイヤーの高さ/2+ブランク)
		info.move.y = std::max(0.0f, rect.bottom - worldTransform_.translation_.y - (kHeight / 2.0f + kBlank));
		// 天井に当たったことを記録
		info.ceiling = true;
	}
}

// マップ衝突判定下方向
 void Player::CheckMapCollisionBottom(CollisionMapInfo& info) {
	if (info.move.y >= 0) { // 下降あり？
		return;
	}

	// 4つの角の座標計算
	std::array<Vector3, kNumCorners> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPostion(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	// 真下の当たり判定
	bool hit = false;

	MapChipField::IndexSet indexSet;
	// 左下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	// ブロックに衝突か
	if (hit) {
		Vector3 offset = {0.0f, kHeight / 2.0f, 0.0f};
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(Subtract(Add(worldTransform_.translation_, info.move), offset));
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		// 移動量Y=下にいるブロックの上端-プレイヤーY座標+(プレイヤーの高さ/2)
		info.move.y = std::max(0.0f, rect.top - worldTransform_.translation_.y + (kHeight / 2.0f));

		// 地面に当たったことを記録
		info.landing = true;
	}
 }

// マップ衝突判定左方向
 void Player::CheckMapCollisionLeft(CollisionMapInfo& info) {
	if (info.move.x >= 0) { // 左移動あり？
		return;
	}
	
	// 4つの角の座標計算
	std::array<Vector3, kNumCorners> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPostion(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	// 左の当たり判定
	bool hit = false;

	MapChipField::IndexSet indexSet;
	// 左上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	// 左下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	// ブロックに衝突か
	if (hit) {
		Vector3 offset = {kWidth / 2.0f, 0.0f, 0.0f};
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(Subtract(Add(worldTransform_.translation_, info.move), offset));
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		// 移動量X=左にいるブロックの右端-プレイヤーX座標-(プレイヤーの高さ/2+ブランク)
		info.move.x = std::max(0.0f, rect.right - worldTransform_.translation_.x - (kHeight / 2.0f + kBlank));
		// 壁に当たったことを記録
		info.hitWall = true;
	}
 }

// マップ衝突判定右方向
void Player::CheckMapCollisionRight(CollisionMapInfo& info) {
	if (info.move.x <= 0) { // 右移動あり？
		return;
	}

	// 4つの角の座標計算
	std::array<Vector3, kNumCorners> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPostion(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	// 右の当たり判定
	bool hit = false;

	MapChipField::IndexSet indexSet;
	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	// ブロックに衝突か
	if (hit) {
		Vector3 offset = {kWidth / 2.0f, 0.0f, 0.0f};
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(Add(Add(worldTransform_.translation_, info.move), offset));
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		// 移動量X=右にいるブロックの左端-プレイヤーX座標-(プレイヤーの横幅/2+ブランク)
		info.move.x = std::max(0.0f, rect.left - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank));

		// 壁に当たったことを記録
		info.hitWall = true;
	}
}


// 判定結果を反映して移動させる
void Player::collisionResult(CollisionMapInfo& info) { 
	// 移動
	worldTransform_.translation_ = Add(worldTransform_.translation_, info.move); 
}

// 天井に接触している場合の処理
void Player::isCeilingCollision(CollisionMapInfo& info) {
	if (info.ceiling) { // 天井衝突フラグ
		velocity_.y = 0.0f;
	}
	if (info.ceiling) {
		velocity_.y = 0.0f;
	}
}

 // 壁に接触している場合の処理
void Player::isWallCollision(CollisionMapInfo& info) {
	// 壁接触による減衰
	if (info.hitWall) { // 壁衝突フラグ
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

// 接地状態の切り替え処理
void Player::isLandingCollision(CollisionMapInfo& info) {
	velocity_ = Add(velocity_, {0.0f, -kGravityAcceleration, 0.0f});
	velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);

	// 自キャラが接地状態？
	if (onGround_) {
		// 接地状態の処理
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {
			// 落下判定

			// 4つの角の座標計算
			std::array<Vector3, kNumCorners> positionsNew;

			for (uint32_t i = 0; i < positionsNew.size(); ++i) {
				positionsNew[i] = CornerPostion(Add(worldTransform_.translation_, info.move), static_cast<Corner>(i));
			}

			MapChipType mapChipType;
			// 真下の当たり判定
			bool hit = false;

			MapChipField::IndexSet indexSet;

			// 吸着　微小な数値
			//const float kSmallOffset = 0.8f;
			//// 下にずらす
			//positionsNew[kLeftBottom].y -= kSmallOffset;
			//positionsNew[kRightBottom].y -= kSmallOffset;

			// 左下点の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}
			// 右下点の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			// 落下開始
			if (!hit) { // 落下なら
				onGround_ = false; // 空中状態に切り替え
			}
		}
	} else {
		// 空中状態の処理 
		if (info.landing) { // 着地フラグ
			onGround_ = true; //着地状態に切り替え(落下を止める)
			velocity_.x *= (1.0f - kAttenuationLanding); // 着地時にX座標を減衰
			velocity_.y = 0.0f; // Y座標をゼロにする
		}
	}
}