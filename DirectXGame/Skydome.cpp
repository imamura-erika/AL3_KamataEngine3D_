#include "Skydome.h"

void Skydome::Initialize(Model* model, ViewProjection* viewProjection) {
	assert(model);                    // 引数として受け取ったポインタが無効なNULLポインタでないか確認
	model_ = model;                   // モデル
	worldTransform_.Initialize();     // ワールドトランスフォーム
	viewProjection_ = viewProjection; // ビュープロジェクション
	//worldTransform_.scale_ = {1, 1, 1};
}

void Skydome::Update() {}

void Skydome::Draw() {
	// 3Dモデル描画
	model_->Draw(worldTransform_, *viewProjection_);
}