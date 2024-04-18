#include "GameScene.h"
#include "TextureManager.h"
#include <cassert>
#include "ImGuiManager.h"

GameScene::GameScene() {}

GameScene::~GameScene() {}

void GameScene::Initialize() {

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();

	// ファイル名を指定してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("uvChecker.png");
	// スプライトの生成
	sprite_ = Sprite::Create(textureHandle_, {100, 100});
	// 3Dモデルの生成
	model_ = Model::Create();

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	// ビュープロジェクションの初期化
	viewProjection_.Initialize();

	// サウンドデータの読み込み
	soundDataHandle_ = audio_->LoadWave("fanfare.wav");
	// 音声再生
	voiceHandle_ = audio_->PlayWave(soundDataHandle_, true);
}

void GameScene::Update() {
	 Vector2 position = sprite_->GetPosition();
	
	// スペースキーを押した瞬間
	if (input_->TriggerKey(DIK_SPACE)) {
		// 再生されていたら
		if (audio_->IsPlaying(voiceHandle_)) {
			// 音声停止
			audio_->StopWave(voiceHandle_);
		} else { // そうでなかったら
			// 再生(voiceHandle_もう一度読み込み)
			voiceHandle_ = audio_->PlayWave(soundDataHandle_, true);
		}
	}

	// デバッグテキストの表示
	// ImGuiはDebugモード専用(Releaseだとエラー)
	#ifdef _DEBUG // Debugモードの時のみ有効にする
	ImGui::Text("Kamata Tarou %d.%d.%d.", 2050, 12, 31);
	ImGui::Begin("Debug1");
	ImGui::Text("Kamata Tarou %d.%d.%d.", 2050, 12, 31);
	ImGui::InputFloat3("InputFloat3", inputFloat3); // float3入力ボックス
	ImGui::SliderFloat3("SliderFloat3", inputFloat3, 0.0f, 1.0f); // float3スライダー
	ImGui::End();

	ImGui::ShowDemoWindow();
	#endif
}

void GameScene::Draw() {

	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに背景スプライトの描画処理を追加できる
	/// </summary>
	
	sprite_->Draw();

	// スプライト描画後処理
	Sprite::PostDraw();
	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理
	Model::PreDraw(commandList);

	/// <summary>
	/// ここに3Dオブジェクトの描画処理を追加できる
	/// </summary>
	model_->Draw(worldTransform_, viewProjection_, textureHandle_);

	// 3Dオブジェクト描画後処理
	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>

	// スプライト描画後処理
	Sprite::PostDraw();

	// デストラクタ
	delete sprite_;
	delete model_;

#pragma endregion
}
