#include "Player.h"
#include "cassert" // assert()を使うのに必要

Player::Player() {}

Player::~Player() {}

void Player::Intialize(Model* model, uint32_t textureHandle, ViewProjection* viewProjection) {
	assert(model); //引数として受け取ったポインタが無効なNULLポインタでないか確認
	model_ = model; //モデル
	textureHandle_ = textureHandle; //テクスチャハンドル
	worldTransform_.Initialize(); //ワールドトランスフォーム
	viewProjection_ = viewProjection; //ビュープロジェクション
}

void Player::Update() { 
	// 行列を定数バッファに転送
	worldTransform_.TransferMatrix();
}

void Player::Draw() {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, viewProjection_, textureHandle_);
}
